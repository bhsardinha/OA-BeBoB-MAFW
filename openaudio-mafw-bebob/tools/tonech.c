// Bench tool (AP-7.3, channel order): plays a quiet sine on ONE CoreAudio output channel of our device, silence on every other channel,
// so the device's own meters (maprop peaks, slots 0-13 on the Audiophile) and the user's ears show which physical port that channel reaches.
//   tonech CHANNEL [SECONDS [DBFS [HZ]]]      1-based channel; defaults 4 s, -30 dBFS, 440 Hz.  OA_DEV=<part of the UID> picks the unit.
// Build: cc -o tonech tonech.c -framework CoreAudio -framework CoreFoundation
#include <CoreAudio/CoreAudio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static UInt32 gChan;            // 0-based target channel across all output buffers
static double gPhase, gStep, gAmp;

static OSStatus ioproc(AudioObjectID d, const AudioTimeStamp *n, const AudioBufferList *in, const AudioTimeStamp *it,
                       AudioBufferList *out, const AudioTimeStamp *ot, void *ctx)
{
    UInt32 base = 0, frames = 0;
    for (UInt32 b = 0; b < out->mNumberBuffers; b++) {
        AudioBuffer *buf = &out->mBuffers[b];
        UInt32 ch = buf->mNumberChannels; float *s = buf->mData;
        frames = buf->mDataByteSize / (ch * sizeof(float));
        memset(s, 0, buf->mDataByteSize);
        if (gChan >= base && gChan < base + ch) {
            double ph = gPhase;
            for (UInt32 f = 0; f < frames; f++) { s[f * ch + (gChan - base)] = (float)(gAmp * sin(ph)); ph += gStep; }
        }
        base += ch;
    }
    gPhase = fmod(gPhase + gStep * frames, 2 * M_PI);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: tonech CHANNEL [SECONDS [DBFS [HZ]]]\n"); return 2; }
    gChan = (UInt32)atoi(argv[1]) - 1;
    int secs = argc > 2 ? atoi(argv[2]) : 4;
    double db = argc > 3 ? atof(argv[3]) : -30.0, hz = argc > 4 ? atof(argv[4]) : 440.0;
    if (db > -12.0) db = -12.0;                                                  // never louder than -12 dBFS
    gAmp = pow(10.0, db / 20.0);
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    AudioObjectID dev = 0;
    for (UInt32 i = 0; i < size / sizeof *ids; i++) {
        CFStringRef uid = NULL; UInt32 n = sizeof uid;
        AudioObjectPropertyAddress ua = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        if (AudioObjectGetPropertyData(ids[i], &ua, 0, NULL, &n, &uid) || !uid) continue;
        char buf[256] = ""; CFStringGetCString(uid, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(uid);
        // our engines' UID is "com_m_audio_FWEngine2:0x<GUID>"; OA_DEV=<text of the UID, e.g. the GUID> picks one unit
        if (strstr(buf, "FWEngine2") && (!getenv("OA_DEV") || strcasestr(buf, getenv("OA_DEV")))) { dev = ids[i]; break; }
    }
    if (!dev) { puts("no OpenAudio FireWire device"); return 1; }
    Float64 rate = 48000; UInt32 rs = sizeof rate;
    AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    AudioObjectGetPropertyData(dev, &ra, 0, NULL, &rs, &rate);
    gStep = 2 * M_PI * hz / rate;
    AudioDeviceIOProcID pid;
    if (AudioDeviceCreateIOProcID(dev, ioproc, NULL, &pid) || AudioDeviceStart(dev, pid)) { puts("cannot start the device"); return 1; }
    printf("tone %.0f Hz %.0f dBFS on output channel %u for %d s (rate %.0f)\n", hz, db, gChan + 1, secs, rate); fflush(stdout);
    sleep(secs);
    AudioDeviceStop(dev, pid); AudioDeviceDestroyIOProcID(dev, pid);
    return 0;
}
