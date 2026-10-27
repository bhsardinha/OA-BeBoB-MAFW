#include "FWCMP.h"
#include <IOKit/IOLib.h>
#include <IOKit/avc/IOFireWirePCRSpace.h>
#include "Trace.h"
#include "FWAudioDevice.h"

// R140: the CMP plug messages also go to the device's MADebugTrace ring (the kernel log drops this kext's IOLog lines under the IOAudio flood)
#define CMP_TRACE(dev, fmt, ...) do { char _b[160]; snprintf(_b, sizeof(_b), fmt, ##__VA_ARGS__); OA_LOG("%s", _b); \
    if (com_openaudio_FWAudioDevice *_d = OSDynamicCast(com_openaudio_FWAudioDevice, (dev))) _d->DebugTrace(_b); } while (0)

static const IOReturn kErrBadArg   = (IOReturn)0xE00002BC;
static const IOReturn kErrNoMem    = (IOReturn)0xE00002BD;      // ref -0x1ffffd43
static const IOReturn kErrNotFound = (IOReturn)0xE00002F0;      // ref breakConnection: no such connection
static const IOReturn kErrNoArray  = (IOReturn)0xE00002BE;      // ref breakConnection: no connection array
// Correction (2026-10-23, R134): names fixed from IOFireWireFamilyCommon.h, values unchanged. 0xE0008014 = kIOFireWireResponseBase + 4 =
// resp_conflict_error from the device (was named kBusReset); 0xE0008020 = kIOFireWireResponseBase + kFWResponseBusResetError = kIOFireWireBusReset
// (was named kGenMismatch).
static const IOReturn kRespConflict = (IOReturn)0xE0008014;     // retryable (-0x1fff7fec)
static const IOReturn kIOFWBusReset = (IOReturn)0xE0008020;     // ref ReconnectP2P stops on this (-0x1fff7fe0)
static const UInt32 kOpcrBase = 0xF0000904;                     // ref: plug * 4 - 0xffff6fc
static const UInt32 kIpcrBase = 0xF0000984;                     // ref: plug * 4 - 0xffff67c
static const UInt32 kSpeedUnchanged = 0x80000000;
static const UInt32 kHostOpcrPayload122 = 0xC1003C7A;           // ref Connect_iPCR @0x247ba: online|bcast|p2p=1|ovhd 0xF|payload 0x7A

// ref HandlePCRLock @0x23f26: an empty callback; the reference never reacts to remote writes of the host PCR.
static void HandlePCRLockStub(void *, UInt16, UInt32, UInt32, UInt32) {}

static inline UInt32 swap32(UInt32 v) { return __builtin_bswap32(v); }

#define super OSObject
OSDefineMetaClassAndStructors(com_openaudio_FWP2PConnection, OSObject)
OSDefineMetaClassAndStructors(com_openaudio_FWAvcConnectionManager, OSObject)

bool com_openaudio_FWAvcConnectionManager::InitWithDevice(IOService *device, IOFireWireNub *nub)
{
    if (!device || !nub || !init()) return false;
    fDevice = device;
    fNub = nub;
    fConnections = 0;
    bzero(fHostPlug, sizeof(fHostPlug));
    bzero(fHostPlugAllocated, sizeof(fHostPlugAllocated));
    fClosed = false;
    fReconnected = 0;
    fReconnectedRefcon = 0;
    return true;
}

// clone (capture 97): give back every host plug still allocated, so no HandlePCRLock registration outlives this object (and the kext).
// Called from free(), which the device reaches from stop() while the nub (and its bus) is still there.
void com_openaudio_FWAvcConnectionManager::FreeHostPlugs()
{
    IOFireWirePCRSpace *pcr = fNub ? IOFireWirePCRSpace::getPCRAddressSpace(fNub->getBus()) : 0;
    for (unsigned i = 0; i < 3; i++) {
        if (!fHostPlugAllocated[i]) continue;
        if (pcr) pcr->freeOutputPlug(fHostPlug[i]);
        CMP_TRACE(fDevice, "CMP: host plug %u (slot %u) freed at teardown%s", (unsigned)fHostPlug[i], i, pcr ? "" : " -- NO PCR SPACE, left registered");
        fHostPlugAllocated[i] = false;
    }
}

// R140: the panic of 2026-10-26 23:43 (capture 106) was IOFireWirePCRSpace calling HandlePCRLockStub in the image unloaded at 23:16, the unload
// having run while the engine streamed. Close() frees the plugs from the device's stop() and refuses any Connect_iPCR after it (a stream
// restart during the teardown would otherwise allocate a plug no one frees).
void com_openaudio_FWAvcConnectionManager::Close()
{
    fClosed = true;
    FreeHostPlugs();
}

// ref free @0x23e96: break every recorded connection (count cleared), then drop the array. Clone: then free the host plugs (FreeHostPlugs).
void com_openaudio_FWAvcConnectionManager::free()
{
    if (fConnections) {
        for (unsigned i = 0, n = fConnections->getCount(); i < n; i++) {
            com_openaudio_FWP2PConnection *c = OSDynamicCast(com_openaudio_FWP2PConnection, fConnections->getObject(i));
            if (c) updateP2PCount(c->pcrAddress, -1, false, 0xffffffff, kSpeedUnchanged);
        }
        fConnections->release();
        fConnections = 0;
    }
    FreeHostPlugs();
    super::free();
}

IOReturn com_openaudio_FWAvcConnectionManager::Connect(bool remoteIsInput, UInt32 channel, UInt32 plug)
{
    if (remoteIsInput) return Connect_iPCR(plug, channel);
    return makeP2POutputConnection(plug, channel);                 // ref: vt[0x158](this, plug, channel)
}

IOReturn com_openaudio_FWAvcConnectionManager::Disconnect(bool remoteIsInput, UInt32 channel, UInt32 plug)
{
    (void)channel;
    if (remoteIsInput) return Disconnect_iPCR(plug);
    return breakP2POutputConnection(plug);                         // ref: vt[0x160]
}

// ref Connect_iPCR @0x247ba. The host-local PCR it programs is an OUTPUT-plug layout (overhead-ID/payload fields exist only in
// an oPCR), so the clone uses the PCR space's output-plug calls for the reference's vt[0x1b8/0x1c0/0x1c8/0x1d0]
// (allocate / free / read / update). **Inference from the value layout, not from the slot index — check at the integrated run.**
// Out-of-range plug indices fall back to element 0, as in the reference.
IOReturn com_openaudio_FWAvcConnectionManager::Connect_iPCR(UInt32 plug, UInt32 channel)
{
    if (fClosed) { CMP_TRACE(fDevice, "CMP: Connect_iPCR(%u) refused: connection manager closed (device stopping)", (unsigned)plug); return kIOReturnNotReady; }   // R140
    IOFireWirePCRSpace *pcr = IOFireWirePCRSpace::getPCRAddressSpace(fNub->getBus());
    if (!pcr) return kErrBadArg;
    const unsigned idx = plug < 3 ? plug : 0;
    UInt32 *slot = &fHostPlug[idx];
    // clone (capture 97): a second Connect on the same slot would leak the first plug (the reference allocates again): free it first
    if (fHostPlugAllocated[idx]) { pcr->freeOutputPlug(*slot); fHostPlugAllocated[idx] = false; }
    IOReturn r = pcr->allocateOutputPlug(this, HandlePCRLockStub, *slot);
    if (r != kIOReturnSuccess) return r;
    fHostPlugAllocated[idx] = true;
    CMP_TRACE(fDevice, "CMP: host plug %u allocated (slot %u, channel %u)", (unsigned)*slot, idx, (unsigned)channel);
    UInt32 old = pcr->readOutputPlug(*slot);
    r = pcr->updateOutputPlug(*slot, old, (channel << 16) | kHostOpcrPayload122);
    if (r != kIOReturnSuccess) return r;
    return makeP2PInputConnection(plug, channel);                  // ref vt[0x148]
}

IOReturn com_openaudio_FWAvcConnectionManager::Disconnect_iPCR(UInt32 plug)
{
    breakP2PInputConnection(plug);                                 // ref vt[0x150]
    IOFireWirePCRSpace *pcr = IOFireWirePCRSpace::getPCRAddressSpace(fNub->getBus());
    if (!pcr) return kErrBadArg;
    const unsigned idx = plug < 3 ? plug : 0;
    UInt32 *slot = &fHostPlug[idx];
    if (!fHostPlugAllocated[idx]) return kIOReturnSuccess;        // clone: nothing allocated on this slot (never connected, or already freed)
    UInt32 old = pcr->readOutputPlug(*slot);
    IOReturn r = pcr->updateOutputPlug(*slot, old, 0x7a);          // channel and flags cleared, payload kept
    pcr->freeOutputPlug(*slot);
    fHostPlugAllocated[idx] = false;
    CMP_TRACE(fDevice, "CMP: host plug %u freed (Disconnect_iPCR, slot %u)", (unsigned)*slot, idx);
    return r;
}

// ---- work-loop plumbing (ref vt[0x688] on the device = getWorkLoop, vt[0x1a0] = IOWorkLoop::runAction) -------------------

IOReturn com_openaudio_FWAvcConnectionManager::makeP2POutputConnection(UInt32 plug, UInt32 channel)
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl) return kErrBadArg;
    return wl->runAction(&CoreMakeP2POutputConnection, this, (void *)(uintptr_t)plug, (void *)(uintptr_t)channel);
}
IOReturn com_openaudio_FWAvcConnectionManager::makeP2PInputConnection(UInt32 plug, UInt32 channel)
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl) return kErrBadArg;
    return wl->runAction(&CoreMakeP2PInputConnection, this, (void *)(uintptr_t)plug, (void *)(uintptr_t)channel);
}
IOReturn com_openaudio_FWAvcConnectionManager::breakP2POutputConnection(UInt32 plug)
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl) return kErrBadArg;
    return wl->runAction(&CoreBreakP2POutputConnection, this, (void *)(uintptr_t)plug);
}
IOReturn com_openaudio_FWAvcConnectionManager::breakP2PInputConnection(UInt32 plug)
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl) return kErrBadArg;
    return wl->runAction(&CoreBreakP2PInputConnection, this, (void *)(uintptr_t)plug);
}

// ref CoreMakeP2POutputConnection @0x23f86: speed = the device nub's FWSpeed (vt 0x850); Input passes 0x80000000 (unchanged).
IOReturn com_openaudio_FWAvcConnectionManager::CoreMakeP2POutputConnection(OSObject *owner, void *plug, void *channel, void *, void *)
{
    com_openaudio_FWAvcConnectionManager *m = static_cast<com_openaudio_FWAvcConnectionManager *>(owner);
    return m->makeConnection(kOpcrBase + 4 * (UInt32)(uintptr_t)plug, (UInt32)(uintptr_t)channel, (UInt32)m->fNub->FWSpeed());
}
IOReturn com_openaudio_FWAvcConnectionManager::CoreMakeP2PInputConnection(OSObject *owner, void *plug, void *channel, void *, void *)
{
    com_openaudio_FWAvcConnectionManager *m = static_cast<com_openaudio_FWAvcConnectionManager *>(owner);
    return m->makeConnection(kIpcrBase + 4 * (UInt32)(uintptr_t)plug, (UInt32)(uintptr_t)channel, kSpeedUnchanged);
}
IOReturn com_openaudio_FWAvcConnectionManager::CoreBreakP2POutputConnection(OSObject *owner, void *plug, void *, void *, void *)
{
    return static_cast<com_openaudio_FWAvcConnectionManager *>(owner)->breakConnection(kOpcrBase + 4 * (UInt32)(uintptr_t)plug);
}
IOReturn com_openaudio_FWAvcConnectionManager::CoreBreakP2PInputConnection(OSObject *owner, void *plug, void *, void *, void *)
{
    return static_cast<com_openaudio_FWAvcConnectionManager *>(owner)->breakConnection(kIpcrBase + 4 * (UInt32)(uintptr_t)plug);
}

// ref makeConnection @0x24980: allocate the record array (capacity 4) on first use, update the PCR (+1), remember it.
IOReturn com_openaudio_FWAvcConnectionManager::makeConnection(UInt32 pcrAddrLo, UInt32 channel, UInt32 speed)
{
    if (!fConnections) {
        fConnections = OSArray::withCapacity(4);
        if (!fConnections) return kErrNoMem;
    }
    com_openaudio_FWP2PConnection *rec = OSTypeAlloc(com_openaudio_FWP2PConnection);
    if (!rec || !rec->Init()) { if (rec) rec->release(); return kErrNoMem; }
    IOReturn r = updateP2PCount(pcrAddrLo, 1, false, channel, speed);
    if (r == kIOReturnSuccess) {
        rec->pcrAddress = pcrAddrLo;
        rec->channel = channel;
        fConnections->setObject(rec);
    }
    rec->release();
    return r;
}

// ref breakConnection @0x24454: find the record, clear the PCR's count, drop the record.
IOReturn com_openaudio_FWAvcConnectionManager::breakConnection(UInt32 pcrAddrLo)
{
    if (!fConnections) return kErrNoArray;
    for (unsigned i = 0; i < fConnections->getCount(); i++) {
        com_openaudio_FWP2PConnection *c = OSDynamicCast(com_openaudio_FWP2PConnection, fConnections->getObject(i));
        if (c && c->pcrAddress == pcrAddrLo) {
            IOReturn r = updateP2PCount(pcrAddrLo, 0, false, 0xffffffff, kSpeedUnchanged);
            fConnections->removeObject(i);
            return r;
        }
    }
    return kErrNotFound;
}

// ref updateP2PCount @0x244f0 (decomp FWAVCConnectionManager__updateP2PCount.c). The new value is recomputed from the FIRST
// read on every attempt; the compare value is the previous attempt's returned old value (host order), as in the reference.
// Mirror of its end: when all four attempts end with success-but-not-locked it still returns the last status (0).
IOReturn com_openaudio_FWAvcConnectionManager::updateP2PCount(UInt32 pcrAddrLo, SInt32 delta, bool failOnReset,
                                                              UInt32 channel, UInt32 speed)
{
    FWAddress addr(0xffff, pcrAddrLo);                       // nodeID filled by the nub command
    UInt32 cur = 0;
    IOFWReadQuadCommand *rd = fNub->createReadQuadCommand(addr, &cur, 1, 0, 0, failOnReset);
    if (!rd) return kErrNoMem;
    IOReturn r = rd->submit();
    rd->release();
    if (r != kIOReturnSuccess) return r;

    const UInt32 first = swap32(cur);                         // device quadlet is big-endian on the wire
    cur = first;                                              // compare value, updated by locked()
    const UInt32 base = first | 0x80000000;
    for (int attempt = 0; attempt < 4; attempt++) {
        UInt32 v = base;
        if (channel != 0xffffffff) v = (first & 0xffc0ffff) | 0x80000000 | ((channel & 0x3f) << 16);
        if (speed != kSpeedUnchanged) v = (v & 0xffff3fff) | ((speed & 3) << 14);
        UInt32 nv = (delta < 1) ? (v & 0xc0ffffff) : ((v & 0xc0ffffff) + 0x1000000);
        UInt32 newVal = swap32(nv);
        FWAddress a2(0xffff, pcrAddrLo);
        IOFWCompareAndSwapCommand *cas = fNub->createCompareAndSwapCommand(a2, &cur, &newVal, 1, 0, 0, false);
        if (!cas) return kErrNoMem;
        r = cas->submit();
        bool locked = cas->locked(&cur);
        cas->release();
        if (r != kRespConflict) {
            if (r != kIOReturnSuccess) return r;
            if (locked) return kIOReturnSuccess;
        }
        IOSleep(100);
    }
    return r;
}

// ---- bus reset ------------------------------------------------------------------------------------------------------------

// ref BusReset @0x24216: keep the array and the device alive, replay on a new thread (UKernelCallHelpers::InvokeOnNewThread).
void com_openaudio_FWAvcConnectionManager::BusReset()
{
    if (!fConnections) return;
    fConnections->retain();
    fDevice->retain();
    this->retain();                                           // clone hardening: the thread uses `this`
    if (!IOCreateThread(&ReconnectP2PThread, this)) {
        this->release();
        fDevice->release();
        fConnections->release();
    }
}

// ref ReconnectP2P @0x24176: wait 1100 ms (0x44c), replay every record with failOnReset, stop on 0xE0008020, notify the engine.
void com_openaudio_FWAvcConnectionManager::ReconnectP2P()
{
    if (!fConnections || fConnections->getCount() == 0) return;
    IOSleep(0x44c);
    for (unsigned i = 0; i < fConnections->getCount(); i++) {
        com_openaudio_FWP2PConnection *c = OSDynamicCast(com_openaudio_FWP2PConnection, fConnections->getObject(i));
        if (!c) continue;
        if (updateP2PCount(c->pcrAddress, 1, true, c->channel, kSpeedUnchanged) == kIOFWBusReset) break;
    }
    if (fReconnected) fReconnected(fReconnectedRefcon);
}

// ref ReconnectP2PThread @0x23ff4: run, then release the array and the device.
void com_openaudio_FWAvcConnectionManager::ReconnectP2PThread(void *arg)
{
    com_openaudio_FWAvcConnectionManager *m = static_cast<com_openaudio_FWAvcConnectionManager *>(arg);
    m->ReconnectP2P();
    m->fConnections->release();
    m->fDevice->release();
    m->release();
}
