// FW1814Mixer -- everything of the FW1814 device that InitializeUserSettableProperties @0xe6ca builds for the mixer: the six channel groups,
// the four settings objects, the batch writers and the shadow registers (A1 Q-ME §2-§4). The reference keeps these as members of the
// device object (offsets in the comments); the clone gathers them in one OSObject owned by the device.
#ifndef OA_FWMIXER_H
#define OA_FWMIXER_H

#include <libkern/c++/OSObject.h>
#include <libkern/OSByteOrder.h>
#include "FWSettings.h"
#include "FWMixerRegs.h"

class com_openaudio_FWAVCAudioDevice;

class com_openaudio_FW1814Mixer : public OSObject
{
    OSDeclareDefaultStructors(com_openaudio_FW1814Mixer)
public:
    // ref FW1814Device::InitializeUserSettableProperties (mixer part): groups, settings, batch writers, A/B switch functors
    bool Init(com_openaudio_FWAVCAudioDevice *device, com_openaudio_FWPropertyDict *dict);
    virtual void free() APPLE_KEXT_OVERRIDE;

    // ref FW1814Device::ResendCurrentMixerSettings @0xd014 (derived part: ABSwitch, Rotary, AuxSend, Levels) / ResetToFactorySettings @0xd190
    void ResendCurrentMixerSettings();
    void ResetToFactorySettings();

    // device register writers (ref FW1814Device vt[0xbb8] SetInputRouting, vt[0xbc0] SetInputPan, vt[0xbc8] SetAuxMonitor, SetHeadphoneSelector
    // @0xd1f8, AVCControlSetLEDStatus @0xdaaa); idx values are the 1-based channel / pair numbers of the property setters
    IOReturn SetInputRouting(UInt32 ch, UInt32 route);
    IOReturn SetInputPan(UInt32 ch, UInt32 pan);
    IOReturn SetAuxMonitor(UInt32 bus, UInt32 on);
    IOReturn SetHeadphoneSelector(UInt32 pair, UInt32 which);
    IOReturn SetLEDStatus(UInt32 state);
    // AV/C-routed writes (ref FWSettingsLevels::SetAVCRouting / FWSettingsAuxSend::SetAuxOutChannelFeature)
    IOReturn SetAvcRouting(bool connect, UInt8 srcPair, UInt8 dstPair);
    IOReturn SetAuxOutChannelFeature(UInt32 channel, UInt8 controlSelector, UInt16 value);
    // ref CustomMixerWriteQuads (vt[0xba8]) and CustomMixerWriteWithRotatedAuxChannels @0xe982
    IOReturn WriteQuads(UInt32 registerOffset, UInt32 quads, void *data);
    IOReturn WriteRotatedAux(UInt32 registerOffset, UInt32 quads, void *data);

    IOAudioEngine *engine() const;
    // M-S deviation: CoreAudio controls follow the main engine when the device re-publishes it
    void DetachAudioControls(IOAudioEngine *oldEngine) { if (fLevels) fLevels->DetachAudioControls(oldEngine); }
    void EstablishAudioControls()                      { if (fLevels) fLevels->EstablishAudioControls(); }                                          // the device's main engine (ref device [+0xe8])
    UInt8 auxMonitorByte3() const { return (UInt8)(fAuxMon >> 24); }      // ref device byte [+0x1317]: aux-monitor bus 3 select (bit 2)
    com_openaudio_FWSettingsABSwitch      *abSwitch() const { return fAB; }
    com_openaudio_FWSettingsRotaryControl *rotary() const   { return fRotary; }
    com_openaudio_FWSettingsLevels        *levels() const   { return fLevels; }
    com_openaudio_FWSettingsAuxSend       *auxSend() const  { return fAux; }

private:
    com_openaudio_FWAVCAudioDevice *fDevice;
    // channel groups (ref device offsets): Output <4> +0x1318, Input <18> +0x13e0, SW-return <4> +0x1668, Headphone <4> +0x1730,
    // AuxSend <22> +0x17f8, AuxOut <2> +0x1b00
    FWConcreteChannelGroup<4>  fOutput, fSWReturn, fHeadphone;
    FWConcreteChannelGroup<18> fInput;
    FWConcreteChannelGroup<22> fAuxSend;
    FWConcreteChannelGroup<2>  fAuxOut;
    com_openaudio_FWSettingsLevels        *fLevels;      // +0x538
    com_openaudio_FWSettingsAuxSend       *fAux;         // +0xf98
    com_openaudio_FWSettingsRotaryControl *fRotary;      // +0x470
    com_openaudio_FWSettingsABSwitch      *fAB;          // +0x2d8
    // shadow registers: 16-bit pan words (+0x12b8, host order, sent as raw memory), routing (+0x1308), headphone selector (+0x1310),
    // aux monitor (+0x1314). Routing / selector / monitor hold the memory image of the big-endian wire word.
    UInt16 fPan[18];
    UInt32 fRouting, fHpSel, fAuxMon;
};

#endif
