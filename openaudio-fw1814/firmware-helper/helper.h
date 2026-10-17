// OpenAudio FW1814 helper -- shared declarations (gating H.3, settings H.4).
#ifndef OA_HELPER_H
#define OA_HELPER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdbool.h>
#include <sys/types.h>

// ---- Gating.c (H.3): every write is gated on system sleep and on a logged-in console user (H2 deviceMatched guard) ----------------
typedef struct {
    void (*woke)(void *ctx);                 // system has powered on (ref MAPowerNotification, A3-M9 §2: only "has powered on" and "will sleep" are acted on)
    void (*loggedIn)(void *ctx);             // a console user appeared
    void (*loggedOut)(void *ctx);
    void *ctx;
} OAGateCallbacks;
void OAGate_Start(const OAGateCallbacks *cb);
bool OAGate_SystemSleeping(void);            // ref sSystemSleeping_b
bool OAGate_ConsoleUser(char *name, size_t nameLen, uid_t *uid, gid_t *gid);   // ref [MAConsoleUser isUserLoggedIn]; "loginwindow" counts as nobody
bool OAGate_Open(void);                      // !sleeping && a user is logged in

// ---- Settings.c (H.4): per-GUID settings, persisted 4 s after the last change, replayed with one setProperties -------------------
void OASettings_Init(void);
void OASettings_DeviceAdded(io_service_t device);      // open a user client, subscribe to the dirty-property port, replay saved settings if gated open
void OASettings_ReplayAll(void);                       // login / wake: replay for every known device
void OASettings_FlushAll(void);                        // commit pending changes now (sleep, logout, exit)

#endif
