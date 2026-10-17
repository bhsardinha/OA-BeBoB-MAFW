// FW1814UserClient -- the vendor user client (A1 Q-UC-surface, Q11; decomp FWUserClient__*.c). Mirrors com_m_audio_FWUserClient:
//   * legacy IOExternalMethod table, selectors 500..507 (MIDI start / stop / stop-alias / send, DiceSet, DiceGet, WaitForTermination,
//     SetInitializedProperty)
//   * clientMemoryForType: 1 = the 4 KiB meter page uint32 peak[52] (read-only), 0x400+n = MIDI source ring n (read-only),
//     0x800+n = MIDI destination ring n
//   * notification type 1 = MIDI-in push channel (320-byte messages); type 0 is the base class (property changes)
// Deviations: clientClose also terminate()s the client (the reference leaks it, Q-UC Correction R16); WaitForTermination waits on an
// IOLock condition instead of a Mach semaphore (no semaphore KPI dependency).
#ifndef OA_FW1814USERCLIENT_H
#define OA_FW1814USERCLIENT_H

#include "FWKIPCUserClient.h"
#include "FWAudioDeviceChain.h"

class com_openaudio_FW1814UserClient : public com_openaudio_FWKIPCUserClient
{
    OSDeclareDefaultStructors(com_openaudio_FW1814UserClient)
public:
    virtual bool initWithTask(task_t owningTask, void *securityToken, UInt32 type) APPLE_KEXT_OVERRIDE;   // ref @0x773e
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;                                          // ref @0x76de
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;                                           // ref @0x766c
    virtual bool willTerminate(IOService *provider, IOOptionBits options) APPLE_KEXT_OVERRIDE;            // ref @0x76c2
    virtual IOReturn clientClose() APPLE_KEXT_OVERRIDE;                                                   // ref @0x71a8
    virtual IOReturn clientMemoryForType(UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory) APPLE_KEXT_OVERRIDE;   // ref @0x7220
    virtual IOExternalMethod *getTargetAndMethodForIndex(IOService **target, UInt32 index) APPLE_KEXT_OVERRIDE;                 // ref @0x72f6
    virtual IOReturn registerNotificationPort(mach_port_t port, UInt32 type, UInt32 refCon) APPLE_KEXT_OVERRIDE;                // ref @0x74aa

    // selectors (ref sMethods @0x339e0)
    IOReturn EntryStartMIDI();                                            // 500
    IOReturn EntryStopMIDI();                                             // 501, 502
    IOReturn EntrySendMIDI(void *packet, UInt32 *result);                 // 503 (280-byte MAMIDIPacket in, 4 bytes out)
    IOReturn EntryDiceSetRequest(void *) { return (IOReturn)0xE00002C7; } // 504 (reference: unsupported)
    IOReturn EntryDiceGetRequest(void *, void *) { return (IOReturn)0xE00002C7; }   // 505
    IOReturn WaitForTermination();                                        // 506 (virtual in the reference, vt[0x968])
    IOReturn SetInitializedProperty();                                    // 507 (vt[0x970])

    // ref ReceivedMIDIPacket @0x757e: push one received MIDI packet to the type-1 port (M-H calls this)
    IOReturn ReceivedMIDIPacket(const void *packet);

private:
    static IOExternalMethod sMethods[8];
    com_openaudio_FWAudioDevice *fDevice;                // [+0x290]
    bool fMidiStarted;                                   // [+0x140]
    Notification fMidiNote;                              // [+0x148 .. +0x287] 320-byte template for type 1 (size 0x140)
    UInt8 fMidiNoteBody[0x140 - sizeof(Notification)];
    IOLock *fTermLock;                                   // replaces [+0x288] semaphore
    bool fTerminating;
};

#endif
