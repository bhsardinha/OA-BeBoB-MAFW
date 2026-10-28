// Bench tool for the integrated run: read and write MA* properties of the device (generic base com_openaudio_FWAVCAudioDevice, any model) through the same IOKit calls the panel uses.
//   maprop all                 -> MAMetaAllProperties (the umbrella dictionary the kext serves through copyProperty), key count and a listing
//   maprop allxml              -> the same dictionary as an XML plist (input of tools/factory-diff.py)
//   maprop get KEY             -> IORegistryEntryCreateCFProperty (served by the kext's copyProperty override)
//   maprop set KEY N [N ...]   -> IORegistryEntrySetCFProperties with a number, or an array when several numbers are given
//   maprop setxml FILE         -> writes an `allxml` dump back with one IORegistryEntrySetCFProperties, as the helper's replay does (Settings.c
//                                 loadPrefs); the session keys (rate, lock, bandwidth, engine options) are left out, so no rate change happens
//   MA_CLASS=com_m_audio_FW1814Device maprop ...  -> the same against the official driver (Snow Leopard reference rig)
//   MA_GUID=74B9FB maprop ...  -> the unit whose GUID ends with these hex digits (two units of the same model)
//   maprop peaks [N]           -> open the user client (type 0), map memory type 1 (the meter page) and print the 52 raw peaks N times
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// The first unit of MA_CLASS, or with MA_GUID=<hex tail, e.g. 74B9FB> the unit whose GUID ends with it (two units of one model).
static io_service_t device(void) {
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(getenv("MA_CLASS") ? getenv("MA_CLASS") : "com_openaudio_FWAVCAudioDevice"), &it)) return 0;
    const char *want = getenv("MA_GUID");
    io_service_t s, found = 0;
    while ((s = IOIteratorNext(it))) {
        if (!want) { found = s; break; }
        CFNumberRef g = IORegistryEntryCreateCFProperty(s, CFSTR("GUID"), kCFAllocatorDefault, 0);
        unsigned long long v = 0; char hex[32];
        if (g) { CFNumberGetValue(g, kCFNumberSInt64Type, &v); CFRelease(g); }
        snprintf(hex, sizeof hex, "%llX", v);
        size_t lh = strlen(hex), lw = strlen(want);
        if (lw <= lh && strcasecmp(hex + lh - lw, want) == 0) { found = s; break; }
        IOObjectRelease(s);
    }
    IOObjectRelease(it); return found;
}
static void show(const char *label, CFTypeRef v) {
    CFStringRef d = v ? CFCopyDescription(v) : CFSTR("(null)");
    char buf[4096]; CFStringGetCString(d, buf, sizeof buf, kCFStringEncodingUTF8);
    printf("%s = %s\n", label, buf); if (v) CFRelease(d);
}
int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: maprop all | allxml | get KEY | set KEY N... | setxml FILE | peaks [N]\n"); return 2; }
    io_service_t s = device();
    if (!s) { puts("no com_openaudio_FWAVCAudioDevice (any model) in the registry"); return 1; }
    if (!strcmp(argv[1], "all")) {
        CFTypeRef v = IORegistryEntryCreateCFProperty(s, CFSTR("MAMetaAllProperties"), NULL, 0);
        if (!v || CFGetTypeID(v) != CFDictionaryGetTypeID()) { show("MAMetaAllProperties", v); return 1; }
        printf("MAMetaAllProperties: %ld keys\n", (long)CFDictionaryGetCount(v));
        CFIndex n = CFDictionaryGetCount(v); const void **k = malloc(n * sizeof *k), **vals = malloc(n * sizeof *vals);
        CFDictionaryGetKeysAndValues(v, k, vals);
        for (CFIndex i = 0; i < n; i++) { char kb[128]; CFStringGetCString(k[i], kb, sizeof kb, kCFStringEncodingUTF8); show(kb, vals[i]); }
        return 0;
    }
    if (!strcmp(argv[1], "allxml")) {                // MAMetaAllProperties as an XML plist on stdout (tools/factory-diff.py)
        CFTypeRef v = IORegistryEntryCreateCFProperty(s, CFSTR("MAMetaAllProperties"), NULL, 0);
        if (!v) { fputs("no MAMetaAllProperties\n", stderr); return 1; }
        CFDataRef x = CFPropertyListCreateData(NULL, v, kCFPropertyListXMLFormat_v1_0, 0, NULL);
        if (!x) return 1;
        fwrite(CFDataGetBytePtr(x), 1, (size_t)CFDataGetLength(x), stdout);
        return 0;
    }
    if (!strcmp(argv[1], "get") && argc >= 3) {
        CFStringRef k = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingUTF8);
        CFTypeRef v = IORegistryEntryCreateCFProperty(s, k, NULL, 0); show(argv[2], v); return v ? 0 : 1;
    }
    if (!strcmp(argv[1], "set") && argc >= 4) {
        CFStringRef k = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingUTF8);
        CFMutableArrayRef arr = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
        for (int i = 3; i < argc; i++) { long long x = atoll(argv[i]); CFNumberRef n = CFNumberCreate(NULL, kCFNumberLongLongType, &x); CFArrayAppendValue(arr, n); }
        CFTypeRef value = argc == 4 ? CFArrayGetValueAtIndex(arr, 0) : (CFTypeRef)arr;
        CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFDictionarySetValue(d, k, value);
        kern_return_t kr = IORegistryEntrySetCFProperties(s, d);
        printf("set %s -> 0x%x\n", argv[2], kr); return kr != 0;
    }
    if (!strcmp(argv[1], "setxml") && argc >= 3) {
        CFURLRef u = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)argv[2], (CFIndex)strlen(argv[2]), false);
        CFReadStreamRef rs = u ? CFReadStreamCreateWithFile(NULL, u) : NULL;
        if (!rs || !CFReadStreamOpen(rs)) { fprintf(stderr, "cannot open %s\n", argv[2]); return 1; }
        CFPropertyListRef pl = CFPropertyListCreateWithStream(NULL, rs, 0, kCFPropertyListImmutable, NULL, NULL);
        if (!pl || CFGetTypeID(pl) != CFDictionaryGetTypeID()) { fprintf(stderr, "%s is not a dictionary plist\n", argv[2]); return 1; }
        CFMutableDictionaryRef d = CFDictionaryCreateMutableCopy(NULL, 0, pl);
        static const char *skip[] = { "MASampleRate", "MARateLocked", "MABytesPerCycle", "MAEnableAggregateEngine", "MAEnableStereoPairEngines" };
        for (unsigned i = 0; i < sizeof skip / sizeof *skip; i++) {
            CFStringRef k = CFStringCreateWithCString(NULL, skip[i], kCFStringEncodingUTF8); CFDictionaryRemoveValue(d, k); CFRelease(k);
        }
        // MAMeta* are actions (reset, virtual knob): never part of a dump (no getter), dropped anyway so a hand-edited file cannot reset the unit
        CFIndex n = CFDictionaryGetCount(d); const void **ks = malloc(n * sizeof *ks); CFDictionaryGetKeysAndValues(d, ks, NULL);
        for (CFIndex i = 0; i < n; i++) if (CFStringHasPrefix(ks[i], CFSTR("MAMeta"))) CFDictionaryRemoveValue(d, ks[i]);
        free(ks);
        kern_return_t kr = IORegistryEntrySetCFProperties(s, d);
        printf("setxml %s: %ld keys -> 0x%x\n", argv[2], (long)CFDictionaryGetCount(d), kr); return kr != 0;
    }
    if (!strcmp(argv[1], "peaks")) {
        int times = argc >= 3 ? atoi(argv[2]) : 1;
        io_connect_t c; kern_return_t kr = IOServiceOpen(s, mach_task_self(), 0, &c);
        printf("IOServiceOpen -> 0x%x\n", kr); if (kr) return 1;
        mach_vm_address_t a = 0; mach_vm_size_t len = 0;
        kr = IOConnectMapMemory64(c, 1, mach_task_self(), &a, &len, 0x1001);
        printf("IOConnectMapMemory64(type 1) -> 0x%x, %llu bytes\n", kr, (unsigned long long)len); if (kr) return 1;
        for (int t = 0; t < times; t++) {
            const unsigned *p = (const unsigned *)(uintptr_t)a;
            for (int i = 0; i < 52; i++) printf("%u%s", p[i], i == 17 || i == 37 ? " | " : " ");
            printf("\n"); usleep(200000);
        }
        IOServiceClose(c); return 0;
    }
    fprintf(stderr, "usage: maprop all | allxml | get KEY | set KEY N... | setxml FILE | peaks [N]\n"); return 2;
}
