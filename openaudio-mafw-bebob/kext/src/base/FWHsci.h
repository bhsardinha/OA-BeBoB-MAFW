// FWHSCI -- the 50 ms status poll of the FW1814 (A1 Q-ME §5; decomp FWHSCI__*.c). Reads GetControlPacketSize() bytes (0x58) from
// 0xFFC7:0x00600000 with an IOFWReadQuadCommand, hands the buffer to the device (ReadHSCIData) and re-arms. The packet carries the
// hardware meters, the rotary / button state and the clock lock + FDF.
//
// Deviation (kept visible): the reference processes every completed read on a NEW kernel thread (HSCIReadComplete ->
// InvokeOnNewThread(HSCISafelyComplete)); the clone processes it on the device work loop (a 0 ms timer hop out of the FireWire thread),
// the same thread that applies property changes, so the shared meter/property state is single-threaded. Nothing else changes.
#ifndef OA_FWHSCI_H
#define OA_FWHSCI_H

#include <libkern/c++/OSObject.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/firewire/IOFireWireNub.h>
#include <IOKit/firewire/IOFWCommand.h>

class com_openaudio_FWHsci : public OSObject
{
    OSDeclareDefaultStructors(com_openaudio_FWHsci)
public:
    typedef void (*PacketCallback)(void *refcon, const UInt32 *packet);
    // ref @0xe064 (StartReceptionOfControlPackets): size, nub, callback = ReadHSCIData
    bool Init(IOFireWireNub *nub, UInt32 sizeBytes, PacketCallback cb, void *refcon);
    bool Start(IOWorkLoop *wl);          // ref Start: counted; the first Start arms the 50 ms timer unless paused
    void Stop();                         // ref Stop: counted; the last Stop cancels
    void Pause();                        // ref Pause / Resume: counted, independent of Start
    void Resume();
    void Release();                      // ref Release: cancel, free the buffer
    virtual void free() APPLE_KEXT_OVERRIDE;
private:
    static void TimerFiredStatic(OSObject *owner, IOTimerEventSource *t);
    static void ProcessStatic(OSObject *owner, IOTimerEventSource *t);
    static void ReadCompleteStatic(void *refcon, IOReturn status, IOFireWireNub *device, IOFWCommand *cmd);
    void TimerFired();                   // ref HSCITimerFired @0x1d600
    void Process();                      // ref HSCISafelyComplete @0x1d4d6
    IOFireWireNub *fNub;                 // [+0x10]
    SInt32 fStarts;                      // [+0x18]
    SInt32 fPauses;                      // [+0x1c]
    IOTimerEventSource *fPoll;           // [+0x20] the poll timer
    IOTimerEventSource *fDone;           // clone: hop to the work loop after a completed read
    IOFWReadQuadCommand *fCmd;           // [+0xa0]
    IOWorkLoop *fLoop;                   // [+0xa8]
    PacketCallback fCb; void *fCbRef;    // [+0xb8]
    UInt32 fSize;                        // [+0xc0]
    UInt32 *fBuf;                        // [+0xc8]
    bool fArmed;
    bool fInFlight;                      // clone: an async read is outstanding (retain held until it completes)
    bool fDead;                          // clone: Release() ran; a late completion must not touch the timers
};

#endif
