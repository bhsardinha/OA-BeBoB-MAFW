// FWSettings* -- the mixer settings objects: Levels, AuxSend, RotaryControl, ABSwitch, and the FW1814 subclasses FWSettings1814Levels /
// FWSettings1814AuxSend. Mirror the reference classes of the same names (A1 Q-ME §3, Q15; decomp FWSettings*__*.c). Each owns the property
// keys of its area (literal MA* names from the decompiles). The generic setters reach the hardware only through the device: per-model
// MapChannelAndFunctionBlock / MapRoutingParameters (vt[0xb48] / vt[0xb40]) and the AV/C function-block builders; the FW1814 subclasses
// override SetInputPan / SetInputRouting / SetAuxMonitor to call the FW1814 register writers (ref vt[0xbc0] / vt[0xbb8] / vt[0xbc8]).
// AP-2.2 (plan v3): 0.1.0 routed all of this through a FW1814Mixer object; the split is now the reference's.
//
// The per-channel level setters (Set*Level, AV/C feature blocks) are installed on every group as in the reference; on the FW1814 the device
// then installs a batch writer on each group, which takes precedence (BaseChannelGroup::SendLevelsToDevice), so they are never called there.
// CoreAudio controls (output master + per-channel volume and mute, input volume and mute, global mute, and - M-U, a v1.9-only addition chosen
// by the user on 2026-10-17 - 18 'pass' input-level volumes + 1 'pass' mute on MADirectMonitoringActive) are created in FWSettingsLevels::Init.
// The reference binary has no caller for the direct-monitoring and clock-source control creators (EstablishDirectMonitoring*Control,
// EstablishClockSourceControl), so there are none here either.
#ifndef OA_FWSETTINGS_H
#define OA_FWSETTINGS_H

#include <libkern/c++/OSObject.h>
#include "FWPropertyDict.h"
#include "FWChannelGroup.h"

class com_openaudio_FWAVCAudioDevice;

class com_openaudio_FWSettings : public OSObject
{
    OSDeclareAbstractStructors(com_openaudio_FWSettings)
public:
    virtual IOReturn SendToDevice() { return kIOReturnSuccess; }                // ref FWSettings::SendToDevice
    virtual IOReturn ResetToFactorySettings() { return kIOReturnSuccess; }
protected:
    com_openaudio_FWAVCAudioDevice *fDevice;                                    // ref [+0x8] (Levels also keeps it at [+0xa58], AuxSend at [+0x2d0])
    com_openaudio_FWPropertyDict *fDict;                                        // the device's dictionary (ref device [+0xf8])
};

// ---- Levels -----------------------------------------------------------------------------------------------------------------------
class com_openaudio_FWSettingsLevels : public com_openaudio_FWSettings
{
    OSDeclareDefaultStructors(com_openaudio_FWSettingsLevels)
public:
    // ref FWSettingsLevels::Init(dev, swReturn, output, input, headphone, auxOut): the groups are members of the device
    IOReturn Init(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *swReturn, FWChannelGroup *output,
                  FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut);
    virtual IOReturn SendToDevice() APPLE_KEXT_OVERRIDE;
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;

    // per-channel setters (group functors): AV/C feature block, control selector 2 (volume), map kinds 3 / 2 / 6 / 1
    IOReturn SetSWReturnLevel(UInt32 ch, UInt32 v);                             // ref @0x22396
    IOReturn SetOutputLevel(UInt32 ch, UInt32 v);                               // ref @0x22428
    IOReturn SetInputLevel(UInt32 ch, UInt32 v);                                // ref @0x21890
    IOReturn SetHeadphonesLevel(UInt32 ch, UInt32 v);                           // ref @0x224ba
    virtual IOReturn SetInputChannelFeature(UInt32 ch, UInt8 controlSelector, UInt32 v);   // ref vt+0x48 @0x2254c
    virtual IOReturn SetAVCRouting(UInt32 kind, UInt32 nDst, UInt32 srcPair, UInt32 mask); // ref vt+0x60 @0x225b4
    virtual IOReturn SetSWReturnRouting(UInt32 idx, UInt32 v);                  // ref @0x218a6, vt+0x70 (FW410: FWSettings410Levels @0x14466)
    virtual void BatchSetSWReturnRouting() {}                                   // ref vt+0x78: empty in the base; FW410 @0x1449a writes the routing word
    virtual IOReturn SetInputPan(UInt32 idx, UInt32 v);                         // ref @0x21878 (FW1814: FWSettings1814Levels @0xd128)
    virtual IOReturn SetInputRouting(UInt32 idx, UInt32 v);                     // ref @0x218f6 (FW1814: FWSettings1814Levels @0xd15c)
    IOReturn SetDirectMonitoringActive(UInt32 v);
    IOReturn SetGlobalMute(UInt32 v);
    IOReturn SetGlobalDim(UInt32 v);
    // M-S deviation: the CoreAudio controls belong to the main engine, which the device re-publishes after an optical switch
    void EstablishAudioControls();
    void DetachAudioControls(IOAudioEngine *engine);

protected:
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
    IOReturn Init(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *auxSend, FWChannelGroup *auxOut, UInt32 nMonitor);   // ref FWSettingsAuxSend::Init
    virtual IOReturn SendToDevice() APPLE_KEXT_OVERRIDE;
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;
    IOReturn SetAuxSendLevel(UInt32 ch, UInt32 v);                              // group functor: map kind 0, feature sel 2
    IOReturn SetAuxOutputLevel(UInt32 ch, UInt32 v);                            // group functor: SetAuxOutChannelFeature(ch, 2, v)
    IOReturn SetAuxOutChannelFeature(UInt32 ch, UInt8 controlSelector, UInt32 v);   // map kind 7
    IOReturn SetAuxPan(UInt32 idx, UInt32 v);                                   // SetAuxOutChannelFeature(idx, 3, -v)
    virtual IOReturn SetAuxMonitor(UInt32 idx, UInt32 v);                       // map kind 5 -> selector (FW1814: FWSettings1814AuxSend @0xd0b4)
protected:
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
    IOReturn Init(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *swReturn, FWChannelGroup *output,
                  FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut);
    virtual IOReturn ResetToFactorySettings() APPLE_KEXT_OVERRIDE;
    void UserRotatedHardwareKnob(SInt32 delta);                                 // ref @ FWSettingsRotaryControl
    // ref SetPairLevelsFromRotary(key, pair, &array, delta): `arr` is created from the current property on first use (caller releases)
    void SetPairLevelsFromRotary(const char *key, UInt32 pair, OSArray *&arr, SInt32 delta);
    IOReturn VirtualRotateKnob(OSObject *v);
    // ref SetLevelsFromKnob(mask, key, firstBit, group, delta): public, the Audiophile's headphone encoder calls it directly (ReadHSCIData @0xba92)
    void SetLevelsFromKnob(UInt64 mask, const char *key, UInt32 firstBit, FWChannelGroup *g, SInt32 delta);
private:
    FWChannelGroup *fGroups[5];                                                 // [+0x10 .. +0x30]
    com_openaudio_FWNumber64Prop *fRotary;                                      // MARotaryControlV2 (mask lives in its value, ref [+0x68])
    com_openaudio_FWFunctionProp *fVirtual;                                     // MAMetaVirtualRotateDataKnob
    // clone (plan v7 AP-5.4): knob step for the phones levels (outputs and aux out: official law since R127), ½-dB per detent: 6, 3, or 0 = official law
    com_openaudio_FWNumber32Prop *fMonitorStep;                                 // MAMonitorKnobStep
    IOReturn SetMonitorKnobStep(UInt32 v);
};

// ---- A/B switch + headphone source -----------------------------------------------------------------------------------------------
class com_openaudio_FWSettingsABSwitch : public com_openaudio_FWSettings
{
    OSDeclareDefaultStructors(com_openaudio_FWSettingsABSwitch)
public:
    IOReturn Init(com_openaudio_FWAVCAudioDevice *device);
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
