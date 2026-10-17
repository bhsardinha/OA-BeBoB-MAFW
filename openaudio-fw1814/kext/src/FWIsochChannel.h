// FWIsochChannel + FWRemoteIsochPort + ForceStopNotificationProc. Mirrors m_audio_b_FWIsochChannel (A1 Q-MC, decomp
// FWIsochChannel__*.c), m_audio_b_FWRemoteIsochPort (FWRemoteIsochPort__*.c) and ForceStopNotificationProc @0x25766.
// Wiring (CreateIsochChannelAndPorts @0x255c2, verified slots in Q-MC): the LOCAL DCL port is the channel's TALKER and the
// REMOTE port is its LISTENER, for both TX and RX programs (Q-CMP-connect §1 had the two swapped; corrected).
#ifndef OA_FWISOCHCHANNEL_H
#define OA_FWISOCHCHANNEL_H

#include <IOKit/IOLocks.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/firewire/IOFWIsochChannel.h>
#include <IOKit/firewire/IOFWIsochPort.h>

class com_openaudio_FWAvcConnectionManager;

class com_openaudio_FWIsochChannel : public IOFWIsochChannel
{
    OSDeclareDefaultStructors(com_openaudio_FWIsochChannel)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    // ref ReleaseTalker @0x6ee6 (panics while running; the clone refuses): talker->releasePort(); release; fTalker = NULL.
    IOReturn ReleaseTalker();
    // ref setTalker @0x6f32: rejects NULL / same port (0xE00002C2), refuses while running, retains the new port,
    // releases the old talker, then IOFWIsochChannel::setTalker.
    virtual IOReturn setTalker(IOFWIsochPort *talker) APPLE_KEXT_OVERRIDE;
    virtual IOReturn start() APPLE_KEXT_OVERRIDE;          // ref @0x6eb0: 0xE00002C2 if already running
    virtual IOReturn stop() APPLE_KEXT_OVERRIDE;           // ref @0x6e7c: 0xE00002C2 if not running
    bool     isRunning() const { return fRunning; }
    IOFWIsochPort *talker() const { return fTalker; }      // the local DCL port (channel+0x28 in the reference)
    UInt32   channelNumber() const { return fChannel; }    // channel+0x4c in the reference (ResetPort)
private:
    bool fRunning;                                          // ref [+0x78]
};

// ref FWRemoteIsochPort: the device end. All CMP work is delegated to the connection manager.
class com_openaudio_FWRemoteIsochPort : public IOFWIsochPort
{
    OSDeclareDefaultStructors(com_openaudio_FWRemoteIsochPort)
public:
    // ref init @0x183c6 (stores [+0x10] manager, [+0x18] flag, [+0x1c] plug). `remoteIsInput` = the local program is the
    // talker (TX) so the device end is an input plug (the reference stores the inverse, [+0x18] = !talking).
    bool init(com_openaudio_FWAvcConnectionManager *mgr, bool remoteIsInput, UInt32 plug);
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual IOReturn getSupported(IOFWSpeed &maxSpeed, UInt64 &chanSupported) APPLE_KEXT_OVERRIDE;   // 0x7fffffff, all channels
    virtual IOReturn allocatePort(IOFWSpeed speed, UInt32 chan) APPLE_KEXT_OVERRIDE;                // -> manager->Connect
    virtual IOReturn releasePort() APPLE_KEXT_OVERRIDE;                                              // -> manager->Disconnect
    virtual IOReturn start() APPLE_KEXT_OVERRIDE { return kIOReturnSuccess; }                       // ref @0x1843e
    virtual IOReturn stop() APPLE_KEXT_OVERRIDE  { return kIOReturnSuccess; }                       // ref @0x18446
private:
    com_openaudio_FWAvcConnectionManager *fMgr;
    bool   fRemoteIsInput;
    UInt32 fPlug;
};

#endif
