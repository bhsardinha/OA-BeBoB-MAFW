// Bench tool: the latency numbers CoreAudio reports for the FW1814 at its current rate (what a DAW adds up for "roundtrip"):
// device latency, safety offset and stream latency per scope, buffer frame size and its range, nominal rate.
//   halinfo
// Build: cc -o halinfo halinfo.c -framework CoreAudio -framework CoreFoundation
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static UInt32 u32(AudioObjectID o, AudioObjectPropertySelector s, AudioObjectPropertyScope sc) {
    AudioObjectPropertyAddress a = { s, sc, 0 };
    UInt32 v = 0, n = sizeof v;
    return AudioObjectGetPropertyData(o, &a, 0, NULL, &n, &v) ? 0xFFFFFFFF : v;
}
int main(void) {
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, 0 };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++) {
        CFStringRef uid = NULL; UInt32 n = sizeof uid;
        AudioObjectPropertyAddress ua = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, 0 };
        AudioObjectGetPropertyData(ids[i], &ua, 0, NULL, &n, &uid);
        char buf[256] = ""; if (uid) CFStringGetCString(uid, buf, sizeof buf, kCFStringEncodingUTF8);
        if (!strstr(buf, "FWEngine2")) continue;
        Float64 rate = 0; n = sizeof rate;
        AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 };
        AudioObjectGetPropertyData(ids[i], &ra, 0, NULL, &n, &rate);
        AudioValueRange br; n = sizeof br;
        AudioObjectPropertyAddress ba = { kAudioDevicePropertyBufferFrameSizeRange, kAudioObjectPropertyScopeGlobal, 0 };
        AudioObjectGetPropertyData(ids[i], &ba, 0, NULL, &n, &br);
        printf("%s  rate %.0f  buffer %u (range %.0f-%.0f)\n", buf, rate, u32(ids[i], kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal), br.mMinimum, br.mMaximum);
        { AudioObjectPropertyAddress na = { kAudioDevicePropertyAvailableNominalSampleRates, kAudioObjectPropertyScopeGlobal, 0 };
          UInt32 nsz = 0; AudioObjectGetPropertyDataSize(ids[i], &na, 0, NULL, &nsz);
          AudioValueRange *nr = malloc(nsz); AudioObjectGetPropertyData(ids[i], &na, 0, NULL, &nsz, nr);
          printf("  available rates:"); for (UInt32 k = 0; k < nsz / sizeof *nr; k++) printf(" %.0f", nr[k].mMinimum); printf("\n"); free(nr); }
        for (int s = 0; s < 2; s++) {
            AudioObjectPropertyScope sc = s ? kAudioObjectPropertyScopeOutput : kAudioObjectPropertyScopeInput;
            UInt32 stream = 0; n = sizeof stream;
            AudioObjectPropertyAddress sa = { kAudioDevicePropertyStreams, sc, 0 };
            UInt32 sz = 0; AudioObjectGetPropertyDataSize(ids[i], &sa, 0, NULL, &sz);
            AudioObjectID st[8]; sz = sz > sizeof st ? sizeof st : sz;
            AudioObjectGetPropertyData(ids[i], &sa, 0, NULL, &sz, st); stream = st[0];
            printf("  %-6s device latency %u  safety offset %u  stream latency %u  (frames)\n", s ? "output" : "input",
                   u32(ids[i], kAudioDevicePropertyLatency, sc), u32(ids[i], kAudioDevicePropertySafetyOffset, sc),
                   u32(stream, kAudioStreamPropertyLatency, kAudioObjectPropertyScopeGlobal));
        }
    }
    return 0;
}
