// FWCMP -- Connection Management Procedures for the FW1814. Mirrors m_audio_b_FWAVCConnectionManager (A1 Q-CMP-connect, Q-MC;
// decomp FWAVCConnectionManager__*.c) and FWP2PConnection. The CMP register work (device oPCR/iPCR compare-and-swap, host
// PCR plug) is done by the kext itself, never by IOFireWireAVC (Q-CMP-connect §3 corrected Q3a).
//   device oPCR[n] = 0xFFFF:F0000904 + 4n   (device -> host; the RX program)
//   device iPCR[n] = 0xFFFF:F0000984 + 4n   (host -> device; the TX program)
#ifndef OA_FWCMP_H
#define OA_FWCMP_H

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSArray.h>
#include <IOKit/IOService.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/firewire/IOFireWireNub.h>

// ref FWP2PConnection: one live device-side connection, replayed after a bus reset. [+0xc] PCR address, [+0x10] channel.
class com_openaudio_FWP2PConnection : public OSObject
{
    OSDeclareDefaultStructors(com_openaudio_FWP2PConnection)
public:
    bool Init() { pcrAddress = 0; channel = 0; return init(); }
    UInt32 pcrAddress;
    UInt32 channel;
};

class com_openaudio_FWAvcConnectionManager : public OSObject
{
    OSDeclareDefaultStructors(com_openaudio_FWAvcConnectionManager)
public:
    // ref InitWithDevice @0x24dfe stores the audio device [+0x10]; the nub is the device's provider nub ([dev+0xe0]).
    // `device` supplies the work loop CMP actions run on (ref vt[0x688] on the device = getWorkLoop).
    bool InitWithDevice(IOService *device, IOFireWireNub *nub);
    virtual void free() APPLE_KEXT_OVERRIDE;

    // ref Connect @0x248ce: remoteIsInput -> device iPCR path (host talks), else device oPCR path (host listens).
    IOReturn Connect(bool remoteIsInput, UInt32 channel, UInt32 plug);
    // ref Disconnect @0x2479a
    IOReturn Disconnect(bool remoteIsInput, UInt32 channel, UInt32 plug);
    // ref BusReset @0x24216: replays every recorded connection on a new thread (ReconnectP2PThread).
    void BusReset();
    // Engine hook run after a reconnect (ref: [[dev+0xe8]]->vt[0xcb0]); installed by the engine milestone (M-D/M-J).
    typedef void (*ReconnectedProc)(void *refcon);
    void SetReconnectedProc(ReconnectedProc p, void *refcon) { fReconnected = p; fReconnectedRefcon = refcon; }

    // ref updateP2PCount @0x244f0: read, modify, compare-and-swap, up to 4 attempts, IOSleep(100) between them.
    //   delta >= 1 -> point-to-point count +1; otherwise count cleared. channel 0xFFFFFFFF / speed 0x80000000 = unchanged.
    IOReturn updateP2PCount(UInt32 pcrAddrLo, SInt32 delta, bool failOnReset, UInt32 channel, UInt32 speed);

    // IOWorkLoop::Action trampolines (ref CoreMakeP2POutputConnection @0x23f86 / ...Input @0x23f50 / CoreBreak... @0x23fdc, @0x23f6e)
    static IOReturn CoreMakeP2POutputConnection(OSObject *owner, void *plug, void *channel, void *, void *);
    static IOReturn CoreMakeP2PInputConnection(OSObject *owner, void *plug, void *channel, void *, void *);
    static IOReturn CoreBreakP2POutputConnection(OSObject *owner, void *plug, void *, void *, void *);
    static IOReturn CoreBreakP2PInputConnection(OSObject *owner, void *plug, void *, void *, void *);

private:
    IOReturn Connect_iPCR(UInt32 plug, UInt32 channel);               // ref @0x247ba
    IOReturn Disconnect_iPCR(UInt32 plug);                            // ref @0x246ba
    IOReturn makeP2PInputConnection(UInt32 plug, UInt32 channel);     // ref @0x243f2 (on the work loop)
    IOReturn makeP2POutputConnection(UInt32 plug, UInt32 channel);    // ref @0x24336
    IOReturn breakP2PInputConnection(UInt32 plug);                    // ref @0x24398
    IOReturn breakP2POutputConnection(UInt32 plug);                   // ref @0x242dc
    IOReturn makeConnection(UInt32 pcrAddrLo, UInt32 channel, UInt32 speed);   // ref @0x24980
    IOReturn breakConnection(UInt32 pcrAddrLo);                       // ref @0x24454
    void     ReconnectP2P();                                          // ref @0x24176
    static void ReconnectP2PThread(void *arg);                        // ref @0x23ff4

    IOService     *fDevice;                // [+0x10]
    IOFireWireNub *fNub;                   // [[dev]+0xe0]
    OSArray       *fConnections;           // [+0x18] of com_openaudio_FWP2PConnection
    UInt32         fHostPlug[3];           // [+0x2c] host PCR plug numbers, [+0x28] = 3 entries
    // clone (capture 97, 2026-10-20): which fHostPlug slots hold a plug allocated in the host PCR space. The reference allocates on every
    // Connect_iPCR and frees only in Disconnect_iPCR (free @0x23e96 never frees them): a plug left allocated keeps HandlePCRLock registered,
    // and after a kextunload the next bus reset calls into the unloaded code (panic, IOFireWirePCRSpace -> HandlePCRLockStub).
    bool           fHostPlugAllocated[3];
    void           FreeHostPlugs();
    ReconnectedProc fReconnected;
    void          *fReconnectedRefcon;
};

#endif
