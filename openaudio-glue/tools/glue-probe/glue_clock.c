// OpenAudio Glue - glue-clock: read-only, for every com_openaudio_FW1814Device: GUID, MAClockSource decoded the way the panel
// decodes it (openaudio-mafw-bebob/control-panel/Sources/MaKeys.swift: bits [1:0] sync, bit 2 S/PDIF coaxial, bit 3 optical
// in ADAT, bit 4 optical out ADAT, bit 5 S/MUX), MARateLocked, MAExternalSyncRate, MASampleRate. Reads through
// IORegistryEntryCreateCFProperty, served by the kext's copyProperty (as tools/maprop does). Writes nothing.
//   cc -mmacosx-version-min=10.13 glue_clock.c -framework IOKit -framework CoreFoundation -o glue-clock
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>

static long Num(io_service_t s, const char *key, int *ok)
{
    CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
    CFTypeRef v = IORegistryEntryCreateCFProperty(s, k, NULL, 0);
    CFRelease(k);
    long n = 0; *ok = 0;
    if (v && CFGetTypeID(v) == CFNumberGetTypeID()) { CFNumberGetValue((CFNumberRef)v, kCFNumberLongType, &n); *ok = 1; }
    if (v) CFRelease(v);
    return n;
}

int main(void)
{
    static const char *kSync[4] = { "Internal (digital inputs unmuted)", "External digital (S/PDIF or ADAT in)", "Word clock",
                                    "Internal (digital inputs muted)" };
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("com_openaudio_FW1814Device"), &it)) return 1;
    io_service_t s; int n = 0;
    while ((s = IOIteratorNext(it))) {
        int ok, okc, okl, oke, okr;
        const long guid = Num(s, "GUID", &ok);
        const long cs = Num(s, "MAClockSource", &okc), locked = Num(s, "MARateLocked", &okl);
        const long ext = Num(s, "MAExternalSyncRate", &oke), rate = Num(s, "MASampleRate", &okr);
        printf("unit GUID 0x%lX\n", (unsigned long)guid);
        if (okc) printf("  MAClockSource 0x%08lX: sync = %s; optical in = %s; optical out = %s; S/MUX %s; S/PDIF in %s\n",
                        (unsigned long)cs, kSync[cs & 3], (cs & 0x08) ? "ADAT" : "S/PDIF", (cs & 0x10) ? "ADAT" : "S/PDIF",
                        (cs & 0x20) ? "on" : "off", (cs & 0x04) ? "coaxial" : "optical");
        else printf("  MAClockSource: not readable\n");
        printf("  MARateLocked %s  MAExternalSyncRate %s%ld  MASampleRate %ld\n", okl ? (locked ? "1" : "0") : "?",
               oke ? "" : "? ", ext, rate);
        IOObjectRelease(s); n++;
    }
    IOObjectRelease(it);
    if (!n) puts("no com_openaudio_FW1814Device");
    return 0;
}
