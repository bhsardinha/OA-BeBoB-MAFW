// Minimal stand-in for the helper's MAFirmwareStart write (A4 H2/H11): sets <key> (default MAFirmwareStart)=true on every
// com_openaudio_FWMetaNub via IORegistryEntrySetCFProperties. M-A test aid only; the real helper is M-FW.
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

int main(int argc, char **argv) {
    const char *key = argc > 1 ? argv[1] : "MAFirmwareStart";
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("com_openaudio_FWMetaNub"), &it)) return 2;
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
    CFDictionarySetValue(d, k, kCFBooleanTrue);
    int n = 0; io_service_t s;
    while ((s = IOIteratorNext(it))) {
        kern_return_t kr = IORegistryEntrySetCFProperties(s, d);
        printf("nub %u: IORegistryEntrySetCFProperties -> 0x%x\n", s, kr);
        IOObjectRelease(s); n++;
    }
    IOObjectRelease(it); CFRelease(d);
    if (!n) { puts("no com_openaudio_FWMetaNub in the registry"); return 1; }
    return 0;
}
