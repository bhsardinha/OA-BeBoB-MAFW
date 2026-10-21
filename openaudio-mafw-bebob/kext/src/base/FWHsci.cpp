#include "FWHsci.h"
#include "FWRegisterWindows.h"
#include "Trace.h"

OSDefineMetaClassAndStructors(com_openaudio_FWHsci, OSObject)

static const UInt32 kPollMs  = 0x32;     // ref HSCITimerFired / HSCISafelyComplete: setTimeoutMS(0x32)
static const UInt32 kRetryMs = 1000;     // ref: submit or read failure -> retry in 1000 ms

bool com_openaudio_FWHsci::Init(IOFireWireNub *nub, UInt32 sizeBytes, PacketCallback cb, void *refcon)
{
    if (!OSObject::init() || !nub) return false;
    fNub = nub; fSize = sizeBytes; fCb = cb; fCbRef = refcon;
    fStarts = fPauses = 0; fPoll = fDone = 0; fCmd = 0; fLoop = 0; fBuf = 0; fArmed = false; fInFlight = false; fDead = false;
    return true;
}

bool com_openaudio_FWHsci::Start(IOWorkLoop *wl)
{
    if (fStarts++ != 0) return true;                                   // ref: counted
    if (!fPoll) {
        fBuf = static_cast<UInt32 *>(IOMalloc((size_t)(fSize >> 2) << 2));
        if (!fBuf) { fStarts = 0; return false; }
        bzero(fBuf, (size_t)(fSize >> 2) << 2);
        fLoop = wl;
        fPoll = IOTimerEventSource::timerEventSource(this, &com_openaudio_FWHsci::TimerFiredStatic);
        fDone = IOTimerEventSource::timerEventSource(this, &com_openaudio_FWHsci::ProcessStatic);
        if (!fPoll || !fDone || wl->addEventSource(fPoll) != kIOReturnSuccess || wl->addEventSource(fDone) != kIOReturnSuccess) {
            Release();
            if (fBuf) { IOFree(fBuf, (size_t)(fSize >> 2) << 2); fBuf = 0; }     // nothing has been read yet, so no command can be using it
            fStarts = 0;
            return false;
        }
        fArmed = true;
    }
    if (fPauses == 0) fPoll->setTimeoutMS(kPollMs);                    // ref: enable + setTimeoutMS(0x32)
    return true;
}

void com_openaudio_FWHsci::Stop()
{
    if (fStarts == 0 || !fPoll) return;
    if (--fStarts == 0) { fPoll->cancelTimeout(); fDone->cancelTimeout(); }
}

void com_openaudio_FWHsci::Pause()
{
    if (fPauses++ == 0 && fStarts != 0 && fPoll) fPoll->cancelTimeout();
}

void com_openaudio_FWHsci::Resume()
{
    if (--fPauses == 0 && fStarts != 0 && fPoll) fPoll->setTimeoutMS(kPollMs);
}

void com_openaudio_FWHsci::Release()
{
    // Correction (2026-10-16, second integrated run): Release() used to call fCmd->cancel() here. FW1814Device::stop runs on the IOService
    // termination thread and kextunload hung there for good (uninterruptible, no panic), the cancel being a synchronous trip through the
    // FireWire workloop. The cancel is not needed: the retain() taken in TimerFired for every read pins this object (and with it the kext) until
    // the completion fires, and the command and buffer are only freed in free(), after the last read has completed.
    fDead = true;
    if (fPoll) { fPoll->cancelTimeout(); if (fLoop) fLoop->removeEventSource(fPoll); fPoll->release(); fPoll = 0; }
    if (fDone) { fDone->cancelTimeout(); if (fLoop) fLoop->removeEventSource(fDone); fDone->release(); fDone = 0; }
    fArmed = false;
}

void com_openaudio_FWHsci::free()
{
    Release();
    if (fCmd) { fCmd->release(); fCmd = 0; }
    if (fBuf) { IOFree(fBuf, (size_t)(fSize >> 2) << 2); fBuf = 0; }
    OSObject::free();
}

void com_openaudio_FWHsci::TimerFiredStatic(OSObject *owner, IOTimerEventSource *)
{
    if (com_openaudio_FWHsci *h = OSDynamicCast(com_openaudio_FWHsci, owner)) h->TimerFired();
}
void com_openaudio_FWHsci::ProcessStatic(OSObject *owner, IOTimerEventSource *)
{
    if (com_openaudio_FWHsci *h = OSDynamicCast(com_openaudio_FWHsci, owner)) h->Process();
}

// ref HSCITimerFired @0x1d600: zero the buffer, read size/4 quadlets from 0xFFC7:0x600000 (the nub's current generation)
void com_openaudio_FWHsci::TimerFired()
{
    if (!fBuf || !fNub || fDead || fInFlight) return;
    bzero(fBuf, (size_t)(fSize >> 2) << 2);
    FWAddress addr(kFWMixerWindowHi, kFWHsciWindowLo);
    IOReturn r;
    if (!fCmd) {
        fCmd = fNub->createReadQuadCommand(addr, fBuf, (int)(fSize >> 2), &com_openaudio_FWHsci::ReadCompleteStatic, this, false);
        r = fCmd ? kIOReturnSuccess : kIOReturnNoMemory;
    } else {
        // IOFWReadQuadCommand::reinit returns an IOReturn (0 = success). Correction (2026-10-16, integrated run): the clone tested it as a
        // bool, so every poll after the first one failed silently and fell back to the 1 s retry with no packet: meters, front-panel knob
        // and A/B switch never updated.
        r = fCmd->reinit(addr, fBuf, (int)(fSize >> 2), &com_openaudio_FWHsci::ReadCompleteStatic, this, false);
    }
    if (r == kIOReturnSuccess) {
        // Correction (2026-10-16, panic on kextunload during the integrated run): the read is asynchronous; if the object or the kext went away
        // while it was outstanding, AppleFWOHCI completed it into freed memory (IOFireWireFamily page fault, R8 = 0x600000, RAX = 0xffc7). Hold
        // a reference for the duration of every read (it pins the object and the kext until the completion fires; Release() does not cancel, see there).
        fInFlight = true;
        retain();
        r = fCmd->submit();
        if (r != kIOReturnSuccess && fInFlight) { fInFlight = false; release(); }
    }
    static UInt32 sFails = 0;                                           // bench diagnostics (integrated run 2026-10-16)
    if (r != kIOReturnSuccess && sFails++ < 20) OA_LOG("HSCI submit/reinit failed 0x%x (#%u)", r, (unsigned)sFails);
    if (r != kIOReturnSuccess && fPoll) fPoll->setTimeoutMS(kRetryMs);
}

// ref HSCIReadComplete @0x1d80e: error -> retry in 1000 ms; success -> process (clone: on the work loop)
void com_openaudio_FWHsci::ReadCompleteStatic(void *refcon, IOReturn status, IOFireWireNub *, IOFWCommand *)
{
    com_openaudio_FWHsci *h = static_cast<com_openaudio_FWHsci *>(refcon);
    if (!h) return;
    static UInt32 sFails = 0;                                                 // bench diagnostics (integrated run 2026-10-16)
    if (status != kIOReturnSuccess && status != kIOReturnAborted && sFails++ < 20) OA_LOG("HSCI read failed 0x%x (#%u)", status, (unsigned)sFails);
    const bool wasInFlight = h->fInFlight;
    h->fInFlight = false;
    if (!h->fDead && h->fPoll && h->fDone) {
        if (status != kIOReturnSuccess) h->fPoll->setTimeoutMS(kRetryMs);
        else h->fDone->setTimeoutMS(0);
    }
    if (wasInFlight) h->release();                                            // the reference taken in TimerFired
}

// ref HSCISafelyComplete @0x1d4d6: hand the packet to the device, re-arm 50 ms
void com_openaudio_FWHsci::Process()
{
    if (fDead) return;
    if (fCb && fBuf) fCb(fCbRef, fBuf);
    if (fPoll && fStarts != 0 && fPauses == 0) fPoll->setTimeoutMS(kPollMs);
}
