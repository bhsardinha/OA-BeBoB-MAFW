// OpenAudio Glue - glue-bypasscheck: with Glue bypassed (every stage Off, thresholds 0, EQ flat - a fresh install), a unit's Glue
// inputs must equal its own inputs bit for bit, delayed by Glue's fixed compensation. Records both devices of one unit at once for N
// seconds (IOProcs, output untouched, rate unchanged), places every buffer by its input sample time, then searches the lag
// -8..+8 at which all channels are identical. Read-only for the audio path; it only streams.
//   cc -mmacosx-version-min=10.13 glue_bypasscheck.c -framework CoreAudio -framework CoreFoundation -o glue-bypasscheck
//   ./glue-bypasscheck <GUID hex, e.g. 0xD6C040012337F> [seconds]
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <mach/mach_time.h>

#define MAXF (48000 * 30)
typedef struct { AudioObjectID id; AudioDeviceIOProcID proc; UInt32 ch; float *data; double first; UInt64 firstHost; long frames; int calls; } Rec;

static OSStatus Proc(AudioObjectID dev, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *inTime,
                     AudioBufferList *out, const AudioTimeStamp *outTime, void *ctx)
{
    (void)dev; (void)now; (void)out; (void)outTime;
    Rec *r = (Rec *)ctx;
    if (!in || in->mNumberBuffers < 1 || !(inTime->mFlags & kAudioTimeStampSampleTimeValid)) return noErr;
    const AudioBuffer *b = &in->mBuffers[0];
    const UInt32 ch = b->mNumberChannels, n = b->mDataByteSize / (4 * ch);
    if (r->calls == 0) { r->first = inTime->mSampleTime; r->firstHost = inTime->mHostTime; }
    r->calls++;
    const long at = (long)llround(inTime->mSampleTime - r->first);
    if (at < 0 || at + n >= MAXF || ch != r->ch) return noErr;
    memcpy(r->data + (size_t)at * ch, b->mData, (size_t)n * ch * 4);
    if (at + n > r->frames) r->frames = at + n;
    return noErr;
}

static AudioObjectID Find(const char *uid)
{
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    AudioObjectID found = 0;
    for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++) {
        AudioObjectPropertyAddress u = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        CFStringRef s = NULL; UInt32 sz = sizeof s; char buf[256] = "";
        if (AudioObjectGetPropertyData(ids[i], &u, 0, NULL, &sz, &s) == noErr && s) { CFStringGetCString(s, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(s); }
        if (!strcmp(buf, uid)) found = ids[i];
    }
    free(ids);
    return found;
}

static UInt32 InputChannels(AudioObjectID id)
{
    AudioObjectPropertyAddress a = { kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeInput, kAudioObjectPropertyElementMaster };
    UInt32 size = 0, ch = 0;
    AudioObjectGetPropertyDataSize(id, &a, 0, NULL, &size);
    AudioBufferList *bl = malloc(size);
    AudioObjectGetPropertyData(id, &a, 0, NULL, &size, bl);
    for (UInt32 i = 0; i < bl->mNumberBuffers; i++) ch += bl->mBuffers[i].mNumberChannels;
    free(bl);
    return ch;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: glue-bypasscheck <GUID hex> [seconds]\n"); return 2; }
    const int secs = argc > 2 ? atoi(argv[2]) : 10;
    char uidUnit[128], uidGlue[128];
    snprintf(uidUnit, sizeof uidUnit, "com_m_audio_FWEngine2:%s", argv[1]);
    snprintf(uidGlue, sizeof uidGlue, "com.openaudio.glue:%s", argv[1]);
    Rec r[2];
    memset(r, 0, sizeof r);
    r[0].id = Find(uidUnit); r[1].id = Find(uidGlue);
    if (!r[0].id || !r[1].id) { printf("device not found (%s / %s)\n", uidUnit, uidGlue); return 1; }
    for (int k = 0; k < 2; k++) {
        r[k].ch = InputChannels(r[k].id);
        r[k].data = calloc((size_t)MAXF * r[k].ch, 4);
        AudioDeviceCreateIOProcID(r[k].id, Proc, &r[k], &r[k].proc);
    }
    if (r[0].ch != r[1].ch) { printf("channel counts differ: unit %u, Glue %u\n", r[0].ch, r[1].ch); return 1; }
    // The unit's engine takes seconds to start (the reference's start sequence): start it, wait until it streams, then Glue.
    AudioDeviceStart(r[0].id, r[0].proc);
    for (int i = 0; i < 200 && r[0].calls < 50; i++) usleep(100000);
    AudioDeviceStart(r[1].id, r[1].proc);
    sleep((unsigned)secs);
    AudioDeviceStop(r[0].id, r[0].proc); AudioDeviceStop(r[1].id, r[1].proc);
    const UInt32 ch = r[0].ch;
    printf("unit: %d buffers, %ld frames from sample %.0f; Glue: %d buffers, %ld frames from sample %.0f\n",
           r[0].calls, r[0].frames, r[0].first, r[1].calls, r[1].frames, r[1].first);
    // The devices count laps from different starts, but share ring positions (sample time mod 8192) and wrap times. Glue's first
    // buffer, placed on the unit's timeline through host time, then rounded to whole 8192-frame laps of difference.
    mach_timebase_info_data_t tb; mach_timebase_info(&tb);
    const double ticksPerFrame = 1e9 * tb.denom / tb.numer / 48000.0;   // the nominal rate is enough to pick the lap
    const double unitAtGlueStart = r[0].first + ((double)r[1].firstHost - (double)r[0].firstHost) / ticksPerFrame;
    const double d = r[1].first - unitAtGlueStart;                       // Glue sample time minus unit sample time, same instant
    const double d8 = 8192.0 * llround(d / 8192.0);
    printf("timelines: Glue - unit = %.1f samples at the same host time; whole laps %.0f\n", d, d8);
    const long offset = (long)llround(r[1].first - d8 - r[0].first);     // Glue index 0 = unit index offset (lag 0)
    int bestLag = 999; long bestEqual = -1, bestTotal = 0; double bestMax = 0;
    for (int lag = -8; lag <= 8; lag++) {
        long eq = 0, total = 0; double maxd = 0;
        for (long t = 48000; t < r[1].frames - 48000; t++) {            // skip the first and last second (start-up, stop)
            const long u = t + offset - lag;
            if (u < 0 || u >= r[0].frames) continue;
            for (UInt32 c = 0; c < ch; c++) {
                const float a = r[1].data[(size_t)t * ch + c], b = r[0].data[(size_t)u * ch + c];
                total++;
                if (memcmp(&a, &b, 4) == 0) eq++;
                else if (fabs(a - b) > maxd) maxd = fabs(a - b);
            }
        }
        if (eq > bestEqual) { bestEqual = eq; bestLag = lag; bestTotal = total; bestMax = maxd; }
    }
    // per channel at the best lag; for a channel that differs, the lag at which it matches (if any)
    printf("per channel at lag %d:", bestLag);
    for (UInt32 c = 0; c < ch; c++) {
        long eqc = 0, tot = 0;
        for (long t = 48000; t < r[1].frames - 48000; t++) {
            const long u = t + offset - bestLag;
            if (u < 0 || u >= r[0].frames) continue;
            tot++;
            if (memcmp(&r[1].data[(size_t)t * ch + c], &r[0].data[(size_t)u * ch + c], 4) == 0) eqc++;
        }
        printf(" %u:%.2f%%", c + 1, tot ? 100.0 * eqc / tot : 0);
        if (tot && eqc != tot) {
            for (int lag = -40; lag <= 40; lag++) {
                long e2 = 0, t2 = 0;
                for (long t = 48000; t < r[1].frames - 48000; t += 7) {
                    const long u = t + offset - lag;
                    if (u < 0 || u >= r[0].frames) continue;
                    t2++;
                    if (memcmp(&r[1].data[(size_t)t * ch + c], &r[0].data[(size_t)u * ch + c], 4) == 0) e2++;
                }
                if (t2 && e2 == t2) { printf("(matches at lag %d)", lag); break; }
            }
            for (UInt32 c2 = 0; c2 < ch; c2++) {                     // or is it another unit channel?
                long e3 = 0, t3 = 0;
                for (long t = 48000; t < r[1].frames - 48000; t += 7) {
                    const long u = t + offset - bestLag;
                    if (u < 0 || u >= r[0].frames) continue;
                    t3++;
                    if (memcmp(&r[1].data[(size_t)t * ch + c], &r[0].data[(size_t)u * ch + c2], 4) == 0) e3++;
                }
                if (c2 != c && t3 && e3 == t3 && r[0].data[(size_t)60000 * ch + c2] != 0.0f) printf("(equals unit channel %u)", c2 + 1);
            }
        }
    }
    printf("\n");
    long nonzero[64] = {0};
    for (long t = 48000; t < r[0].frames - 48000; t++) for (UInt32 c = 0; c < ch && c < 64; c++) if (r[0].data[(size_t)t * ch + c] != 0.0f) nonzero[c]++;
    printf("best lag %d sample(s): %ld of %ld samples bit-identical (%.6f %%), largest difference elsewhere %.3g\n",
           bestLag, bestEqual, bestTotal, bestTotal ? 100.0 * bestEqual / bestTotal : 0, bestMax);
    printf("signal present (non-zero samples per channel, unit side):");
    for (UInt32 c = 0; c < ch; c++) printf(" %ld", nonzero[c]);
    printf("\n");
    return 0;
}
