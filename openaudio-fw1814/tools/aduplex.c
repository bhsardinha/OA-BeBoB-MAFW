// Bench tool for the integrated run (O1, ADAT/S-PDIF switch while streaming): a duplex CoreAudio client for the FW1814 that outputs silence
// (nothing audible) and reports, once a second, the IOProc callback count, the first/last input sample peak and the device's current channel counts.
//   aduplex [SECONDS]      default 60; ends earlier if the device disappears
// Build: cc -o build/aduplex tools/aduplex.c -framework CoreAudio -framework CoreFoundation
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

static volatile UInt64 gCalls = 0;
static volatile float gPeak = 0;
static volatile UInt32 gInBuf = 0, gOutBuf = 0;

static OSStatus ioproc(AudioObjectID dev, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *it,
                       AudioBufferList *out, const AudioTimeStamp *ot, void *ref) {
    gCalls++;
    gInBuf = in ? in->mNumberBuffers : 0;
    gOutBuf = out ? out->mNumberBuffers : 0;
    float p = 0;
    for (UInt32 b = 0; in && b < in->mNumberBuffers; b++) {
        const float *s = in->mBuffers[b].mData;
        UInt32 n = in->mBuffers[b].mDataByteSize / sizeof(float);
        for (UInt32 i = 0; s && i < n; i++) { float a = fabsf(s[i]); if (a > p) p = a; }
    }
    gPeak = p;
    for (UInt32 b = 0; out && b < out->mNumberBuffers; b++) memset(out->mBuffers[b].mData, 0, out->mBuffers[b].mDataByteSize);   // silence
    return noErr;
}

static UInt32 channels(AudioObjectID dev, AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress a = { kAudioDevicePropertyStreamConfiguration, scope, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(dev, &a, 0, NULL, &size) || !size) return 0;
    AudioBufferList *l = malloc(size);
    UInt32 n = 0;
    if (!AudioObjectGetPropertyData(dev, &a, 0, NULL, &size, l)) for (UInt32 i = 0; i < l->mNumberBuffers; i++) n += l->mBuffers[i].mNumberChannels;
    free(l);
    return n;
}

int main(int argc, char **argv) {
    int seconds = argc > 1 ? atoi(argv[1]) : 60;
    AudioObjectPropertyAddress ha = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &ha, 0, NULL, &size);
    AudioObjectID *ids = malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &ha, 0, NULL, &size, ids);
    AudioObjectID dev = 0;
    for (UInt32 i = 0; i < size / sizeof *ids; i++) {
        CFStringRef name = NULL; UInt32 s = sizeof name;
        AudioObjectPropertyAddress na = { kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        if (AudioObjectGetPropertyData(ids[i], &na, 0, NULL, &s, &name) || !name) continue;
        char buf[256]; CFStringGetCString(name, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(name);
        if (strstr(buf, "1814")) { dev = ids[i]; printf("device %u \"%s\"\n", (unsigned)dev, buf); break; }
    }
    if (!dev) { puts("no FireWire 1814 audio device"); return 1; }
    AudioDeviceIOProcID pid;
    OSStatus r = AudioDeviceCreateIOProcID(dev, ioproc, NULL, &pid);
    if (r) { printf("CreateIOProcID %d\n", (int)r); return 1; }
    r = AudioDeviceStart(dev, pid);
    if (r) { printf("AudioDeviceStart %d\n", (int)r); return 1; }
    printf("streaming (silence out, input monitored)\n"); fflush(stdout);
    UInt64 last = 0;
    for (int t = 1; t <= seconds; t++) {
        sleep(1);
        UInt64 c = gCalls;
        printf("t=%2ds calls +%llu in-ch %u out-ch %u in-bufs %u out-bufs %u peak %.5f\n", t, (unsigned long long)(c - last),
               (unsigned)channels(dev, kAudioDevicePropertyScopeInput), (unsigned)channels(dev, kAudioDevicePropertyScopeOutput),
               (unsigned)gInBuf, (unsigned)gOutBuf, gPeak);
        fflush(stdout);
        last = c;
    }
    AudioDeviceStop(dev, pid);
    AudioDeviceDestroyIOProcID(dev, pid);
    return 0;
}
