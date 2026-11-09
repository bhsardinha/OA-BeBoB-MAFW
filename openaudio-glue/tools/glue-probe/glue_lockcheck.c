// OpenAudio Glue - glue-lockcheck: are two FW1814s on one clock? Runs an IOProc on every "FireWire 1814" device at once for
// N seconds (output silence, inputs ignored; the rate is never changed) and measures each device's real sample rate from its
// own timestamps (sample time over host time). Locked units agree to well under 1 ppm; free-running crystals differ by tens.
//   cc -mmacosx-version-min=10.13 glue_lockcheck.c -framework CoreAudio -framework CoreFoundation -o glue-lockcheck
//   ./glue-lockcheck [seconds]          (default 20)
#include <CoreAudio/CoreAudio.h>
#include <mach/mach_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct { AudioObjectID id; char name[128]; char uid[128]; AudioDeviceIOProcID proc;
                 double firstS, lastS; UInt64 firstH, lastH; int calls; } Dev;

static OSStatus Proc(AudioObjectID dev, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *inTime,
                     AudioBufferList *out, const AudioTimeStamp *outTime, void *ctx)
{
    (void)dev; (void)in; (void)outTime; (void)now;
    Dev *d = (Dev *)ctx;
    for (UInt32 i = 0; i < out->mNumberBuffers; i++) memset(out->mBuffers[i].mData, 0, out->mBuffers[i].mDataByteSize);
    if ((inTime->mFlags & kAudioTimeStampSampleHostTimeValid) == kAudioTimeStampSampleHostTimeValid) {
        if (d->calls == 0) { d->firstS = inTime->mSampleTime; d->firstH = inTime->mHostTime; }
        d->lastS = inTime->mSampleTime; d->lastH = inTime->mHostTime;
        d->calls++;
    }
    return noErr;
}

static void Str(AudioObjectID id, AudioObjectPropertySelector sel, char *buf, int n)
{
    AudioObjectPropertyAddress a = { sel, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    CFStringRef s = NULL; UInt32 sz = sizeof s; buf[0] = 0;
    if (AudioObjectGetPropertyData(id, &a, 0, NULL, &sz, &s) == noErr && s) { CFStringGetCString(s, buf, n, kCFStringEncodingUTF8); CFRelease(s); }
}

int main(int argc, char **argv)
{
    const int seconds = argc > 1 ? atoi(argv[1]) : 20;
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = (AudioObjectID *)malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    Dev devs[8]; int n = 0;
    for (UInt32 i = 0; i < size / sizeof(AudioObjectID) && n < 8; i++) {
        char uid[128]; Str(ids[i], kAudioDevicePropertyDeviceUID, uid, sizeof uid);
        if (strncmp(uid, "com_m_audio_FWEngine2:", 22) != 0) continue;   // the units themselves, not aggregates
        memset(&devs[n], 0, sizeof devs[n]);
        devs[n].id = ids[i];
        strcpy(devs[n].uid, uid);
        Str(ids[i], kAudioObjectPropertyName, devs[n].name, sizeof devs[n].name);
        n++;
    }
    if (n < 2) { printf("found %d FW1814 device(s); need two\n", n); return 1; }
    for (int k = 0; k < n; k++) {
        if (AudioDeviceCreateIOProcID(devs[k].id, Proc, &devs[k], &devs[k].proc) != noErr ||
            AudioDeviceStart(devs[k].id, devs[k].proc) != noErr) { printf("cannot start %s\n", devs[k].uid); return 1; }
    }
    printf("streaming silence on %d units for %d s...\n", n, seconds);
    sleep((unsigned)seconds);
    for (int k = 0; k < n; k++) { AudioDeviceStop(devs[k].id, devs[k].proc); AudioDeviceDestroyIOProcID(devs[k].id, devs[k].proc); }
    mach_timebase_info_data_t tb; mach_timebase_info(&tb);
    double rate[8];
    for (int k = 0; k < n; k++) {
        const double secs = (double)(devs[k].lastH - devs[k].firstH) * tb.numer / tb.denom * 1e-9;
        rate[k] = secs > 0 ? (devs[k].lastS - devs[k].firstS) / secs : 0;
        printf("%s: %d cycles, %.3f s, measured rate %.4f Hz\n", devs[k].uid, devs[k].calls, secs, rate[k]);
    }
    for (int k = 1; k < n; k++)
        printf("%s vs %s: %+.3f ppm\n", devs[k].uid + 22, devs[0].uid + 22, (rate[k] / rate[0] - 1.0) * 1e6);
    return 0;
}
