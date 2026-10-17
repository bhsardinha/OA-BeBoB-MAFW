// Bus-reset plumbing (A1 Q4, Q-ME §4): run a member on a new kernel thread, the reference's UKernelCallHelpers::InvokeOnNewThread.
// The reference spawns a thread per call; the clone uses IOCreateThread (deprecated but present in the 10.13 kernel KPI) the same way
// FWAvcConnectionManager::BusReset does (FWCMP.cpp). Untested on 10.13.6 until the integrated run (STATE.md open item).
#ifndef OA_FWBUSRESET_H
#define OA_FWBUSRESET_H

#include <IOKit/IOLib.h>

// Runs fn(arg) on a new thread; returns false if the thread could not be created (the caller keeps ownership of `arg` then).
bool FWRunOnNewThread(void (*fn)(void *), void *arg);

#endif
