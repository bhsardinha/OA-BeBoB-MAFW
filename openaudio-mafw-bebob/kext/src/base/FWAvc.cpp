#include "FWAvc.h"
#include "FWAvcFrames.h"
#include "Trace.h"

static const IOReturn kErrBadArg   = (IOReturn)0xE00002C2;
static const IOReturn kErrNoMem    = (IOReturn)0xE00002BD;
static const IOReturn kErrNotImpl  = (IOReturn)0xE00002C7;    // -0x1ffffd39: response 0x08 NOT IMPLEMENTED
static const IOReturn kErrRejected = (IOReturn)0xE00002C2;    // -0x1ffffd3e: response 0x0A REJECTED
static const IOReturn kBusReset    = (IOReturn)0xE0008014;    // -0x1fff7fec: retried by the reference

OSDefineMetaClassAndStructors(com_openaudio_FWAsyncAVCCommand, IOFireWireAVCAsynchronousCommand)

#define super IOFireWireAVCUnit
OSDefineMetaClassAndStructors(com_openaudio_FWAvcUnit, IOFireWireAVCUnit)

// ref AllocateConnectionManager @0x23c72 (asm): nub = device[+0xe0]; props = nub->vt[0x2f0] (dictionaryWithProperties); unit = new (0xF8 B);
// unit->vt[0x248](props) (init); unit->vt[0x670](device) (attach); unit->vt[0x28]() (release: the registry link keeps it); unit->vt[0x5c0](nub) (start).
// The clone keeps its own reference (the device releases it in ReleaseConnectionManager), which is equivalent and avoids a dangling pointer.
bool com_openaudio_FWAvcUnit::Setup(IOService *device, IOFireWireNub *nub)
{
    if (!device || !nub) return false;
    OSDictionary *props = nub->dictionaryWithProperties();
    const bool ok = init(props);
    if (props) props->release();
    if (!ok) return false;
    fAsyncLock = IOLockAlloc();
    fAccepting = true;
    if (!fAsyncLock) return false;
    if (!attach(device)) return false;
    return start(nub);
}

void com_openaudio_FWAvcUnit::free()
{
    if (fAsyncLock) { IOLockFree(fAsyncLock); fAsyncLock = 0; }
    super::free();
}

// ref start @0x1692: if the flag byte [+0xe0] is clear, call the base class's start (IOFireWireAVCUnit's vtable slot, non-virtual) and store the
// result; return the flag. stop @0x165e: if the flag is set, call the base class's stop and clear it.
bool com_openaudio_FWAvcUnit::start(IOService *provider)
{
    if (!fStarted) fStarted = super::start(provider);
    return fStarted;
}

void com_openaudio_FWAvcUnit::stop(IOService *provider)
{
    DrainAsyncQueue();                                        // clone (AP-3.9): before the base class detaches from the nub
    if (fStarted) { super::stop(provider); fStarted = false; }
}

// Refuse new commands, drop every queued command behind the head (never submitted: release it and its unit reference), cancel the head.
void com_openaudio_FWAvcUnit::DrainAsyncQueue()
{
    if (!fAsyncLock) return;
    IOLockLock(fAsyncLock);
    fAccepting = false;
    com_openaudio_FWAsyncAVCCommand *head = fAsyncHead, *drop = head ? head->fNext : 0;
    if (head) { head->fNext = 0; fAsyncTail = head; }
    IOLockUnlock(fAsyncLock);
    UInt32 dropped = 0;
    while (drop) { com_openaudio_FWAsyncAVCCommand *n = drop->fNext; drop->release(); release(); drop = n; dropped++; }
    if (head) head->cancel();                                 // completion (if the family delivers one) runs FinishAsyncCommand
    OA_LOG("AVC async queue drained: %u dropped, head %s", (unsigned)dropped, head ? "cancelled" : "none");
}

// ref updateSubUnits @0x13cc: returns at once -- the unit never creates IOFireWireAVCSubUnit nubs for the device's audio / music sub-units.
void com_openaudio_FWAvcUnit::updateSubUnits(bool) {}

// ref AVCCommand @0x17fc: new FWAsyncAVCCommand; init(command, len, FWAVCAsyncCommandCallback, functor); [+0xa0] = unit; if the list is empty submit it and make it
// the head, otherwise append behind the tail. A failed init/submit releases the command and returns the status.
static void FWAVCAsyncCommandCallback(void *, IOFireWireAVCAsynchronousCommand *cmd)
{
    com_openaudio_FWAsyncAVCCommand *c = OSDynamicCast(com_openaudio_FWAsyncAVCCommand, cmd);
    if (c && !c->isPending() && c->fUnit) c->fUnit->FinishAsyncCommand(c);      // ref asm 0x1726..0x178b (the functor branch is never used: no caller passes one)
}

IOReturn com_openaudio_FWAvcUnit::AVCCommandAsync(const UInt8 *cmd, UInt32 len)
{
    if (!fAsyncLock || !fStarted || !fAccepting) return kIOReturnNotReady;
    com_openaudio_FWAsyncAVCCommand *c = OSTypeAlloc(com_openaudio_FWAsyncAVCCommand);
    if (!c) return kErrNoMem;
    IOReturn r = c->init(cmd, len, &FWAVCAsyncCommandCallback, 0);
    if (r != kIOReturnSuccess) { c->release(); return r; }
    c->fUnit = this; c->fNext = 0;
    retain();                                                 // clone (AP-3.9): released by FinishAsyncCommand / StartQueueHead / DrainAsyncQueue
    IOLockLock(fAsyncLock);
    const bool first = (fAsyncHead == 0);
    if (first) fAsyncHead = fAsyncTail = c;
    else       { fAsyncTail->fNext = c; fAsyncTail = c; }
    IOLockUnlock(fAsyncLock);
    if (first) StartQueueHead();                            // submit outside the lock: the completion callback takes it on the FireWire work loop
    return kIOReturnSuccess;
}

// Submit the head; a command the family refuses is dropped and the next one is tried (the reference has no such path: its submit failure only
// happens for the very first command and releases it).
void com_openaudio_FWAvcUnit::StartQueueHead()
{
    for (;;) {
        IOLockLock(fAsyncLock);
        com_openaudio_FWAsyncAVCCommand *h = fAsyncHead;
        IOLockUnlock(fAsyncLock);
        if (!h) return;
        const IOReturn r = h->submit(this);
        if (r == kIOReturnSuccess) return;
        OA_LOG("AVC async submit failed 0x%x", (unsigned)r);
        IOLockLock(fAsyncLock);
        fAsyncHead = h->fNext;
        if (fAsyncTail == h) fAsyncTail = 0;
        IOLockUnlock(fAsyncLock);
        h->release();
        release();                                            // the unit reference that command held
    }
}

// ref FinishAsyncCommand @0x16c8: only the head can finish; head = head->next, tail cleared when it was the tail, submit the new head, release the command.
void com_openaudio_FWAvcUnit::FinishAsyncCommand(com_openaudio_FWAsyncAVCCommand *c)
{
    bool wasHead = false, more = false;
    IOLockLock(fAsyncLock);
    if (fAsyncHead == c && c) {
        wasHead = true;
        fAsyncHead = c->fNext;
        if (fAsyncTail == c) fAsyncTail = 0;
        more = (fAsyncHead != 0);
    }
    IOLockUnlock(fAsyncLock);
    if (!wasHead) return;
    if (more) StartQueueHead();
    c->release();
    release();                                                // the unit reference this command held; may be the last one
}

IOReturn com_openaudio_FWAvcUnit::AVCCommandExtendedTimeout(const UInt8 *cmd, UInt32 len, UInt8 *resp, UInt32 *respLen)
{
    if (len - 1 >= 0x200) {                                   // ref @0x15a3: unsigned compare, 1..0x200
        IOLog(OA_TAG "Loopy AVCCmd, len %u, respLen %u\n", (unsigned)len, respLen ? (unsigned)*respLen : 0u);
        return kErrBadArg;
    }
    if (!fDevice || !avcLock || !cmdLock) return kIOReturnNotReady;
    IOFireWireAVCCommand *c = IOFireWireAVCCommand::withNub(fDevice, cmd, len, resp, respLen);     // ref: nub = [+0x88]
    if (!c) return kErrNoMem;
    IOLockLock(avcLock);                                      // ref [+0x98], held for the whole command
    fCommand = c;                                             // ref [+0xa0]: the command the unit's AVCResponse delivers the response to
    c->setTimeout(1000000);                                   // ref: *(cmd+0x48) = 1,000,000 us
    AbsoluteTime t0, t1; UInt64 ns = 0;
    clock_get_uptime(&t0);
    IOReturn r = c->submit(false);                            // ref vt[0x140](0)
    clock_get_uptime(&t1);
    absolutetime_to_nanoseconds((UInt64)t1 - (UInt64)t0, &ns);
    IOLockLock(cmdLock);                                      // ref [+0xd0]
    fCommand = 0;
    IOLockUnlock(cmdLock);
    c->release();
    IOLockUnlock(avcLock);
    // bench diagnostics (2026-10-17, SW-return routing bug): every synchronous AV/C command with its result and elapsed time
    OA_LOG("AVC %02x %02x %02x %02x %02x %02x len %u -> r=0x%x resp0 %02x respLen %u  %u ms", len > 0 ? cmd[0] : 0, len > 1 ? cmd[1] : 0, len > 2 ? cmd[2] : 0,
           len > 3 ? cmd[3] : 0, len > 4 ? cmd[4] : 0, len > 5 ? cmd[5] : 0, (unsigned)len, (unsigned)r, (resp && respLen && *respLen) ? resp[0] : 0,
           respLen ? (unsigned)*respLen : 0u, (unsigned)(ns / 1000000));
    return r;
}

IOReturn com_openaudio_FWAvcUnit::AVCCommandWithErrorResponse(const UInt8 *cmd, UInt32 len, UInt8 *resp, UInt32 *respLen)
{
    IOReturn r = AVCCommandExtendedTimeout(cmd, len, resp, respLen);
    UInt32 tries = 0;
    while (r == kBusReset) {
        IOSleep(1000);
        if (++tries > 0x19) break;
        r = AVCCommandExtendedTimeout(cmd, len, resp, respLen);
    }
    return CheckAVCError(r, resp);
}

IOReturn com_openaudio_FWAvcUnit::CheckAVCError(IOReturn r, const UInt8 *resp)
{
    if (r != kIOReturnSuccess) return r;
    const UInt8 code = resp[0];
    UInt32 pause;
    if ((UInt8)(code - 8) < 8) {                              // 0x08..0x0F: responses
        if (code != 0x0F) {
            if (code == 0x0A) return kErrRejected;
            if (code != 0x08) return kIOReturnSuccess;        // ACCEPTED/IMPLEMENTED/...: no pause
            return kErrNotImpl;
        }
        pause = 500;                                          // INTERIM
    } else {
        pause = 1000;                                         // not a recognised response code
    }
    IOSleep(pause);
    return r;
}

IOReturn FWAvcControlPlugSignalFormat(com_openaudio_FWAvcUnit *unit, bool input, UInt8 plug, UInt8 fdf)
{
    if (!unit) return kIOReturnNotReady;
    UInt8 cmd[8] = { 0x00, 0xFF, (UInt8)(0x18 | (input ? 1 : 0)), plug, 0x90, fdf, 0xFF, 0xFF };
    UInt8 resp[8];
    UInt32 respLen = 8;
    bzero(resp, sizeof(resp));
    return unit->AVCCommandWithErrorResponse(cmd, 8, resp, &respLen);
}

IOReturn FWAvcSelectorCommand(com_openaudio_FWAvcUnit *unit, UInt8 selector, UInt8 value)
{
    if (!unit) return kIOReturnNotReady;
    UInt8 cmd[12]; FWAvcSelectorFrame(cmd, selector, value);                // bytes: FWAvcFrames.h
    return unit->AVCCommandAsync(cmd, sizeof(cmd));                         // ref vt[0x8b0]
}

IOReturn FWAvcFeatureFunctionBlock(com_openaudio_FWAvcUnit *unit, UInt8 fb, UInt8 controlSelector, UInt8 channel, UInt16 value)
{
    if (!unit) return kIOReturnNotReady;
    UInt8 cmd[12]; FWAvcFeatureFrame(cmd, fb, controlSelector, channel, value);
    return unit->AVCCommandAsync(cmd, sizeof(cmd));                         // ref vt[0x8b0]
}

IOReturn FWAvcMixerConnectSingle(com_openaudio_FWAvcUnit *unit, bool connect, UInt8 fb, UInt8 b, UInt8 src, UInt8 dst)
{
    if (!unit) return kIOReturnNotReady;
    UInt8 cmd[16]; FWAvcMixerConnectFrame(cmd, connect, fb, b, src, dst);
    return unit->AVCCommandAsync(cmd, sizeof(cmd));                         // ref vt[0x8b0]; the pair builder no longer stops on a slow device
}

IOReturn FWAvcMixerConnectPair(com_openaudio_FWAvcUnit *unit, bool connect, UInt8 fb, UInt8 b, UInt8 src, UInt8 dst)
{
    IOReturn r = FWAvcMixerConnectSingle(unit, connect, fb, b, (UInt8)(src * 2 + 0xff), (UInt8)(dst * 2 + 0xff));
    if (r != kIOReturnSuccess) return r;
    return FWAvcMixerConnectSingle(unit, connect, fb, b, (UInt8)(src * 2), (UInt8)(dst * 2));
}
