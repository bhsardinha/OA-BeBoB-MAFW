// Bench tool: log every CoreAudio HAL property notification of the FW1814 (device, its streams, the system device list) with a timestamp,
// plus a snapshot (stream object ids, input/output channel totals) after each burst. Audio MIDI Setup builds its device list ("N ins / M outs")
// from exactly these notifications, so running this against the official driver (Snow Leopard rig, v1.9) and against the clone shows what the
// clone fails to announce when an ADAT/S-PDIF switch changes the channel counts.
//   halwatch [SECONDS]      default 40
// Build: cc -o halwatch halwatch.c -framework CoreAudio -framework CoreFoundation   (SL: add -arch i386 -arch x86_64 -mmacosx-version-min=10.6)
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static AudioObjectID gDev = 0;
static struct timeval gT0;
#define MASTER 0                                        // kAudioObjectPropertyElementMaster (the symbol is deprecated on newer SDKs)

static double now(void) { struct timeval t; gettimeofday(&t, NULL); return (t.tv_sec - gT0.tv_sec) + (t.tv_usec - gT0.tv_usec) / 1e6; }
static void fourcc(UInt32 v, char *o) { for (int i = 0; i < 4; i++) { char c = (char)((v >> (24 - 8 * i)) & 0xff); o[i] = (c >= 32 && c < 127) ? c : '.'; } o[4] = 0; }

static UInt32 channels(AudioObjectID obj, AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress a = { kAudioDevicePropertyStreamConfiguration, scope, MASTER };
    UInt32 size = 0, n = 0;
    if (AudioObjectGetPropertyDataSize(obj, &a, 0, NULL, &size) || !size) return 0;
    AudioBufferList *l = malloc(size);
    if (!AudioObjectGetPropertyData(obj, &a, 0, NULL, &size, l)) for (UInt32 i = 0; i < l->mNumberBuffers; i++) n += l->mBuffers[i].mNumberChannels;
    free(l);
    return n;
}

// Stream object ids of the device, input scope then output scope (the global scope returns none).
static UInt32 streamIDs(AudioObjectID *out, UInt32 max, UInt32 *nIn) {
    UInt32 n = 0;
    for (int pass = 0; pass < 2; pass++) {
        AudioObjectPropertyAddress a = { kAudioDevicePropertyStreams, pass == 0 ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput, MASTER };
        UInt32 size = 0;
        if (!AudioObjectGetPropertyDataSize(gDev, &a, 0, NULL, &size) && n + size / sizeof(AudioObjectID) <= max) {
            AudioObjectGetPropertyData(gDev, &a, 0, NULL, &size, out + n);
            n += size / sizeof(AudioObjectID);
        }
        if (pass == 0) *nIn = n;
    }
    return n;
}

static void snapshot(const char *why) {
    AudioObjectID ids[32]; UInt32 nIn = 0;
    UInt32 n = streamIDs(ids, 32, &nIn);
    printf("%8.3f   [snapshot %s] streams:", now(), why);
    for (UInt32 i = 0; i < n; i++) {
        UInt32 ch = 0;
        AudioStreamBasicDescription f; UInt32 fs = sizeof f;
        AudioObjectPropertyAddress fa = { kAudioStreamPropertyVirtualFormat, kAudioObjectPropertyScopeGlobal, MASTER };
        if (!AudioObjectGetPropertyData(ids[i], &fa, 0, NULL, &fs, &f)) ch = f.mChannelsPerFrame;
        printf(" %s%u(%uch)", i < nIn ? "in:" : "out:", (unsigned)ids[i], (unsigned)ch);
    }
    printf("   device in/out channels: %u / %u\n", (unsigned)channels(gDev, kAudioDevicePropertyScopeInput), (unsigned)channels(gDev, kAudioDevicePropertyScopeOutput));
    fflush(stdout);
}

static OSStatus listener(AudioObjectID obj, UInt32 n, const AudioObjectPropertyAddress *addrs, void *ref) {
    for (UInt32 i = 0; i < n; i++) {
        char sel[5], sc[5];
        fourcc(addrs[i].mSelector, sel); fourcc(addrs[i].mScope, sc);
        printf("%8.3f   object %-4u %s  selector '%s' scope '%s' element %u\n", now(), (unsigned)obj, obj == kAudioObjectSystemObject ? "(system)" : obj == gDev ? "(device)" : "(stream)",
               sel, sc, (unsigned)addrs[i].mElement);
    }
    fflush(stdout);
    return noErr;
}

static void listen(AudioObjectID obj) {
    AudioObjectPropertyAddress w = { kAudioObjectPropertySelectorWildcard, kAudioObjectPropertyScopeWildcard, kAudioObjectPropertyElementWildcard };
    AudioObjectAddPropertyListener(obj, &w, listener, NULL);
}

int main(int argc, char **argv) {
    int seconds = argc > 1 ? atoi(argv[1]) : 40;
    gettimeofday(&gT0, NULL);
    AudioObjectPropertyAddress ha = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, MASTER };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &ha, 0, NULL, &size);
    AudioObjectID *ids = malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &ha, 0, NULL, &size, ids);
    for (UInt32 i = 0; i < size / sizeof *ids; i++) {
        CFStringRef name = NULL; UInt32 s = sizeof name;
        AudioObjectPropertyAddress na = { kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal, MASTER };
        if (AudioObjectGetPropertyData(ids[i], &na, 0, NULL, &s, &name) || !name) continue;
        char buf[256]; CFStringGetCString(name, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(name);
        if (strstr(buf, "1814")) { gDev = ids[i]; printf("device %u \"%s\"\n", (unsigned)gDev, buf); break; }
    }
    if (!gDev) { puts("no FireWire 1814 audio device"); return 1; }
    listen(kAudioObjectSystemObject);
    listen(gDev);
    { AudioObjectID st[32]; UInt32 nIn = 0, n = streamIDs(st, 32, &nIn); for (UInt32 i = 0; i < n; i++) listen(st[i]); }
    snapshot("start");
    for (int t = 0; t < seconds; t++) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0, false);
        if (t % 5 == 4) snapshot("tick");
    }
    snapshot("end");
    return 0;
}
