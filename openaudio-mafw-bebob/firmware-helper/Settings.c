// H.4 -- per-GUID settings persistence and replay (A4 H11).
// Reference: -[MAFireWireDeviceInstance commitSettings:] @0x2892, -loadPrefsForUser: @0x2afe, -receivedPropertyChangeMessage: @0x26df;
// appID com.m-audio.device-settings.firewire; the kext's type-0 notification port delivers the dirty properties (Q-MF); changes are
// merged into the device's settings dictionary and written 4 s after the LAST change under the console user's identity. On attach and
// at login the saved dictionary is pushed with ONE IORegistryEntrySetCFProperties; with nothing saved the device's MAMetaAllProperties
// seeds the dictionary. MAMetaConfigureDCLLatency is NOT implemented (dead code in the reference, H11).
#include "helper.h"
#include <IOKit/IOCFUnserialize.h>
#include <IOKit/IOMessage.h>
#include <mach/mach.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const CFStringRef kAppID = CFSTR("com.m-audio.device-settings.firewire");
static const double kCommitDelay = 4.0;

typedef struct Device {
    struct Device *next;
    io_service_t svc;
    io_connect_t conn;
    CFStringRef key;                       // canonical GUID, upper-case hex without 0x (ref CanonicalGUIDForDevice)
    CFMutableDictionaryRef settings;       // ref fSettings [+0xc]
    bool dirty;                            // ref [+0x18]
    CFRunLoopTimerRef timer;
    CFMachPortRef port;
    CFRunLoopSourceRef portSrc;
    io_object_t notifier;                  // interest notification: the device going away (ref -service:changedProperties: / instance release)
} Device;

static Device *sDevices;
static IONotificationPortRef sNotifyPort;

static CFStringRef guidKey(io_service_t svc)
{
    CFTypeRef g = IORegistryEntryCreateCFProperty(svc, CFSTR("GUID"), NULL, 0);
    if (!g) return NULL;
    CFStringRef r = NULL;
    uint64_t v = 0;
    if (CFGetTypeID(g) == CFNumberGetTypeID() && CFNumberGetValue(g, kCFNumberSInt64Type, &v))
        r = CFStringCreateWithFormat(NULL, NULL, CFSTR("%llX"), v);
    CFRelease(g);
    return r;
}

// ref -commitSettings: -- run the preference write as the console user
static void commit(Device *d)
{
    if (!d->dirty) return;
    char name[128]; uid_t uid; gid_t gid;
    if (!OAGate_ConsoleUser(name, sizeof(name), &uid, &gid)) return;            // nobody to own the preferences: keep them dirty
    uid_t oldU = geteuid(); gid_t oldG = getegid();
    if (setegid(gid) != 0) return;
    if (seteuid(uid) != 0) { setegid(oldG); return; }
    CFStringRef user = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    CFPreferencesSetValue(d->key, d->settings, kAppID, user, kCFPreferencesAnyHost);
    CFPreferencesSynchronize(kAppID, user, kCFPreferencesAnyHost);
    CFRelease(user);
    seteuid(oldU); setegid(oldG);
    d->dirty = false;
}

static void timerFired(CFRunLoopTimerRef t, void *info) { (void)t; commit((Device *)info); }

static void schedule(Device *d)
{
    d->dirty = true;
    if (d->timer) { CFRunLoopTimerInvalidate(d->timer); CFRelease(d->timer); }
    CFRunLoopTimerContext ctx = { 0, d, NULL, NULL, NULL };
    d->timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + kCommitDelay, 0, 0, 0, timerFired, &ctx);
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), d->timer, kCFRunLoopDefaultMode);
}

// Headphone safety (clone addition, user 2026-10-20; the official helper has no such step): a unit with no saved settings starts at the
// factory phones level, 0 dB, which is far too loud for 32-ohm headphones. Every element of the device's MAHeadphoneLevels (every phones
// channel the model has: 4 on the FW1814, 2 on the Audiophile) above the ceiling is lowered to it in one write; lower levels are kept.
// Same ceiling as the panel's start-up limit (control-panel DeviceModel.phonesStartupCeiling: -86 half-dB steps = -43 dB).
static const int kPhonesCeiling = -86;

static void limitPhones(Device *d)
{
    CFTypeRef cur = IORegistryEntryCreateCFProperty(d->svc, CFSTR("MAHeadphoneLevels"), NULL, 0);
    if (!cur) return;
    if (CFGetTypeID(cur) != CFArrayGetTypeID()) { CFRelease(cur); return; }
    CFIndex n = CFArrayGetCount(cur);
    CFMutableArrayRef lim = CFArrayCreateMutable(NULL, n, &kCFTypeArrayCallBacks);
    bool changed = false;
    for (CFIndex i = 0; i < n; i++) {
        int v = 0;
        CFNumberRef e = CFArrayGetValueAtIndex(cur, i);
        if (CFGetTypeID(e) == CFNumberGetTypeID()) CFNumberGetValue(e, kCFNumberIntType, &v);
        if (v > kPhonesCeiling) { v = kPhonesCeiling; changed = true; }
        CFNumberRef x = CFNumberCreate(NULL, kCFNumberIntType, &v);
        CFArrayAppendValue(lim, x); CFRelease(x);
    }
    CFRelease(cur);
    if (changed) {
        CFMutableDictionaryRef w = CFDictionaryCreateMutable(NULL, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFDictionarySetValue(w, CFSTR("MAHeadphoneLevels"), lim);
        kern_return_t kr = IORegistryEntrySetCFProperties(d->svc, w);
        CFRelease(w);
        if (kr == KERN_SUCCESS) CFDictionarySetValue(d->settings, CFSTR("MAHeadphoneLevels"), lim);
        printf("no saved settings: phones limited to %d on %ld channel(s) -> 0x%x\n", kPhonesCeiling, (long)n, kr);
    }
    CFRelease(lim);
}

// ref -loadPrefsForUser: -- saved dictionary -> one SetProperties; none saved -> seed from MAMetaAllProperties (no write in the reference;
// the clone then limits the phones, limitPhones above)
static void loadPrefs(Device *d)
{
    char name[128];
    if (!OAGate_ConsoleUser(name, sizeof(name), NULL, NULL)) { fprintf(stderr, "No user logged in\n"); return; }
    CFStringRef user = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    CFPropertyListRef saved = CFPreferencesCopyValue(d->key, kAppID, user, kCFPreferencesAnyHost);
    CFRelease(user);
    if (saved && CFGetTypeID(saved) == CFDictionaryGetTypeID()) {
        CFDictionaryRemoveAllValues(d->settings);
        CFDictionaryRef sd = saved;
        CFIndex n = CFDictionaryGetCount(sd);
        const void **k = malloc(n * sizeof(void *)), **v = malloc(n * sizeof(void *));
        CFDictionaryGetKeysAndValues(sd, k, v);
        for (CFIndex i = 0; i < n; i++) CFDictionarySetValue(d->settings, k[i], v[i]);
        free(k); free(v);
        kern_return_t kr = IORegistryEntrySetCFProperties(d->svc, d->settings);
        printf("replayed saved settings for %s -> 0x%x\n", CFStringGetCStringPtr(d->key, kCFStringEncodingUTF8) ?: "?", kr);
    } else {
        CFTypeRef all = IORegistryEntryCreateCFProperty(d->svc, CFSTR("MAMetaAllProperties"), NULL, 0);
        if (all && CFGetTypeID(all) == CFDictionaryGetTypeID()) {
            CFDictionaryRemoveAllValues(d->settings);
            CFDictionaryRef ad = all;
            CFIndex n = CFDictionaryGetCount(ad);
            const void **k = malloc(n * sizeof(void *)), **v = malloc(n * sizeof(void *));
            CFDictionaryGetKeysAndValues(ad, k, v);
            for (CFIndex i = 0; i < n; i++) CFDictionarySetValue(d->settings, k[i], v[i]);
            free(k); free(v);
        }
        if (all) CFRelease(all);
        limitPhones(d);
    }
    if (saved) CFRelease(saved);
}

// ref -receivedPropertyChangeMessage: -- body at message offset +0x24 (24-byte user header + type, refCon, generation), IOKit XML
static void received(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
    (void)port;
    Device *d = (Device *)info;
    if (size <= 0x24 || ((const uint32_t *)msg)[6] != 0) return;                // type 0 = property change (offset 0x18 in the user message)
    size_t len = (size_t)size - 0x24;
    char *xml = malloc(len + 1);
    if (!xml) return;
    memcpy(xml, (const char *)msg + 0x24, len);
    xml[len] = 0;
    CFStringRef err = NULL;
    CFTypeRef dict = IOCFUnserialize(xml, NULL, 0, &err);
    free(xml);
    if (err) CFRelease(err);
    if (!dict) return;
    if (CFGetTypeID(dict) == CFDictionaryGetTypeID() && OAGate_Open()) {        // ref: merged only while a user is logged in
        CFDictionaryRef nd = dict;
        CFIndex n = CFDictionaryGetCount(nd);
        const void **k = malloc(n * sizeof(void *)), **v = malloc(n * sizeof(void *));
        CFDictionaryGetKeysAndValues(nd, k, v);
        for (CFIndex i = 0; i < n; i++) CFDictionarySetValue(d->settings, k[i], v[i]);
        free(k); free(v);
        schedule(d);
    }
    CFRelease(dict);
}

// Correction (2026-10-17, second integrated session): the helper kept its user client and the io_service_t of a device after the kext tore the
// device down, so the kernel object stayed retained and `kextunload` was refused (exit 3, "FW1814Device=1" alive). The reference drops the
// device instance when the service goes away; here: on kIOMessageServiceIsTerminated flush the pending commit, close the connection, release
// the service, the notifier and the notification port, and forget the device.
static void removeDevice(Device *d)
{
    for (Device **pp = &sDevices; *pp; pp = &(*pp)->next) if (*pp == d) { *pp = d->next; break; }
    if (d->timer) { CFRunLoopTimerInvalidate(d->timer); CFRelease(d->timer); d->timer = NULL; }
    commit(d);                                                                      // keeps the last change (gate permitting)
    if (d->notifier) { IOObjectRelease(d->notifier); d->notifier = 0; }
    if (d->conn) { IOServiceClose(d->conn); d->conn = 0; }
    if (d->portSrc) { CFRunLoopRemoveSource(CFRunLoopGetCurrent(), d->portSrc, kCFRunLoopDefaultMode); CFRelease(d->portSrc); d->portSrc = NULL; }
    if (d->port) { CFMachPortInvalidate(d->port); CFRelease(d->port); d->port = NULL; }
    if (d->svc) { IOObjectRelease(d->svc); d->svc = 0; }
    if (d->settings) CFRelease(d->settings);
    if (d->key) CFRelease(d->key);
    free(d);
}

static void deviceInterest(void *refcon, io_service_t service, natural_t messageType, void *messageArgument)
{
    (void)service; (void)messageArgument;
    if (messageType == kIOMessageServiceIsTerminated) removeDevice((Device *)refcon);
}

void OASettings_Init(void)
{
    sDevices = NULL;
    sNotifyPort = IONotificationPortCreate(kIOMasterPortDefault);
    if (sNotifyPort) CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(sNotifyPort), kCFRunLoopDefaultMode);
}

void OASettings_DeviceAdded(io_service_t svc)
{
    for (Device *e = sDevices; e; e = e->next) if (IOObjectIsEqualTo(e->svc, svc)) return;
    Device *d = calloc(1, sizeof(Device));
    if (!d) return;
    d->key = guidKey(svc);
    d->settings = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!d->key || !d->settings || IOServiceOpen(svc, mach_task_self(), 0, &d->conn) != KERN_SUCCESS) {
        if (d->key) CFRelease(d->key);
        if (d->settings) CFRelease(d->settings);
        free(d);
        return;
    }
    IOObjectRetain(svc);
    d->svc = svc;
    CFMachPortContext ctx = { 0, d, NULL, NULL, NULL };
    d->port = CFMachPortCreate(NULL, received, &ctx, NULL);
    d->portSrc = CFMachPortCreateRunLoopSource(NULL, d->port, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), d->portSrc, kCFRunLoopDefaultMode);
    IOConnectSetNotificationPort(d->conn, 0, CFMachPortGetPort(d->port), 0);
    if (sNotifyPort) IOServiceAddInterestNotification(sNotifyPort, svc, kIOGeneralInterest, deviceInterest, d, &d->notifier);
    d->next = sDevices; sDevices = d;
    if (OAGate_Open()) loadPrefs(d);
    else if (!OAGate_SystemSleeping()) limitPhones(d);                        // login window: nothing to replay yet, phones still lowered
}

void OASettings_ReplayAll(void)
{
    if (!OAGate_Open()) return;
    for (Device *d = sDevices; d; d = d->next) loadPrefs(d);
}

void OASettings_FlushAll(void)
{
    for (Device *d = sDevices; d; d = d->next) {
        if (d->timer) { CFRunLoopTimerInvalidate(d->timer); CFRelease(d->timer); d->timer = NULL; }
        commit(d);
    }
}
