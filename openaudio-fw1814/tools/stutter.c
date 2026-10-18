// Bench tool: does the audio I/O cycle stall while the system is busy (the "file delete makes the sound loop ~200 ms" report)?
// Runs a duplex IOProc on the FW1814 that plays a quiet 440 Hz sine and, for every callback, checks (a) the host-time gap since the previous callback against the
// nominal period and (b) the continuity of the HAL's sample times. Prints a line for every gap longer than 3 periods and a one-second summary.
//   stutter [SECONDS [BUFFER]]            defaults 30 s, current buffer.  Do other work (dd, rm, cp) in another shell meanwhile.
// Build: cc -o stutter stutter.c -framework CoreAudio -framework CoreFoundation
#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/HostTime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

static Float64 gRate; static UInt32 gBuf;
static UInt64 gLastHost, gStart; static Float64 gLastOutSample = -1, gLastInSample = -1;
static volatile UInt64 gCalls, gSecCalls, gBigGaps, gOutJumps, gInJumps; static volatile double gSecMax, gMax;
static double gPhase;
#define EVT 256
static struct { double t_ms, gap_ms; } gEvents[EVT]; static volatile int gNev;

static OSStatus ioproc(AudioObjectID d, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *it, AudioBufferList *out, const AudioTimeStamp *ot, void *c) {
    (void)d; (void)c;
    UInt64 h = now->mHostTime;
    if (gLastHost) {
        double gap = AudioConvertHostTimeToNanos(h - gLastHost) / 1e6, period = gBuf * 1000.0 / gRate;
        if (gap > gSecMax) gSecMax = gap;
        if (gap > gMax) gMax = gap;
        if (gap > 3 * period) { gBigGaps++; if (gNev < EVT) { gEvents[gNev].t_ms = AudioConvertHostTimeToNanos(h - gStart) / 1e6; gEvents[gNev].gap_ms = gap; gNev++; } }
    } else gStart = h;
    gLastHost = h; gCalls++; gSecCalls++;
    if (ot && ot->mFlags & kAudioTimeStampSampleTimeValid) { if (gLastOutSample >= 0 && fabs(ot->mSampleTime - gLastOutSample - gBuf) > 0.5) gOutJumps++; gLastOutSample = ot->mSampleTime; }
    if (it && it->mFlags & kAudioTimeStampSampleTimeValid) { if (gLastInSample >= 0 && fabs(it->mSampleTime - gLastInSample - gBuf) > 0.5) gInJumps++; gLastInSample = it->mSampleTime; }
    if (out && out->mNumberBuffers) {
        float *o = out->mBuffers[0].mData; UInt32 ch = out->mBuffers[0].mNumberChannels, n = out->mBuffers[0].mDataByteSize / (ch * 4);
        for (UInt32 i = 0; i < n; i++) { float v = 0.05f * sinf((float)gPhase); gPhase += 2 * M_PI * 440 / gRate; if (gPhase > 2 * M_PI) gPhase -= 2 * M_PI; for (UInt32 k = 0; k < ch; k++) o[i * ch + k] = (k < 2) ? v : 0; }
    }
    return 0;
}

int main(int argc, char **argv) {
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, 0 };
    UInt32 size = 0; AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = malloc(size), dev = 0; AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++) {
        CFStringRef uid = NULL; UInt32 n = sizeof uid; AudioObjectPropertyAddress ua = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, 0 };
        AudioObjectGetPropertyData(ids[i], &ua, 0, NULL, &n, &uid); char buf[256] = ""; if (uid) CFStringGetCString(uid, buf, sizeof buf, kCFStringEncodingUTF8);
        if (strstr(buf, "FWEngine2")) dev = ids[i];
    }
    if (!dev) { puts("no FW1814"); return 1; }
    if (argc > 2 && atoi(argv[2])) { UInt32 b = atoi(argv[2]); AudioObjectPropertyAddress ba = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, 0 }; AudioObjectSetPropertyData(dev, &ba, 0, NULL, sizeof b, &b); }
    UInt32 n = sizeof gRate; AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 }; AudioObjectGetPropertyData(dev, &ra, 0, NULL, &n, &gRate);
    n = sizeof gBuf; AudioObjectPropertyAddress ba = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, 0 }; AudioObjectGetPropertyData(dev, &ba, 0, NULL, &n, &gBuf);
    int secs = argc > 1 ? atoi(argv[1]) : 30;
    printf("rate %.0f buffer %u (period %.2f ms), %d s\n", gRate, gBuf, gBuf * 1000 / gRate, secs); fflush(stdout);
    AudioDeviceIOProcID pid; AudioDeviceCreateIOProcID(dev, ioproc, NULL, &pid); AudioDeviceStart(dev, pid);
    for (int s = 1; s <= secs; s++) { sleep(1); printf("t=%2ds calls %4llu max gap %6.2f ms  big gaps %llu  out jumps %llu  in jumps %llu\n", s, gSecCalls, gSecMax, gBigGaps, gOutJumps, gInJumps); gSecCalls = 0; gSecMax = 0; fflush(stdout); }
    AudioDeviceStop(dev, pid);
    printf("TOTAL calls %llu, longest gap %.2f ms, gaps > 3 periods: %llu, sample-time jumps out/in: %llu/%llu\n", gCalls, gMax, gBigGaps, gOutJumps, gInJumps);
    for (int i = 0; i < gNev; i++) printf("  gap %.1f ms at t=%.2f s\n", gEvents[i].gap_ms, gEvents[i].t_ms / 1000);
    return 0;
}
