// OpenAudio FW1814 helper (M-FW).
//
// Mirrors the "no update needed" branch of -[MAFireWireLoader deviceMatched:] (A4 H2,
// a4-h2-bootloader-detection.log:92-213): watch com_openaudio_FWMetaNub; on every match write
// MAFirmwareStart on the nub. That write is what makes the nub register a second time so the product class
// starts (Q-SDK5), and, for a device sitting in the bootloader, makes the kext send Go (Q-P3 §4).
//
// Scope cut (user decision 2026-10-15): this driver never writes the device's flash. The reference helper's
// firmware-update branch (bundled .bcd, date comparison, MAFirmwareHalt / MAFirmwareUpgradeImage, one-shot and
// inhibit files) is not implemented. It is recoverable from git history.
//
// Not in this version (BUILD-PLAN §4, later): system-sleep gating, console-user gating (H2 gates deviceMatched on
// both), status UI (H5), settings replay (H11).
//
// Options:  --once   handle the nubs present now, then exit (no run loop)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include <unistd.h>

static const char *kNubClass = "com_openaudio_FWMetaNub";
static const char *kDeviceClass = "com_openaudio_FW1814Device";
static bool sGated = true;                           // --no-gate disables H.3 for bench runs


static uint32_t le32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }

static void handle(io_service_t svc) {
    if (sGated && !OAGate_Open()) {                  // H.3: no writes during sleep or without a console user; re-walked on wake / login
        printf("nub matched but gated (sleeping=%d); deferred\n", OAGate_SystemSleeping());
        return;
    }
    char product[256] = "?";
    CFTypeRef pn = IORegistryEntryCreateCFProperty(svc, CFSTR("FireWire Product Name"), NULL, 0);
    if (pn && CFGetTypeID(pn) == CFStringGetTypeID())
        CFStringGetCString((CFStringRef)pn, product, sizeof(product), kCFStringEncodingUTF8);
    if (pn) CFRelease(pn);

    CFTypeRef info = IORegistryEntryCreateCFProperty(svc, CFSTR("MAFirmwareInfo"), NULL, 0);
    if (info && CFGetTypeID(info) == CFDataGetTypeID() && CFDataGetLength(info) >= 0x30) {
        const uint8_t *ib = CFDataGetBytePtr(info);                 // Q-P3 §3: [0x0C] = bootloader version, [0x20..] = date/time
        printf("nub matched: '%s' state=%s firmware %.8s %.6s\n", product,
               le32(ib + 0x0C) ? "bootloader" : "running", ib + 0x20, ib + 0x28);
    } else {
        printf("nub matched: '%s' (firmware info not available)\n", product);   // ref logs the same and returns; we still start
    }
    if (info) CFRelease(info);

    kern_return_t kr = IORegistryEntrySetCFProperty(svc, CFSTR("MAFirmwareStart"), CFSTR("YES"));   // ref writes @"YES"
    printf("  wrote MAFirmwareStart -> 0x%x\n", kr);
    fflush(stdout);
}

static void matched(void *refcon, io_iterator_t it) {
    (void)refcon;
    io_service_t s;
    while ((s = IOIteratorNext(it))) { handle(s); IOObjectRelease(s); }
}

static void deviceMatched(void *refcon, io_iterator_t it) {
    (void)refcon;
    io_service_t s;
    while ((s = IOIteratorNext(it))) { OASettings_DeviceAdded(s); IOObjectRelease(s); }
}

// re-walk what is already present (wake / login): firmware start for nubs, settings replay for devices
static void rewalk(void *ctx) {
    (void)ctx;
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(kNubClass), &it) == KERN_SUCCESS) { matched(NULL, it); IOObjectRelease(it); }
    OASettings_ReplayAll();
}
static void loggedOut(void *ctx) { (void)ctx; OASettings_FlushAll(); }

int main(int argc, char **argv) {
    int once = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--once")) once = 1;
        else if (!strcmp(argv[i], "--no-gate")) sGated = false;
        else { fprintf(stderr, "usage: %s [--once] [--no-gate]\n", argv[0]); return 2; }
    }
    IONotificationPortRef port = IONotificationPortCreate(kIOMasterPortDefault);
    io_iterator_t it;
    kern_return_t kr = IOServiceAddMatchingNotification(port, kIOFirstMatchNotification,
                                                        IOServiceMatching(kNubClass), matched, NULL, &it);
    if (kr) { fprintf(stderr, "IOServiceAddMatchingNotification: 0x%x\n", kr); return 1; }
    matched(NULL, it);                                   // arm + drain existing nubs
    if (once) return 0;
    OASettings_Init();
    io_iterator_t dit;
    kr = IOServiceAddMatchingNotification(port, kIOFirstMatchNotification, IOServiceMatching(kDeviceClass), deviceMatched, NULL, &dit);
    if (kr) { fprintf(stderr, "device notification: 0x%x\n", kr); return 1; }
    deviceMatched(NULL, dit);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(port), kCFRunLoopDefaultMode);
    OAGateCallbacks cb = { rewalk, rewalk, loggedOut, NULL };
    OAGate_Start(&cb);
    CFRunLoopRun();
    OASettings_FlushAll();
    return 0;
}
