// OpenAudio Glue - glue-settings: prints a Glue device's settings (custom property 'glst') and can set only its "bypass" key, the rest
// staying as it is (the plug-in merges a partial plist, driver/src/GlueSettings.h). Used to check bypass without changing a user's setup:
// note the value it prints, set bypass 1, check, put the value back. The plug-in persists every write.
//   cc -mmacosx-version-min=10.13 glue_settings.c -framework CoreAudio -framework CoreFoundation -o glue-settings
//   ./glue-settings <GUID hex, e.g. 0xD6C040012337F> [bypass 0|1]
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static double num(CFDictionaryRef d, const char *k)
{
    CFStringRef key = CFStringCreateWithCString(NULL, k, kCFStringEncodingUTF8);
    CFTypeRef v = CFDictionaryGetValue(d, key);
    CFRelease(key);
    double x = -1;
    if (v && CFGetTypeID(v) == CFNumberGetTypeID()) CFNumberGetValue((CFNumberRef)v, kCFNumberDoubleType, &x);
    return x;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: glue-settings <GUID hex> [bypass 0|1]\n"); return 2; }
    char uid[128]; snprintf(uid, sizeof uid, "com.openaudio.glue:%s", argv[1]);
    CFStringRef s = CFStringCreateWithCString(NULL, uid, kCFStringEncodingUTF8);
    AudioObjectID dev = 0; UInt32 size = sizeof dev;
    AudioObjectPropertyAddress t = { kAudioHardwarePropertyTranslateUIDToDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &t, sizeof s, &s, &size, &dev) || !dev) { fprintf(stderr, "no device %s\n", uid); return 1; }
    AudioObjectPropertyAddress a = { 'glst', kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    if (argc >= 4 && !strcmp(argv[2], "bypass")) {
        int v = atoi(argv[3]);
        CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &v);
        CFDictionaryRef d = CFDictionaryCreate(NULL, (const void *[]){ CFSTR("bypass") }, (const void *[]){ n }, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        OSStatus e = AudioObjectSetPropertyData(dev, &a, 0, NULL, sizeof d, &d);
        if (e) { fprintf(stderr, "set failed %d\n", (int)e); return 1; }
    }
    CFPropertyListRef p = NULL; size = sizeof p;
    OSStatus e = AudioObjectGetPropertyData(dev, &a, 0, NULL, &size, &p);
    if (e || !p) { fprintf(stderr, "get failed %d\n", (int)e); return 1; }
    CFDictionaryRef d = (CFDictionaryRef)p;
    printf("bypass %d  drive %.1f  stages", (int)num(d, "bypass"), num(d, "drive"));
    CFArrayRef st = (CFArrayRef)CFDictionaryGetValue(d, CFSTR("stages"));
    for (CFIndex i = 0; st && i < CFArrayGetCount(st); i++) { int v = 0; CFNumberGetValue((CFNumberRef)CFArrayGetValueAtIndex(st, i), kCFNumberIntType, &v); printf(" %d", v); }
    printf("\n");
    CFArrayRef ch = (CFArrayRef)CFDictionaryGetValue(d, CFSTR("channels"));
    for (CFIndex i = 0; ch && i < CFArrayGetCount(ch); i++) {
        CFDictionaryRef c = (CFDictionaryRef)CFArrayGetValueAtIndex(ch, i);
        printf("ch%-2ld on %d thr %.1f src %d link %d\n", (long)i + 1, (int)num(c, "on"), num(c, "thr"), (int)num(c, "src"), (int)num(c, "link"));
    }
    return 0;
}
