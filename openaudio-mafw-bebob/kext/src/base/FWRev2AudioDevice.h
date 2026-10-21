// FWRev2AudioDevice module (base) -- reference FWRev2AudioDevice (A1 Q-AP0 §2, R77, R85, R96). Models: Audiophile, FW410, Solo, Ozonic,
// NRV10, Lightbridge. Adds to the AV/C layer: stream format through EXTENDED STREAM FORMAT INFORMATION (0x2F: C1 list walk, then C0 set of
// the matching entry), sync through SIGNAL SOURCE (0x1A) with the source from GetAddressAndPlugForSyncType (vt 0xbc0), the Rev2 latency
// table, UpdatePeakMeters (hardware peaks from the status packet). A Rev2 model supplies: names, channel table, MapChannelAndFunctionBlock,
// MapRoutingParameters, SetClockSourceInternal (which sync type per MAClockSource value), GetControlPacketSize + ReadHSCIData, its settings
// objects (InitializeUserSettableProperties), SetHeadphoneSelector, AVCControlSetLEDStatus.
#ifndef OA_FWREV2AUDIODEVICE_H
#define OA_FWREV2AUDIODEVICE_H

#include "FWAVCAudioDevice.h"

// Rev2 (A1 Q-AP0 §2, R77, R85): Audiophile, FW410, Solo, Ozonic, NRV10, Lightbridge. Stream format through AV/C EXTENDED STREAM FORMAT (0x2F,
// C1 list walk then C0 set), sync through SIGNAL SOURCE (0x1A), the 50 ms control-packet poll of GetControlPacketSize() bytes. The ctor @0x13574
// adds no fields; the HSCI lives at [+0x2c8]. Never used for the FW1814 (its firmware freezes on 0x2F, snd-bebob bebob_maudio.c:30-33).
class com_openaudio_FWRev2AudioDevice : public com_openaudio_FWAVCAudioDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWRev2AudioDevice)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual IOReturn AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf) APPLE_KEXT_OVERRIDE;    // ref @0x12e7e
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const APPLE_KEXT_OVERRIDE;                       // ref @0x12a54
    virtual IOReturn GetAddressAndPlugForSyncType(UInt32 type, UInt8 plug, UInt8 &addr, UInt8 &srcPlug);   // ref vt[0xbc0] @0x12ae8
    IOReturn AVCSetupSyncStream(UInt32 type, UInt8 plug);                                                  // ref @0x12c60
    // Clone addition (plan v4 AP-3.10, R92): after the reference bring-up, a read-only EXTENDED PLUG INFO query of iso plug 0 in both directions
    // (channel position + cluster info) logged to MADebugTrace, to settle the physical channel order before any audio. The reference
    // FWRev2AudioDevice::initHardware only forwards to the parent.
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;
    void QueryPlugChannelInfo();
    virtual com_openaudio_FWBaseEngine *AllocateAudioEngine() APPLE_KEXT_OVERRIDE;                       // ref @0xb66a: new FWEngine2 (every Rev2 model)
    void UpdatePeakMeters(const UInt32 *buf, UInt32 n);                                                    // ref: hardware peaks -> [+0x108]
};

#endif
