// OpenAudio Glue - glue-tapread: the Glue tap from an ordinary process (not coreaudiod). Maps the page (0x1000) and the ring (0x1001)
// of the first OpenAudio unit through com_openaudio_FWUserClient, prints the geometry and the mapped size, touches every page of the
// ring, then starts the unit's own Core Audio device (as a DAW would) and keeps reading for N seconds, reporting generation / state /
// write position changes. Used to find why the plug-in faulted reading the ring (2026-10-28).
//   cc -mmacosx-version-min=10.13 -I<kext src/base> glue_tapread.c -framework IOKit -framework CoreAudio -framework CoreFoundation -o glue-tapread
//   ./glue-tapread <GUID hex> [seconds] [nostart]
#include <CoreAudio/CoreAudio.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "GlueTap.h"

static volatile uint32_t gSink;
static OSStatus Proc(AudioObjectID d, const AudioTimeStamp *n, const AudioBufferList *i, const AudioTimeStamp *it, AudioBufferList *o,
                     const AudioTimeStamp *ot, void *c) { (void)d; (void)n; (void)i; (void)it; (void)o; (void)ot; (void)c; return noErr; }

static AudioObjectID FindUnit(const char *uid)
{
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    AudioObjectID found = 0;
    for (UInt32 k = 0; k < size / sizeof(AudioObjectID); k++) {
        AudioObjectPropertyAddress u = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        CFStringRef s = NULL; UInt32 sz = sizeof s; char buf[256] = "";
        if (AudioObjectGetPropertyData(ids[k], &u, 0, NULL, &sz, &s) == noErr && s) { CFStringGetCString(s, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(s); }
        if (!strcmp(buf, uid)) found = ids[k];
    }
    free(ids);
    return found;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: glue-tapread <GUID hex> [seconds] [nostart]\n"); return 2; }
    const int secs = argc > 2 ? atoi(argv[2]) : 10;
    const int start = !(argc > 3 && !strcmp(argv[3], "nostart"));
    unsigned long long want = strtoull(argv[1], NULL, 16);
    io_iterator_t it; io_service_t s, dev = 0;
    IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("com_openaudio_FWAudioDevice"), &it);
    while ((s = IOIteratorNext(it))) {
        CFTypeRef g = IORegistryEntryCreateCFProperty(s, CFSTR("GUID"), NULL, 0); long long v = 0;
        if (g) { CFNumberGetValue((CFNumberRef)g, kCFNumberLongLongType, &v); CFRelease(g); }
        if ((unsigned long long)v == want) dev = s; else IOObjectRelease(s);
    }
    if (!dev) { printf("unit %s not found\n", argv[1]); return 1; }
    io_connect_t conn;
    if (IOServiceOpen(dev, mach_task_self(), 0, &conn)) { printf("IOServiceOpen failed\n"); return 1; }
    mach_vm_address_t pa = 0, ra = 0; mach_vm_size_t ps = 0, rs = 0;
    kern_return_t kp = IOConnectMapMemory64(conn, kGlueTapPageType, mach_task_self(), &pa, &ps, kIOMapAnywhere);
    kern_return_t kr = IOConnectMapMemory64(conn, kGlueTapRingType, mach_task_self(), &ra, &rs, kIOMapAnywhere);
    printf("map page: 0x%x size %llu; map ring: 0x%x at 0x%llx size %llu\n", kp, (unsigned long long)ps, kr, (unsigned long long)ra, (unsigned long long)rs);
    if (kp || kr) return 1;
    const GlueTapPage *g = (const GlueTapPage *)pa;
    printf("page: magic %08x v%u gen %u state %u rate %u in %u wire %u frameQuads %u ringBytes %u wraps %llu writeOff %u isoch %u glue %u\n",
           g->magic, g->version, g->generation, g->state, g->rateHz, g->inChannels, g->wireQuadlets, g->frameQuads, g->ringBytes,
           (unsigned long long)g->wrapCount, g->writeOff, g->isochRefs, g->glueRefs);
    for (unsigned c = 0; c < g->inChannels && c < 16; c++) printf("%s%u<-%u/%u", c ? " " : "map: ", c, g->map[c].slot, g->map[c].delay);
    printf("\n");
    const volatile uint32_t *ring = (const volatile uint32_t *)ra;
    uint32_t sum = 0;
    for (mach_vm_size_t off = 0; off < rs; off += 4096) sum += ring[off / 4];
    gSink = sum;
    printf("touched every page of the ring (%llu pages): ok\n", (unsigned long long)(rs / 4096));
    AudioObjectID unit = 0; AudioDeviceIOProcID proc = 0;
    if (start) {
        char uid[128]; snprintf(uid, sizeof uid, "com_m_audio_FWEngine2:%s", argv[1]);
        unit = FindUnit(uid);
        if (unit && AudioDeviceCreateIOProcID(unit, Proc, NULL, &proc) == noErr && AudioDeviceStart(unit, proc) == noErr) printf("started %s\n", uid);
    }
    uint32_t lastGen = g->generation, lastState = g->state;
    for (int t = 0; t < secs * 10; t++) {
        usleep(100000);
        if (g->generation != lastGen || g->state != lastState) {
            printf("%.1f s: gen %u -> %u, state %u -> %u, ringBytes %u, wraps %llu, isoch %u glue %u\n", t / 10.0, lastGen, g->generation, lastState,
                   g->state, g->ringBytes, (unsigned long long)g->wrapCount, g->isochRefs, g->glueRefs);
            lastGen = g->generation; lastState = g->state;
        }
        sum = 0;
        for (mach_vm_size_t off = 0; off < rs; off += 4096) sum += ring[off / 4];   // faults here if the mapping lost its pages
        gSink = sum;
    }
    printf("end: wraps %llu writeOff %u; every page still readable\n", (unsigned long long)g->wrapCount, g->writeOff);
    if (proc) { AudioDeviceStop(unit, proc); AudioDeviceDestroyIOProcID(unit, proc); }
    return 0;
}
