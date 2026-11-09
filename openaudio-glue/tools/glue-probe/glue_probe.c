// OpenAudio Glue - glue-probe: read-only listing of every Core Audio device (UID, transport, clock domain, channels, rate,
// latency, safety offset) and of each aggregate's composition (sub-devices, clock master, drift correction). It changes
// nothing. Used for G5 (clock domain) and the Dual-aggregate design (docs/PROVENANCE_v1.md).
//   cc -mmacosx-version-min=10.13 glue_probe.c -framework CoreAudio -framework CoreFoundation -o glue-probe
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>

static int Get(AudioObjectID id, AudioObjectPropertySelector sel, AudioObjectPropertyScope scope, void *out, UInt32 size)
{
    AudioObjectPropertyAddress a = { sel, scope, kAudioObjectPropertyElementMaster };
    if (!AudioObjectHasProperty(id, &a)) return 0;
    return AudioObjectGetPropertyData(id, &a, 0, NULL, &size, out) == noErr;
}

static void Str(AudioObjectID id, AudioObjectPropertySelector sel, char *buf, int n)
{
    CFStringRef s = NULL;
    buf[0] = 0;
    if (Get(id, sel, kAudioObjectPropertyScopeGlobal, &s, sizeof s) && s) { CFStringGetCString(s, buf, n, kCFStringEncodingUTF8); CFRelease(s); }
}

static UInt32 Channels(AudioObjectID id, AudioObjectPropertyScope scope)
{
    AudioObjectPropertyAddress a = { kAudioDevicePropertyStreamConfiguration, scope, kAudioObjectPropertyElementMaster };
    UInt32 size = 0, ch = 0;
    if (AudioObjectGetPropertyDataSize(id, &a, 0, NULL, &size) != noErr || size == 0) return 0;
    AudioBufferList *bl = (AudioBufferList *)malloc(size);
    if (AudioObjectGetPropertyData(id, &a, 0, NULL, &size, bl) == noErr)
        for (UInt32 i = 0; i < bl->mNumberBuffers; i++) ch += bl->mBuffers[i].mNumberChannels;
    free(bl);
    return ch;
}

static void FourCC(UInt32 v, char out[5]) { out[0] = (char)(v >> 24); out[1] = (char)(v >> 16); out[2] = (char)(v >> 8); out[3] = (char)v; out[4] = 0; }

int main(void)
{
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
    AudioObjectID *ids = (AudioObjectID *)malloc(size);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
    for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++) {
        AudioObjectID id = ids[i];
        char name[256], uid[256], t[5];
        UInt32 transport = 0, domain = 0, inLat = 0, outLat = 0, inSafe = 0, outSafe = 0, buf = 0;
        Float64 rate = 0;
        Str(id, kAudioObjectPropertyName, name, sizeof name);
        Str(id, kAudioDevicePropertyDeviceUID, uid, sizeof uid);
        Get(id, kAudioDevicePropertyTransportType, kAudioObjectPropertyScopeGlobal, &transport, sizeof transport);
        const int hasDomain = Get(id, kAudioDevicePropertyClockDomain, kAudioObjectPropertyScopeGlobal, &domain, sizeof domain);
        Get(id, kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, &rate, sizeof rate);
        Get(id, kAudioDevicePropertyLatency, kAudioDevicePropertyScopeInput, &inLat, sizeof inLat);
        Get(id, kAudioDevicePropertyLatency, kAudioDevicePropertyScopeOutput, &outLat, sizeof outLat);
        Get(id, kAudioDevicePropertySafetyOffset, kAudioDevicePropertyScopeInput, &inSafe, sizeof inSafe);
        Get(id, kAudioDevicePropertySafetyOffset, kAudioDevicePropertyScopeOutput, &outSafe, sizeof outSafe);
        Get(id, kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, &buf, sizeof buf);
        FourCC(transport, t);
        printf("%s\n  uid %s\n  transport '%s'  clock domain %s%u  rate %.0f  in %u / out %u  buffer %u\n"
               "  latency in %u out %u  safety offset in %u out %u\n",
               name, uid, t, hasDomain ? "" : "(none) ", domain, rate, Channels(id, kAudioDevicePropertyScopeInput),
               Channels(id, kAudioDevicePropertyScopeOutput), buf, inLat, outLat, inSafe, outSafe);
        if (transport == kAudioDeviceTransportTypeAggregate) {
            CFArrayRef subs = NULL;
            if (Get(id, kAudioAggregateDevicePropertyFullSubDeviceList, kAudioObjectPropertyScopeGlobal, &subs, sizeof subs) && subs) {
                for (CFIndex k = 0; k < CFArrayGetCount(subs); k++) {
                    char s[256]; CFStringGetCString((CFStringRef)CFArrayGetValueAtIndex(subs, k), s, sizeof s, kCFStringEncodingUTF8);
                    printf("  sub-device %ld: %s\n", (long)k, s);
                }
                CFRelease(subs);
            }
            char master[256]; Str(id, kAudioAggregateDevicePropertyMasterSubDevice, master, sizeof master);
            printf("  clock master: %s\n", master);
            CFDictionaryRef comp = NULL;
            if (Get(id, kAudioAggregateDevicePropertyComposition, kAudioObjectPropertyScopeGlobal, &comp, sizeof comp) && comp) {
                CFArrayRef list = (CFArrayRef)CFDictionaryGetValue(comp, CFSTR(kAudioAggregateDeviceSubDeviceListKey));
                for (CFIndex k = 0; list && k < CFArrayGetCount(list); k++) {
                    CFDictionaryRef d = (CFDictionaryRef)CFArrayGetValueAtIndex(list, k);
                    CFNumberRef drift = (CFNumberRef)CFDictionaryGetValue(d, CFSTR(kAudioSubDeviceDriftCompensationKey));
                    int dv = -1; if (drift) CFNumberGetValue(drift, kCFNumberIntType, &dv);
                    printf("  sub-device %ld drift compensation: %d\n", (long)k, dv);
                }
                CFRelease(comp);
            }
        }
    }
    free(ids);
    return 0;
}
