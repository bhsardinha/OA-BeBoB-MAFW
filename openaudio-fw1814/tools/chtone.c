// Bench tool (capture 105): play a quiet 1 kHz sine on ONE output channel of the default output device, straight from a CoreAudio IOProc
// (no channel mapping, unlike afplay, which folds a multichannel file down to the first two channels).
//   chtone CHANNEL [SECONDS [DBFS [DEVICE]]]   CHANNEL is 1-based over all output streams; default 5 s at -40 dBFS on the default output;
//                                              DEVICE = a substring of the device name (e.g. "410") instead of the default output
// Build (SL 10.6 target, on the build PC): cc -arch x86_64 -mmacosx-version-min=10.6 -o build/chtone tools/chtone.c -framework CoreAudio
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

static UInt32 gChannel;          // 0-based
static double gPhase, gStep, gAmp;

static OSStatus ioproc(AudioObjectID dev, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *it,
                       AudioBufferList *out, const AudioTimeStamp *ot, void *ref) {
    UInt32 base = 0;
    for (UInt32 b = 0; out && b < out->mNumberBuffers; b++) {
        AudioBuffer *buf = &out->mBuffers[b];
        UInt32 nch = buf->mNumberChannels, frames = nch ? buf->mDataByteSize / (4 * nch) : 0;
        float *p = (float *)buf->mData;
        for (UInt32 f = 0; f < frames; f++)
            for (UInt32 c = 0; c < nch; c++) p[f * nch + c] = (base + c == gChannel) ? (float)(gAmp * sin(gPhase + gStep * f)) : 0.0f;
        if (gChannel >= base && gChannel < base + nch) gPhase = fmod(gPhase + gStep * frames, 2 * M_PI);
        base += nch;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: chtone CHANNEL [SECONDS [DBFS]]\n"); return 2; }
    gChannel = (UInt32)atoi(argv[1]) - 1;
    int seconds = argc > 2 ? atoi(argv[2]) : 5;
    gAmp = pow(10.0, (argc > 3 ? atof(argv[3]) : -40.0) / 20.0);
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    AudioObjectID dev = 0; UInt32 size = sizeof(dev);
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, &dev) || !dev) { fprintf(stderr, "no default output\n"); return 1; }
    if (argc > 4) {                                      // pick the first device whose name contains argv[4]
        AudioObjectPropertyAddress da = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        UInt32 n = 0; AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &da, 0, NULL, &n);
        AudioObjectID ids[64]; if (n > sizeof ids) n = sizeof ids;
        AudioObjectGetPropertyData(kAudioObjectSystemObject, &da, 0, NULL, &n, ids);
        dev = 0;
        for (UInt32 i = 0; i < n / sizeof(AudioObjectID) && !dev; i++) {
            CFStringRef name = NULL; UInt32 sz = sizeof name;
            AudioObjectPropertyAddress na = { kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
            char buf[256] = "";
            if (!AudioObjectGetPropertyData(ids[i], &na, 0, NULL, &sz, &name) && name) { CFStringGetCString(name, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(name); }
            if (strstr(buf, argv[4])) dev = ids[i];
        }
        if (!dev) { fprintf(stderr, "no device named *%s*\n", argv[4]); return 1; }
    }
    Float64 rate = 0; size = sizeof(rate);
    a.mSelector = kAudioDevicePropertyNominalSampleRate;
    AudioObjectGetPropertyData(dev, &a, 0, NULL, &size, &rate);
    gStep = 2 * M_PI * 1000.0 / (rate > 0 ? rate : 48000.0);
    AudioDeviceIOProcID pid = NULL;
    if (AudioDeviceCreateIOProcID(dev, ioproc, NULL, &pid) || AudioDeviceStart(dev, pid)) { fprintf(stderr, "start failed\n"); return 1; }
    printf("device %u rate %.0f channel %u for %d s\n", (unsigned)dev, rate, (unsigned)gChannel + 1, seconds);
    fflush(stdout);
    sleep((unsigned)seconds);
    AudioDeviceStop(dev, pid);
    AudioDeviceDestroyIOProcID(dev, pid);
    return 0;
}
