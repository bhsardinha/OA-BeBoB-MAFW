// com_openaudio_FWMetaNub -- polymorphic nub on IOFireWireDevice; owns the firmware state machine.
//
// Mirrors com_m_audio_FWMetaNub (A1 Q7, Q-INFO, Q-SDK5). Superclass is IOService, NOT IOFireWireUnit:
// the reference constructor (mafw.asm 0x1d1b4) calls IOService::IOService(OSMetaClass const*) directly and
// its OSMetaClass is registered with instance size 0x188 (mafw.asm 0x1d318..0x1d33a).
#ifndef OA_FWMETANUB_H
#define OA_FWMETANUB_H

#include <IOKit/IOService.h>
#include <IOKit/firewire/IOFireWireNub.h>
#include <kern/thread_call.h>

class OSDictionary;

class com_openaudio_FWMetaNub : public IOService
{
    OSDeclareDefaultStructors(com_openaudio_FWMetaNub)

public:
    // fState values (Q7 StateString @0x1b81e).
    enum {
        kStateNubStart        = 0,
        kStateInitialRegister = 1,
        kStateReregister      = 2,
        kStateFirmwareStart   = 3,
        kStateFirmwareHalt    = 4,
        kStateFirmwareUpdate  = 5,
    };

    virtual bool         init(OSDictionary *dict = 0) APPLE_KEXT_OVERRIDE;
    virtual void         free() APPLE_KEXT_OVERRIDE;
    virtual IOService   *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
    virtual bool         start(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void         stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual IOReturn     message(UInt32 type, IOService *provider, void *argument = 0) APPLE_KEXT_OVERRIDE;
    virtual IOReturn     setProperties(OSObject *properties) APPLE_KEXT_OVERRIDE;

    // Public so FWAVCAudioDevice::probe can read them (Q-SDK5 obs. 5: [+0xf0] and [+0xf3]).
    bool bootloaderActive() const        { return fBootloaderActive; }
    bool firmwareMatchStarted() const    { return fFirmwareMatchStarted; }
    IOFireWireNub *fireWireNub() const   { return fNub; }

    // Returns the primary product name for a primary or alias name, else NULL (mafw.asm 0x1ce06).
    static const char *GetActualProductName(const char *name);

    const char *StateString() const;

protected:
    bool         SetOneProperty(const OSString *key, OSObject *value);   // ref @0x1ca24
    IOReturn     FirmwareStart();                                        // ref @0x1beb4
    IOReturn     FirmwareCacheInfoRegisters();                           // ref @0x1cc9e
    IOReturn     FireWireWriteCommand(UInt32 addrHi, UInt32 addrLo, UInt32 len, const void *data); // ref @0x1bd3e
    IOReturn     FireWireReadCommand(UInt32 addrHi, UInt32 addrLo, UInt32 len, void *dst);         // ref @0x1c06e
    bool         CopyPropertyFrom(IOService *provider, const char *key);
    UInt32       ProtocolVersion() const;                                // raw quadlet [this+0x98]

    // Re-register path (ref @0x1bba6 / 0x1bc5e / 0x1bcc2). The reference arms a kernel timer on the FireWire
    // workloop whose handler spawns a thread (UKernelCallHelpers::InvokeOnNewThread). The clone uses one
    // thread_call with a 2500 ms delay: same delay, and the blocking FireWire reads run off the workloop
    // because the callout runs on a thread-call thread. Deviation is mechanical equivalence, not a 10.13 constraint.
    IOReturn     ReregisterDeviceAfterFirmwareReset();
    void         RegregisterDeviceThread();
    static void  ReregisterCallout(thread_call_param_t p0, thread_call_param_t p1);

    UInt32         fState;                 // [+0x88]
    IOFireWireNub *fNub;                   // [+0xe0]
    bool           fBootloaderActive;      // [+0xf0]
    bool           fUpdateInProgress;      // [+0xf1]  (Q7 called it fCached_IsRequestingClose; Q-P3 §5)
    bool           fServiceClosing;        // [+0xf2]
    bool           fFirmwareMatchStarted;  // [+0xf3]  sticky; set only by FirmwareStart
    bool           fStopped;               // [+0xf4]
    UInt8          fInfoBlock[0x50];       // [+0x90]  bootloader info block, published as MAFirmwareInfo
    thread_call_t  fReregisterCall;        // see above
    bool           fReregisterPending;

};

#endif
