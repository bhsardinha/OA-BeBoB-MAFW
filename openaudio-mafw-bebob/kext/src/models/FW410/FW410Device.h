// FW410Device module (models/FW410) -- reference com_m_audio_FW410Device (FWRev2AudioDevice; vtable @0x3c600; A1 Q-410-0, A2 PN410, plan
// docs/BUILD_FW410_PLAN_v2.md). A Rev2 device (0x2F stream format, SIGNAL SOURCE sync, control-packet poll) with a REGISTER mixer like the
// FW1814's: the SW-return / output / input level groups have batch writers (CustomMixerWriteQuads, 0xFFC7:0x00700000 + offset) and pan,
// routing and the phones source go through the 410's own register writers (FW410MixerRegs.h); aux sends, aux out, phones level and the aux
// monitor switches keep the generic AV/C paths (FW410MixerMap.h).
// The com_m_audio_FW410Device alias keeps the reference MIDI plug-in (com.m-audio.midiplug.fw410) and panel matching.
#ifndef OA_FW410DEVICE_H
#define OA_FW410DEVICE_H

#include "FWRev2AudioDevice.h"
#include "FWChannelGroup.h"
#include "FW410Tables.h"
#include "FW410MixerMap.h"

class com_openaudio_FWSettings410Levels;
class com_openaudio_FWSettingsAuxSend;
class com_openaudio_FWSettingsRotaryControl;
class com_openaudio_FWNumber32Prop;

class com_m_audio_FW410Device : public com_openaudio_FWRev2AudioDevice
{
    OSDeclareAbstractStructors(com_m_audio_FW410Device)
};

class com_openaudio_FW410Device : public com_m_audio_FW410Device
{
    OSDeclareDefaultStructors(com_openaudio_FW410Device)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual const char *GetIconName() const APPLE_KEXT_OVERRIDE { return "FW 410 box.png"; }                   // decomp GetIconName
    virtual const char *GetDescriptionString() const APPLE_KEXT_OVERRIDE { return "FireWire 410"; }            // decomp GetDescriptionString
    virtual const char *GetShortDescriptionString() const APPLE_KEXT_OVERRIDE { return "FW410"; }             // decomp GetShortDescriptionString
    virtual void GetDeviceChannelsForSampleRate(UInt32 rateHz, FWDeviceChannels &out) const APPLE_KEXT_OVERRIDE   // asm 0x14010
        { out = rateHz > 96000 ? kFW410Channels192 : kFW410Channels; }
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const APPLE_KEXT_OVERRIDE { return FW410RoundTripLatencyForFDF(fdf); }
    virtual void IndexForAC3OutputChannels(UInt32 &left, UInt32 &right) const APPLE_KEXT_OVERRIDE { left = 0; right = 5; }   // decomp: wire slots of S/PDIF L/R
    virtual UInt32 GetFirstSWReturnPeakMeterIndex() const APPLE_KEXT_OVERRIDE { return 4; }    // decomp; count = base (out channels)
    virtual UInt32 GetControlPacketSize() const APPLE_KEXT_OVERRIDE { return 0x4c; }           // decomp (vt 0xba0): 76-byte status packet
    // ref SetupAudioEngine (decomp): MIDI 1/1, out 10 / in 4 / midi 1 (stream = channels), [+0x2d6] = [+0x34a] = [+0x34b] = 1, procs
    // _RearrangeFrameLLRR10WithMIDI / _SamplePeaks10 / _ConvertLLRR4SkipMIDI1 (+ arg = this)
    virtual void SetupAudioEngine(com_openaudio_FWBaseEngine *engine) APPLE_KEXT_OVERRIDE;
    // ref SetClockSourceInternal @0x14eda (asm): gate bit 30; clk & 3 == 0 internal (sync type 0), == 1 S/PDIF: coax (bit 2) type 4, optical type 3
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) APPLE_KEXT_OVERRIDE;
    virtual void ReadHSCIData(const UInt32 *packet) APPLE_KEXT_OVERRIDE;                     // ref @0x15198 (FW410Status.h)
    virtual IOReturn MapChannelAndFunctionBlock(FunctionBlockMapping &m) APPLE_KEXT_OVERRIDE { return (IOReturn)FW410MapChannel(m); }   // @0x14600
    virtual IOReturn MapRoutingParameters(Routing &r) APPLE_KEXT_OVERRIDE { return (IOReturn)FW410MapRouting(r); }                   // @0x1428a
    virtual IOReturn InitializeUserSettableProperties() APPLE_KEXT_OVERRIDE;                 // ref @0x14fca
    virtual void ResendCurrentMixerSettings() APPLE_KEXT_OVERRIDE;                           // ref @0x14548
    virtual void ResetToFactorySettings() APPLE_KEXT_OVERRIDE;                               // ref @0x142de

    // ---- the 410's register writers (vt 0xbc8..0xbe8 in the reference; FW410MixerRegs.h) ----
    IOReturn SetInputRouting(UInt32 pair, UInt32 mask);    // vt 0xbc8 @0x14d5a
    IOReturn SetInputPan(UInt32 ch, UInt32 v);             // vt 0xbd0 @0x14e7a
    IOReturn SetSWReturnRouting(UInt32 pair, UInt32 mask); // vt 0xbd8 @0x14076 (shadow only)
    void     BatchSetSWReturnRouting();                     // vt 0xbe0 @0x14268
    // vt 0xbe8 @0x14cf8. No caller in v1.11.2 (R136); the input-level batch writer covers the same registers. Kept reachable (plan v2 §6).
    IOReturn SetInputLevel(UInt32 ch, UInt32 v);
    IOReturn SetHeadphoneSource(UInt32 mask);               // @0x144bc, MAHeadphoneSourceV2 (6 sources; the sixth = mask bit 5, capture 104)
    IOReturn CustomMixerWriteBatch(UInt32 off, UInt32 quads, void *data) { return CustomMixerWriteQuads(off, quads, data); }   // batch functor target

private:
    com_openaudio_FWSettingsRotaryControl *fRotary;        // ref +0x2d8 (TRotaryControl at +0x2d0)
    com_openaudio_FWSettingsAuxSend       *fAux;           // ref +0x3a0
    com_openaudio_FWSettings410Levels     *fLevels;        // ref +0x678
    com_openaudio_FWNumber32Prop          *fPhonesSource;  // ref +0x1148, value +0x1174
    UInt16 fInLevel[4];                                    // ref +0x1100
    UInt16 fInPan[4];                                      // ref +0x1110
    UInt32 fInRouting[2];                                  // ref +0x1134 / +0x1138
    UInt32 fSWRouting;                                     // ref +0x113c
    UInt32 fPhonesWord;                                    // ref +0x1140
    FWConcreteChannelGroup<10> fOutput;                    // ref +0x1188
    FWConcreteChannelGroup<4>  fInput;                     // ref +0x1310
    FWConcreteChannelGroup<10> fSWReturn;                  // ref +0x13d8
    FWConcreteChannelGroup<2>  fHeadphone;                 // ref +0x1560
    FWConcreteChannelGroup<14> fAuxSend;                   // ref +0x15e8
    FWConcreteChannelGroup<2>  fAuxOut;                    // ref +0x17f0
};

#endif
