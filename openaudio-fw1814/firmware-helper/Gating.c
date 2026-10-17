// H.3 -- gate every write on system sleep and on a logged-in console user.
// Reference: sSystemSleeping_b; -[MAPowerNotification observeSleepNotifications:]; -[MAConsoleUser startObservingLogins:] / isUserLoggedIn
// (A4 H2 deviceMatched guards, A3-M9 §2 for the IOKit power messages handled). Console user via SCDynamicStore, power via IORegisterForSystemPower.
#include "helper.h"
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <string.h>

static OAGateCallbacks sCb;
static volatile bool sSleeping = false;
static bool sUserWasIn = false;
static io_connect_t sRootPort;
static SCDynamicStoreRef sStore;

bool OAGate_SystemSleeping(void) { return sSleeping; }

bool OAGate_ConsoleUser(char *name, size_t nameLen, uid_t *uid, gid_t *gid)
{
    uid_t u = 0; gid_t g = 0;
    CFStringRef n = SCDynamicStoreCopyConsoleUser(sStore, &u, &g);
    if (!n) return false;
    char buf[256] = "";
    CFStringGetCString(n, buf, sizeof(buf), kCFStringEncodingUTF8);
    CFRelease(n);
    if (buf[0] == 0 || strcmp(buf, "loginwindow") == 0) return false;
    if (name && nameLen) { strncpy(name, buf, nameLen - 1); name[nameLen - 1] = 0; }
    if (uid) *uid = u;
    if (gid) *gid = g;
    return true;
}

bool OAGate_Open(void) { return !sSleeping && OAGate_ConsoleUser(NULL, 0, NULL, NULL); }

static void powerCallback(void *refcon, io_service_t service, natural_t type, void *arg)
{
    (void)refcon; (void)service;
    switch (type) {
        case kIOMessageCanSystemSleep:       // never veto
            IOAllowPowerChange(sRootPort, (long)arg);
            break;
        case kIOMessageSystemWillSleep:
            sSleeping = true;
            if (sCb.ctx) { /* pending settings are committed by the owner on its own timer; the owner flushes on sleep via loggedOut-like hook */ }
            IOAllowPowerChange(sRootPort, (long)arg);
            break;
        case kIOMessageSystemHasPoweredOn:
            sSleeping = false;
            if (sCb.woke) sCb.woke(sCb.ctx);
            break;
        default: break;
    }
}

static void userChanged(SCDynamicStoreRef store, CFArrayRef keys, void *info)
{
    (void)store; (void)keys; (void)info;
    const bool now = OAGate_ConsoleUser(NULL, 0, NULL, NULL);
    if (now && !sUserWasIn && sCb.loggedIn) sCb.loggedIn(sCb.ctx);
    if (!now && sUserWasIn && sCb.loggedOut) sCb.loggedOut(sCb.ctx);
    sUserWasIn = now;
}

void OAGate_Start(const OAGateCallbacks *cb)
{
    sCb = *cb;
    SCDynamicStoreContext ctx = { 0, NULL, NULL, NULL, NULL };
    sStore = SCDynamicStoreCreate(NULL, CFSTR("com.openaudio.fw1814.helper"), userChanged, &ctx);
    if (sStore) {
        CFStringRef key = SCDynamicStoreKeyCreateConsoleUser(NULL);
        CFArrayRef keys = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
        SCDynamicStoreSetNotificationKeys(sStore, keys, NULL);
        CFRelease(keys); CFRelease(key);
        CFRunLoopSourceRef src = SCDynamicStoreCreateRunLoopSource(NULL, sStore, 0);
        if (src) { CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopDefaultMode); CFRelease(src); }
    }
    sUserWasIn = OAGate_ConsoleUser(NULL, 0, NULL, NULL);
    IONotificationPortRef np;
    io_object_t notifier;
    sRootPort = IORegisterForSystemPower(NULL, &np, powerCallback, &notifier);
    if (sRootPort) CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(np), kCFRunLoopDefaultMode);
}
