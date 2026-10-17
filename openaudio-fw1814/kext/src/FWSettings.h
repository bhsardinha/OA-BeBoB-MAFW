// FWSettings* -- the mixer settings objects of the FW1814: Levels, AuxSend, RotaryControl, ABSwitch. Mirror FWSettingsLevels,
// FWSettingsAuxSend, FWSettingsRotaryControl, FWSettingsABSwitch (A1 Q-ME §3; decomp FWSettings*__*.c). Each owns the property keys of
// its area (literal MA* names from the decompiles) and the hardware-facing setters. The FW1814 subclasses of the reference
// (FWSettings1814Levels / FWSettings1814AuxSend) only override SetInputPan / SetInputRouting / SetAuxMonitor to call the device; here those
// calls go to the mixer object, which owns the shadow registers.
//
// Status (2026-10-16): the two AV/C-based writes (MASWReturnRoutingV2 -> SetAVCRouting/AVCMixerConnectPair, MAAuxOutputPan2 ->
// AVCControlFeatureFunctionBlock) live in FWMixer.cpp (queued asynchronously through FWAvcUnit::AVCCommandAsync as in the reference since 2026-10-17; the earlier synchronous send timed out), and the
// CoreAudio controls (output master + per-channel volume and mute, input volume and mute, global mute, and - M-U, a v1.9-only addition chosen
// by the user on 2026-10-17 - 18 'pass' input-level volumes + 1 'pass' mute on MADirectMonitoringActive) are created in FWSettingsLevels::Init.
// The reference binary has no caller for the direct-monitoring and clock-source control creators (EstablishDirectMonitoring*Control,
// EstablishClockSourceControl), so the FW1814 has none either. The per-channel (non-batch) level setters of the reference (SetOutputLevel etc.,
// AV/C feature blocks) are never used on the FW1814, which installs a batch writer on every group.
#ifndef OA_FWSETTINGS_H
#define OA_FWSETTINGS_H

#include <libkern/c++/OSObject.h>
#include "FWPropertyDict.h"
#include "FWChannelGroup.h"

class com_openaudio_FW1814Mixer;

class com_openaudio_FWSettings : public OSObject
{
    OSDeclareAbstractStructors(com_openaudio_FWSettings)
public:
    virtual IOReturn SendToDevice() { return kIOReturnSuccess; }                // ref FWSettings::SendToDevice
    virtual IOReturn ResetToFactorySettings() { return kIOReturnSuccess; }
protected:
    com_openaudio_FW1814Mixer *fMixer;
    com_openaudio_FWPropertyDict *fDict;
};

// ---- Levels -----------------------------------------------------------------------------------------------------------------------
class com_openaudio_FWSettingsLevels : public com_openaudio_FWSettings
{
    OSDeclareDefaultStructors(com_openaudio_FWSettingsLevels)
public:
    // ref FWSettingsLevels::Init(dev, swReturn, output, input, headphone, auxOut) @0x2...: the groups are owned by the mixer
    IOReturn Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict, FWChannelGroup *swReturn, FWChannelGroup *output,
                  FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut);
    virtual IOReturn SendToDevice() APPLE_KEXT_OVERRIDE;
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;

    IOReturn SetSWReturnRouting(UInt32 idx, UInt32 v);                          // AV/C (E4)
    IOReturn SetInputPan(UInt32 idx, UInt32 v);                                 // ref FWSettings1814Levels::SetInputPan
    IOReturn SetInputRouting(UInt32 idx, UInt32 v);                             // ref FWSettings1814Levels::SetInputRouting
    IOReturn SetDirectMonitoringActive(UInt32 v);
    IOReturn SetGlobalMute(UInt32 v);
    IOReturn SetGlobalDim(UInt32 v);
    // M-S deviation: the CoreAudio controls belong to the main engine, which the device re-publishes after an optical switch
    void EstablishAudioControls();
    void DetachAudioControls(IOAudioEngine *engine);

private:
    IOReturn InitGroupProp(const char *name, int slot, FWChannelGroup *g, bool solo, FWChannelGroup *soloSecond);
    FWChannelGroup *fSWReturn, *fOutput, *fInput, *fHeadphone, *fAuxOut;       // [+0x8b0 .. +0x8c8], aux out = Init's last argument
    FWGroupChannelGroup fUnionIn;                                               // [+0x8d0] SW-return + input
    FWGroupChannelGroup fUnionOut;                                              // [+0x930] output + aux out + headphone
    com_openaudio_FWChannelGroupProp *fGroupProp[4][4];                         // [group: SWReturn, Output, Input, Headphone][Levels, Mute, Solo, Link]
    com_openaudio_FWArrayProp  *fPan;                                           // MAInputPan2        [+0x10]
    com_openaudio_FWArrayProp  *fInputRouting;                                  // MAInputRoutingV2   [+0x990]
    com_openaudio_FWArrayProp  *fSWReturnRouting;                               // MASWReturnRoutingV2 [+0x9f0]
    com_openaudio_FWNumber32Prop *fDirectMon, *fGlobalMute, *fGlobalDim;        // [+0xf0] [+0x70] [+0xb0]
};

// ---- AuxSend ----------------------------------------------------------------------------------------------------------------------
class com_openaudio_FWSettingsAuxSend : public com_openaudio_FWSettings
{
    OSDeclareDefaultStructors(com_openaudio_FWSettingsAuxSend)
public:
    IOReturn Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict, FWChannelGroup *auxSend, FWChannelGroup *auxOut, UInt32 nMonitor);
    virtual IOReturn SendToDevice() APPLE_KEXT_OVERRIDE;
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;
    IOReturn SetAuxPan(UInt32 idx, UInt32 v);                                   // AV/C (E4)
    IOReturn SetAuxMonitor(UInt32 idx, UInt32 v);                               // ref FWSettings1814AuxSend::SetAuxMonitor
private:
    FWChannelGroup *fAuxSend, *fAuxOut;                                         // [+0x10] [+0x18]
    UInt32 fMonitorCount;                                                       // [+0x2c8]
    com_openaudio_FWChannelGroupProp *fSendLevels, *fOutLevels, *fOutMute, *fOutLink;
    com_openaudio_FWArrayProp *fMonitor, *fPan;                                 // MAMonitorAuxOnOutput [+0x200], MAAuxOutputPan2 [+0x260]
};

// ---- Rotary control ---------------------------------------------------------------------------------------------------------------
class com_openaudio_FWSettingsRotaryControl : public com_openaudio_FWSettings
{
    OSDeclareDefaultStructors(com_openaudio_FWSettingsRotaryControl)
public:
    IOReturn Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict, FWChannelGroup *swReturn, FWChannelGroup *output,
                  FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut);
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;
    void UserRotatedHardwareKnob(SInt32 delta);                                 // ref @ FWSettingsRotaryControl
    // ref SetPairLevelsFromRotary(key, pair, &array, delta): `arr` is created from the current property on first use (caller releases)
    void SetPairLevelsFromRotary(const char *key, UInt32 pair, OSArray *&arr, SInt32 delta);
    IOReturn VirtualRotateKnob(OSObject *v);
private:
    void SetLevelsFromKnob(UInt64 mask, const char *key, UInt32 firstBit, FWChannelGroup *g, SInt32 delta);
    FWChannelGroup *fGroups[5];                                                 // [+0x10 .. +0x30]
    com_openaudio_FWNumber64Prop *fRotary;                                      // MARotaryControlV2 (mask lives in its value, ref [+0x68])
    com_openaudio_FWFunctionProp *fVirtual;                                     // MAMetaVirtualRotateDataKnob
};

// ---- A/B switch + headphone source -----------------------------------------------------------------------------------------------
class com_openaudio_FWSettingsABSwitch : public com_openaudio_FWSettings
{
    OSDeclareDefaultStructors(com_openaudio_FWSettingsABSwitch)
public:
    IOReturn Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict);
    virtual IOReturn SendToDevice() APPLE_KEXT_OVERRIDE;
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;
    void SetLEDFunctor(const FWCall1 &f) { fLed = f; }                          // ref [+0x18]
    void SetHeadphoneFunctor(const FWCall2 &f) { fHpSelect = f; }               // ref [+0x28]
    void UpdateLEDs();                                                          // ref @0xe96a
    void MomentarySwitchPressed();
    IOReturn SetMomentarySwitchRoutingSource(UInt32 idx, UInt32 v);
    IOReturn SetHeadphoneSource(UInt32 v);
    IOReturn SetHeadphoneSecondSource(UInt32 v);
    IOReturn SetMomentarySwitchSource(UInt32 v);
    static IOReturn NullSet32(UInt32) { return kIOReturnSuccess; }
private:
    void SubmitHeadphoneSourceChange();
    void SendMixerSettingsForCurrentMomentaryMode();
    UInt32 SourceMaskToHeadphoneSelector(UInt32 mask) const;
    FWCall1 fLed;                                                               // [+0x18] AVCControlSetLEDStatus(state)
    FWCall2 fHpSelect;                                                          // [+0x28] SetHeadphoneSelector(pair, which)
    com_openaudio_FWNumber32Prop *fMomentarySource;                             // MAMomentarySwitchSource   (value at ABSwitch +0x5c)
    com_openaudio_FWArrayProp    *fRoutings;                                    // MAMomentarySwitchRoutings (n = 4)
    com_openaudio_FWNumber32Prop *fLatent;                                      // MAMomentarySwitchLatent
    com_openaudio_FWNumber32Prop *fHeadphoneSource;                             // MAHeadphoneSourceV2       (value at +0x144)
    com_openaudio_FWNumber32Prop *fHeadphoneSecond;                             // MAHeadphoneSecondSource   (value at +0x184)
    UInt32 fMode;                                                               // [+0xd0] 0 = A, 1 = B
    UInt32 fHeadphoneOverride;                                                  // [+0x104]
};

#endif
