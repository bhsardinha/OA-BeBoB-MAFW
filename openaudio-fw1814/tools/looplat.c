// Bench tool (K14 / M-L): real round-trip latency through the FW1814 with a loopback cable (analog OUT n -> analog IN n, any pair).
// Sends one click per second on every output channel and watches every input channel; the delay is counted in frames between the output
// sample's index and the input sample's index in the same IOProc stream (independent of any latency the driver reports).
//   looplat [RATE [BUFFER [SECONDS]]]     defaults: current rate, current buffer, 8 s
//   LOOPLAT_OUT=n: click on output channel n only (1-based; default every output) -- e.g. a loop OUT 3 -> IN 1 without clicks on the monitors
// Build: cc -o looplat looplat.c -framework CoreAudio -framework CoreFoundation
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

static AudioObjectID gDev;
static UInt32 gOutCh, gInCh, gBuf;
static volatile UInt64 gOutFrame, gInFrame;            // frames delivered so far (start of the current callback)
static UInt64 gClickAt = 0;                            // output frame index of the click in flight
static volatile int gSeen[64];
static volatile double gDelay[64];
static volatile int gArmed;
static Float64 gRate;
static float gThresh = 0.03f;
static int gOutOnly = 0;                             // LOOPLAT_OUT, 0 = every output                         // a quiet loopback (input gain low) still clears it; LOOPLAT_THRESH overrides
static volatile float gPeak[64];

static OSStatus ioproc(AudioObjectID dev, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *it,
                       AudioBufferList *out, const AudioTimeStamp *ot, void *ctx) {
    (void)dev; (void)now; (void)it; (void)ot; (void)ctx;
    UInt32 frames = 0;
    if (out && out->mNumberBuffers) {
        frames = out->mBuffers[0].mDataByteSize / (out->mBuffers[0].mNumberChannels * 4);
        float *o = out->mBuffers[0].mData; UInt32 oc = out->mBuffers[0].mNumberChannels;
        memset(o, 0, out->mBuffers[0].mDataByteSize);
        for (UInt32 i = 0; i < frames; i++) {
            UInt64 idx = gOutFrame + i;
            if (idx % (UInt64)gRate == 100) {           // one click per second
                for (UInt32 c = 0; c < oc; c++) if (!gOutOnly || (int)c == gOutOnly - 1) o[i * oc + c] = 0.9f;
                gClickAt = idx; gArmed = 1; memset((void *)gSeen, 0, sizeof gSeen);
            }
        }
    }
    if (in && in->mNumberBuffers && gArmed) {
        UInt32 ic = in->mBuffers[0].mNumberChannels, n = in->mBuffers[0].mDataByteSize / (ic * 4);
        const float *x = in->mBuffers[0].mData;
        for (UInt32 i = 0; i < n; i++)
            for (UInt32 c = 0; c < ic && c < 64; c++) {
                if (fabsf(x[i * ic + c]) > gPeak[c]) gPeak[c] = fabsf(x[i * ic + c]);
                if (!gSeen[c] && fabsf(x[i * ic + c]) > gThresh && gInFrame + i >= gClickAt) {
                    gSeen[c] = 1; gDelay[c] = (double)(gInFrame + i - gClickAt);
                    printf("in ch %2u  round trip %6.0f frames = %6.1f ms\n", c + 1, gDelay[c], gDelay[c] * 1000 / gRate); fflush(stdout);
                }
            }
    }
    gOutFrame += frames;
    if (in && in->mNumberBuffers) gInFrame += in->mBuffers[0].mDataByteSize / (in->mBuffers[0].mNumberChannels * 4);
    return 0;
}

int main(int argc, char **argv) {
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
        if (strstr(buf, "com_m_audio_FWEngine")) gDev = ids[i];   // FWEngine2 (FW1814, Audiophile) or FWEngine1 (FW410 UID, R135)
    }
    if (!gDev) { puts("no FW1814 in CoreAudio"); return 1; }
    UInt32 n;
    if (argc > 1 && atoi(argv[1])) {
        Float64 r = atoi(argv[1]); AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 };
        OSStatus st = AudioObjectSetPropertyData(gDev, &ra, 0, NULL, sizeof r, &r); printf("set rate %.0f -> %d\n", r, (int)st); sleep(8);
    }
    if (argc > 2 && atoi(argv[2])) {
        UInt32 b = atoi(argv[2]); AudioObjectPropertyAddress ba = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, 0 };
        AudioObjectSetPropertyData(gDev, &ba, 0, NULL, sizeof b, &b);
    }
    AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 };
    n = sizeof gRate; AudioObjectGetPropertyData(gDev, &ra, 0, NULL, &n, &gRate);
    AudioObjectPropertyAddress ba = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, 0 };
    n = sizeof gBuf; AudioObjectGetPropertyData(gDev, &ba, 0, NULL, &n, &gBuf);
    { UInt32 sz = 0; AudioObjectPropertyAddress sa = { kAudioDevicePropertyStreamConfiguration, kAudioObjectPropertyScopeInput, 0 };
      AudioObjectGetPropertyDataSize(gDev, &sa, 0, NULL, &sz); AudioBufferList *l = malloc(sz); AudioObjectGetPropertyData(gDev, &sa, 0, NULL, &sz, l); gInCh = l->mBuffers[0].mNumberChannels; }
    if (getenv("LOOPLAT_THRESH")) gThresh = atof(getenv("LOOPLAT_THRESH"));
    if (getenv("LOOPLAT_OUT")) gOutOnly = atoi(getenv("LOOPLAT_OUT"));
    printf("rate %.0f  buffer %u frames (%.1f ms)  - clicks on every output, watching every input\n", gRate, gBuf, gBuf * 1000 / gRate);
    AudioDeviceIOProcID pid;
    AudioDeviceCreateIOProcID(gDev, ioproc, NULL, &pid);
    AudioDeviceStart(gDev, pid);
    sleep(argc > 3 ? atoi(argv[3]) : 8);
    AudioDeviceStop(gDev, pid);
    int any = 0; for (int c = 0; c < 64; c++) any |= gSeen[c];
    if (!any) {
        printf("no click came back (threshold %.3f). Loudest input sample per channel:", gThresh);
        for (UInt32 c = 0; c < gInCh && c < 64; c++) printf(" %u:%.4f", c + 1, gPeak[c]);
        puts("\ncheck the loopback cable (OUT n -> IN n), the output level in the panel and the input gain");
    }
    return 0;
}
