// Bench tool for the integrated run: read and write MA* properties of com_openaudio_FW1814Device through the same IOKit calls the panel uses.
//   maprop all                 -> MAMetaAllProperties (the umbrella dictionary the kext serves through copyProperty), key count and a listing
//   maprop get KEY             -> IORegistryEntryCreateCFProperty (served by the kext's copyProperty override)
//   maprop set KEY N [N ...]   -> IORegistryEntrySetCFProperties with a number, or an array when several numbers are given
//   MA_CLASS=com_m_audio_FW1814Device maprop ...  -> the same against the official driver (Snow Leopard reference rig)
//   maprop peaks [N]           -> open the user client (type 0), map memory type 1 (the meter page) and print the 52 raw peaks N times
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static io_service_t device(void) {
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(getenv("MA_CLASS") ? getenv("MA_CLASS") : "com_openaudio_FW1814Device"), &it)) return 0;
    io_service_t s = IOIteratorNext(it); IOObjectRelease(it); return s;
}
static void show(const char *label, CFTypeRef v) {
    CFStringRef d = v ? CFCopyDescription(v) : CFSTR("(null)");
    char buf[4096]; CFStringGetCString(d, buf, sizeof buf, kCFStringEncodingUTF8);
    printf("%s = %s\n", label, buf); if (v) CFRelease(d);
}
int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: maprop all | get KEY | set KEY N...\n"); return 2; }
    io_service_t s = device();
    if (!s) { puts("no com_openaudio_FW1814Device in the registry"); return 1; }
    if (!strcmp(argv[1], "all")) {
        CFTypeRef v = IORegistryEntryCreateCFProperty(s, CFSTR("MAMetaAllProperties"), NULL, 0);
        if (!v || CFGetTypeID(v) != CFDictionaryGetTypeID()) { show("MAMetaAllProperties", v); return 1; }
        printf("MAMetaAllProperties: %ld keys\n", (long)CFDictionaryGetCount(v));
        CFIndex n = CFDictionaryGetCount(v); const void **k = malloc(n * sizeof *k), **vals = malloc(n * sizeof *vals);
        CFDictionaryGetKeysAndValues(v, k, vals);
        for (CFIndex i = 0; i < n; i++) { char kb[128]; CFStringGetCString(k[i], kb, sizeof kb, kCFStringEncodingUTF8); show(kb, vals[i]); }
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
    fprintf(stderr, "usage: maprop all | get KEY | set KEY N... | peaks [N]\n"); return 2;
}
