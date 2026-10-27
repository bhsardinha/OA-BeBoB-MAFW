// Bench tool: walks the FW1814 through a list of sample rates with CoreAudio (what the panel and Audio MIDI Setup do) and prints, after each
// change, the HAL's result code and the input/output channel totals. Works against the official driver on the SL rig and the clone.
//   ratecycle 48000 96000 192000 48000
// Build: cc -o ratecycle ratecycle.c -framework CoreAudio -framework CoreFoundation   (SL: -arch i386 -arch x86_64 -mmacosx-version-min=10.6)
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static UInt32 channels(AudioObjectID dev, AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress a = { kAudioDevicePropertyStreamConfiguration, scope, 0 };
    UInt32 size = 0, n = 0;
    if (AudioObjectGetPropertyDataSize(dev, &a, 0, NULL, &size) || !size) return 0;
    AudioBufferList *l = malloc(size);
    if (!AudioObjectGetPropertyData(dev, &a, 0, NULL, &size, l)) for (UInt32 i = 0; i < l->mNumberBuffers; i++) n += l->mBuffers[i].mNumberChannels;
    free(l);
    return n;
}
static Float64 rate(AudioObjectID dev) {
    AudioObjectPropertyAddress a = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 };
    Float64 r = 0; UInt32 n = sizeof r; AudioObjectGetPropertyData(dev, &a, 0, NULL, &n, &r); return r;
}
int main(int argc, char **argv) {
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, 0 };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = malloc(size), dev = 0;
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++) {
        CFStringRef uid = NULL; UInt32 n = sizeof uid;
        AudioObjectPropertyAddress ua = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, 0 };
        AudioObjectGetPropertyData(ids[i], &ua, 0, NULL, &n, &uid);
        char buf[256] = ""; if (uid) CFStringGetCString(uid, buf, sizeof buf, kCFStringEncodingUTF8);
        if (strstr(buf, "com_m_audio_FWEngine")) dev = ids[i];   // FWEngine2 (FW1814, Audiophile) or FWEngine1 (FW410 UID, R135)
    }
    if (!dev) { puts("no FW1814 in CoreAudio"); return 1; }
    printf("start: rate %.0f  in %u  out %u\n", rate(dev), (unsigned)channels(dev, kAudioObjectPropertyScopeInput), (unsigned)channels(dev, kAudioObjectPropertyScopeOutput));
    for (int i = 1; i < argc; i++) {
        Float64 r = atof(argv[i]);
        AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 };
        OSStatus st = AudioObjectSetPropertyData(dev, &ra, 0, NULL, sizeof r, &r);
        int waited = 0;                                       // the change completes asynchronously (several seconds on the SL rig): poll up to 30 s
        while (waited < 30 && (int)rate(dev) != (int)r) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, false); waited++; }   // the SL's HAL applies the request only while the run loop spins
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);   // let the channel counts settle
        printf("-> %.0f: status %d  after %d s: rate %.0f  in %u  out %u\n", r, (int)st, waited, rate(dev), (unsigned)channels(dev, kAudioObjectPropertyScopeInput), (unsigned)channels(dev, kAudioObjectPropertyScopeOutput));
        fflush(stdout);
    }
    return 0;
}
