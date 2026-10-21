// FWAudiophileDevice module (models/FWAudiophile) -- reference com_m_audio_FWAudiophileDevice (Rev2; A1 Q-AP0, Q-AP0b, Q-AP0c, Q-AP0d;
// object layout Q-AP0 §4, instance size 0x17E0). Everything here is what the Audiophile hardware makes different; the rest comes from base/
// (FWRev2AudioDevice: 0x2F stream format, SIGNAL SOURCE sync, control-packet poll, hardware peaks). Build order of the module (plan v4):
//  AP-3.1 identity, tables, engine setup (this file) · AP-3.2 converters + peak proc · AP-3.3 clock / bring-up · AP-3.4 status packet ·
//  AP-3.5 mixer mapping · AP-3.6 settings objects, resend, factory reset.
// The com_m_audio_FWAudiophileDevice alias keeps the reference MIDI plug-in / panel matching (as the FW1814's alias).
#ifndef OA_FWAUDIOPHILEDEVICE_H
#define OA_FWAUDIOPHILEDEVICE_H

#include "FWRev2AudioDevice.h"
#include "FWAudiophileTables.h"
#include "FWChannelGroup.h"
#include "FWAudiophileMixerMap.h"

class com_openaudio_FWSettingsLevels;
class com_openaudio_FWSettingsAuxSend;
class com_openaudio_FWSettingsRotaryControl;
class com_openaudio_FWSettingsABSwitch;

// Reference-name alias: abstract, no behaviour (see com_m_audio_FW1814Device).
class com_m_audio_FWAudiophileDevice : public com_openaudio_FWRev2AudioDevice
{
    OSDeclareAbstractStructors(com_m_audio_FWAudiophileDevice)
};

class com_openaudio_FWAudiophileDevice : public com_m_audio_FWAudiophileDevice
{
    OSDeclareDefaultStructors(com_openaudio_FWAudiophileDevice)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual const char *GetIconName() const APPLE_KEXT_OVERRIDE { return "FW AP box.png"; }                    // decomp GetIconName
    virtual const char *GetDescriptionString() const APPLE_KEXT_OVERRIDE { return "FireWire Audiophile"; }     // decomp GetDescriptionString
    virtual const char *GetShortDescriptionString() const APPLE_KEXT_OVERRIDE { return "FWAudiophile"; }      // decomp GetShortDescriptionString
    virtual void GetDeviceChannelsForSampleRate(UInt32 rateHz, FWDeviceChannels &out) const APPLE_KEXT_OVERRIDE   // @0xafb4: one table
        { (void)rateHz; out = kFWAudiophileChannels; }
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const APPLE_KEXT_OVERRIDE { return FWAudiophileRoundTripLatencyForFDF(fdf); }
    virtual void IndexForAC3OutputChannels(UInt32 &left, UInt32 &right) const APPLE_KEXT_OVERRIDE { left = 2; right = 5; }   // decomp
    virtual UInt32 GetFirstSWReturnPeakMeterIndex() const APPLE_KEXT_OVERRIDE { return 0xe; }   // @0xaa02; count = base (out channels = 6)
    virtual UInt32 GetControlPacketSize() const APPLE_KEXT_OVERRIDE { return 0x3c; }            // @0xaf7e (vt 0xba0): 60-byte status packet
    // ref SetupAudioEngine (decomp FWAudiophileDevice__SetupAudioEngine.c): m = 1, MIDI ports 1/1, in 4 / out 6 (stream = channels), then the
    // input converter, output rearrangement and peak procs (_ConvertLLRR4SkipMIDI1, _RearrangeSampleFrameLLRR6, _SamplePeaks6). It does not set the FW1814's engine flags [+0x2d5]/[+0x2d6]/[+0x348].
    virtual void SetupAudioEngine(com_openaudio_FWBaseEngine *engine) APPLE_KEXT_OVERRIDE;
    // ref SetClockSourceInternal @0xb9d4 (asm 0xb9d4-0xba4e; Q-AP0 §2.3): MAClockSource -> SIGNAL SOURCE. Internal or S/PDIF only (no word
    // clock, no "internal muted"). The base SetBlankSlateClockSource calls it with 0x40000000 = internal.
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) APPLE_KEXT_OVERRIDE;
    // ref ReadHSCIData @0xba92 (AP-3.4): hardware peaks (UpdatePeakMeters 14), SW-return slots 14-19 from the host peaks, lock/FDF ->
    // DetectExternalSyncChanges, the two encoders and the A/B button (FWAudiophileStatus.h)
    virtual void ReadHSCIData(const UInt32 *packet) APPLE_KEXT_OVERRIDE;
    // ---- mixer addressing (AP-3.5; FWAudiophileMixerMap.h) ----
    virtual IOReturn MapChannelAndFunctionBlock(FunctionBlockMapping &m) APPLE_KEXT_OVERRIDE { return (IOReturn)FWAudiophileMapChannel(m); }   // @0xb0e2
    virtual IOReturn MapRoutingParameters(Routing &r) APPLE_KEXT_OVERRIDE { return (IOReturn)FWAudiophileMapRouting(r); }                   // @0xba50
    // ref SetHeadphoneSelector (decomp FWAudiophileDevice__SetHeadphoneSelector.c): map kind 8 (selector fb 4), then a 12-byte selector frame
    // 00 08 B8 80 <fb> 10 02 <which> 01 00 00 00 through AVCControlFunctionBlock (async). `pair` is the mapping channel (single headphone).
    virtual IOReturn SetHeadphoneSelector(UInt32 pair, UInt32 which) APPLE_KEXT_OVERRIDE;
    // ref AVCControlSetLEDStatus @0xb7d0 (asm): AVCVendorCommand(2, 0, state) -> 00 FF 00 00 0D 6C 02 00 01 <state> 00 00 00 00 00 00
    virtual IOReturn AVCControlSetLEDStatus(UInt32 state) APPLE_KEXT_OVERRIDE { return AVCVendorCommand(2, 0, (UInt8)state); }
    // ---- settings objects (AP-3.6) ----
    // ref InitializeUserSettableProperties @0xb7e6: base properties, then Rotary (groups SW, out, in, hp, aux out), AuxSend (aux sends, aux
    // out, Output.Count() >> 1 = 3 monitor switches), Levels (SW, out, in, hp, aux out), ABSwitch, its LED (vt 0xbc8) and headphone-selector
    // functors, UpdateLEDs. The generic settings classes are used as they are (the Audiophile's mixer is all AV/C; no batch writers).
    virtual IOReturn InitializeUserSettableProperties() APPLE_KEXT_OVERRIDE;
    virtual void ResendCurrentMixerSettings() APPLE_KEXT_OVERRIDE;   // ref @0xb00e: base (throttle + clock), ABSwitch, Rotary, AuxSend, Levels
    virtual void ResetToFactorySettings() APPLE_KEXT_OVERRIDE;       // ref @0xb078: base ([+0x144] = 0x40000000), ABSwitch, Rotary, AuxSend, Levels
private:
    // Mixer surface (ref object layout, Q-AP0 §4): settings objects ABSwitch +0x2d0, Rotary +0x468, AuxSend +0x530, Levels +0x808, and the
    // $_667 channel groups at +0x1270. Built in InitializeUserSettableProperties (AP-3.6); NULL / unused until then.
    com_openaudio_FWSettingsABSwitch      *fAB;
    com_openaudio_FWSettingsRotaryControl *fRotary;
    com_openaudio_FWSettingsAuxSend       *fAux;
    com_openaudio_FWSettingsLevels        *fLevels;
    FWConcreteChannelGroup<6>  fOutput;      // +0x1270
    FWConcreteChannelGroup<4>  fInput;       // +0x1378
    FWConcreteChannelGroup<6>  fSWReturn;    // +0x1440
    FWConcreteChannelGroup<2>  fHeadphone;   // +0x1548
    FWConcreteChannelGroup<10> fAuxSend;     // +0x15d0
    FWConcreteChannelGroup<2>  fAuxOut;      // +0x1758
};

#endif
