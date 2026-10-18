// Bench tool: finds glitches / repeats in what the FW1814 really outputs, using an analog loopback Out 3 -> In 1 (so outputs 1/2 stay free for headphones).
// Plays a quiet 440 Hz on outputs 1/2 (to listen to) and a steady 1 kHz sine on output 3; records input 1 and checks the sine for breaks: a pure sine obeys
// x[n] = 2cos(w) x[n-1] - x[n-2], so any discontinuity (a repeated or dropped stretch) shows as a large residual; also flags level dropouts.
// Events are printed with wall-clock time so they can be matched with what is done in another shell (afplay, rm ...).
//   loopcheck [SECONDS [OUT_CH [IN_CH]]]      defaults 30 s, output 3, input 1 (1-based)
// Build: cc -o loopcheck loopcheck.c -framework CoreAudio -framework CoreFoundation
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

static Float64 gRate; static int gOut = 2, gIn = 0;
static double gPhase, gPhaseLo; static float gX1, gX2; static double gPeak; static long gN, gLastEv = -10000, gQuiet;
static volatile int gNev, gDrop; static const int EVMAX = 200;
static struct { double wall; long n; float r, a; int kind; } gEv[200];
static float gBlockPeak; static int gBlockN;
static double now(void) { struct timeval t; gettimeofday(&t, NULL); return t.tv_sec + t.tv_usec / 1e6; }

static OSStatus ioproc(AudioObjectID d, const AudioTimeStamp *t, const AudioBufferList *in, const AudioTimeStamp *it, AudioBufferList *out, const AudioTimeStamp *ot, void *c) {
    (void)d; (void)t; (void)it; (void)ot; (void)c;
    if (out && out->mNumberBuffers) {
        float *o = out->mBuffers[0].mData; UInt32 ch = out->mBuffers[0].mNumberChannels, n = out->mBuffers[0].mDataByteSize / (ch * 4);
        for (UInt32 i = 0; i < n; i++) {
            float lo = 0.05f * sinf((float)gPhaseLo), hi = 0.5f * sinf((float)gPhase);
            gPhaseLo += 2 * M_PI * 440 / gRate; if (gPhaseLo > 2 * M_PI) gPhaseLo -= 2 * M_PI;
            gPhase += 2 * M_PI * 1000 / gRate;  if (gPhase > 2 * M_PI) gPhase -= 2 * M_PI;
            for (UInt32 k = 0; k < ch; k++) o[i * ch + k] = (k < 2) ? lo : ((int)k == gOut ? hi : 0);
        }
    }
    if (in && in->mNumberBuffers) {
        const float *x = in->mBuffers[0].mData; UInt32 ch = in->mBuffers[0].mNumberChannels, n = in->mBuffers[0].mDataByteSize / (ch * 4);
        const float cw = (float)(2 * cos(2 * M_PI * 1000 / gRate));
        for (UInt32 i = 0; i < n; i++) {
            float v = x[i * ch + gIn];
            gPeak = fmax(fabs(v), gPeak * 0.9995);                       // slow envelope of the level
            float r = v - (cw * gX1 - gX2); gX2 = gX1; gX1 = v; gN++;
            if (gN > 20000 && gPeak > 0.01) {
                if (fabsf(r) > 0.25f * gPeak && gN - gLastEv > 200 && gNev < EVMAX) { gEv[gNev].wall = now(); gEv[gNev].n = gN; gEv[gNev].r = r; gEv[gNev].a = (float)gPeak; gEv[gNev].kind = 0; gNev++; gLastEv = gN; }
                gBlockPeak = fmaxf(gBlockPeak, fabsf(v)); if (++gBlockN >= 220) {   // 5 ms blocks
                    if (gBlockPeak < 0.3f * gPeak && gNev < EVMAX && gN - gLastEv > 200) { gEv[gNev].wall = now(); gEv[gNev].n = gN; gEv[gNev].r = gBlockPeak; gEv[gNev].a = (float)gPeak; gEv[gNev].kind = 1; gNev++; gLastEv = gN; }
                    gBlockPeak = 0; gBlockN = 0; }
            }
        }
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
    int secs = argc > 1 ? atoi(argv[1]) : 30; if (argc > 2) gOut = atoi(argv[2]) - 1; if (argc > 3) gIn = atoi(argv[3]) - 1;
    UInt32 n = sizeof gRate; AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 }; AudioObjectGetPropertyData(dev, &ra, 0, NULL, &n, &gRate);
    printf("rate %.0f, %d s, 1 kHz on output %d -> input %d, 440 Hz (quiet) on outputs 1/2\n", gRate, secs, gOut + 1, gIn + 1); fflush(stdout);
    AudioDeviceIOProcID pid; AudioDeviceCreateIOProcID(dev, ioproc, NULL, &pid); AudioDeviceStart(dev, pid);
    double t0 = now();
    for (int s = 0; s < secs; s++) { sleep(1); if (s == 3) printf("  loop level (peak) %.3f at t=3 s%s\n", gPeak, gPeak < 0.01 ? "   <- NO SIGNAL: check the cable Out 3 -> In 1 and the input gain" : ""); fflush(stdout); }
    AudioDeviceStop(dev, pid);
    printf("events: %d (kind 0 = break in the sine, 1 = level dropout); wall clock | seconds into run\n", gNev);
    for (int i = 0; i < gNev; i++) { time_t tt = (time_t)gEv[i].wall; struct tm tmv; localtime_r(&tt, &tmv);
        printf("  %02d:%02d:%02d.%03d  +%6.2f s  %s  value %.3f (level %.3f)\n", tmv.tm_hour, tmv.tm_min, tmv.tm_sec, (int)((gEv[i].wall - floor(gEv[i].wall)) * 1000), gEv[i].wall - t0, gEv[i].kind ? "DROPOUT" : "BREAK  ", gEv[i].r, gEv[i].a); }
    return 0;
}
