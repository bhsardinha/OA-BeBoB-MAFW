// FW1814Device module (models/FW1814) -- reference com_m_audio_FW1814Device (Rev1). Everything here is what the FW1814 hardware makes
// different; the rest comes from base/. Contents of the module:
//  * FW1814Device.{h,cpp}: names, AllocateAudioEngine (FWEngine2) + SetupAudioEngine @0xed34, input-delay workaround @0xcba4, clock
//    (SetClockSourceInternal @0xe25c: HSCI vendor command + selector; SetBlankSlateClockSource @0xe4f0; SwitchDigitalSignals @0xe5dc for the
//    optical ADAT/S/PDIF modes), ReconnectP2P on bus reset, the 0x58-byte status packet (ReadHSCIData @0xd952: meters, encoders, A/B button,
//    lock/FDF), the mixer = a register block (groups + batch writers + shadow registers; MapChannelAndFunctionBlock @0xde84,
//    SetInputPan/Routing, SetAuxMonitor, SetHeadphoneSelector, LED vendor command).
//  * FW1814Settings: FWSettings1814Levels / FWSettings1814AuxSend (register-path overrides of the generic settings).
//  * FW1814Tables.h (rates, DeviceChannels rows, latency), FW1814MixerRegs.h (register map), FW1814Peaks (output peak procs).
// The com_m_audio_FW1814Device alias keeps the reference MIDI plug-in matching.
#ifndef OA_FW1814DEVICE_H
#define OA_FW1814DEVICE_H

#include "FWRev1AudioDevice.h"
#include "FWChannelGroup.h"
#include "FW1814Tables.h"

// Reference-name alias (A3-M2: the reference MIDI plug-in and panel find the device with IOServiceMatching("com_m_audio_FW1814Device"), which
// matches by class inheritance). An abstract class of that exact name sits between the chain and our concrete class so the unmodified
// reference user-space components still match the clone. It adds no behaviour. Conflicts with the reference kext (same class name) by design.
class com_m_audio_FW1814Device : public com_openaudio_FWRev1AudioDevice
{
    OSDeclareAbstractStructors(com_m_audio_FW1814Device)
};

class com_openaudio_FW1814Device : public com_m_audio_FW1814Device
{
    OSDeclareDefaultStructors(com_openaudio_FW1814Device)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    // ref FW1814Device::GetRoundTripLatencyForFDF @0xcf30 / GetDeviceChannelsForSampleRate @0xc998 (mode = MAClockSource bits [+0x144])
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const APPLE_KEXT_OVERRIDE { return FW1814RoundTripLatencyForFDF(fdf); }
    virtual void GetDeviceChannelsForSampleRate(UInt32 rateHz, FWDeviceChannels &out) const APPLE_KEXT_OVERRIDE { FW1814ComposeDeviceChannels(rateHz, fMode, out); }
    virtual UInt32 GetFirstSWReturnPeakMeterIndex() const APPLE_KEXT_OVERRIDE { return 0x26; }          // ref @0xc3ba
    virtual UInt32 GetSWReturnPeakMeterCount() const APPLE_KEXT_OVERRIDE { return 0xe; }               // ref @0xc3c6
    virtual com_openaudio_FWBaseEngine *AllocateAudioEngine() APPLE_KEXT_OVERRIDE;      // ref @0xd78a -> FWEngine2
    virtual void SetupAudioEngine(com_openaudio_FWBaseEngine *engine) APPLE_KEXT_OVERRIDE;   // ref @0xed34
    virtual void ResetRearrangementProcs(com_openaudio_FWBaseEngine *engine) APPLE_KEXT_OVERRIDE;   // ref @0xcad4
    virtual void IndexForAC3OutputChannels(UInt32 &left, UInt32 &right) const APPLE_KEXT_OVERRIDE { left = 4; right = 5; }   // ref @0xc3fe
    virtual const char *GetIconName() const APPLE_KEXT_OVERRIDE { return "FW 1814 box.png"; }          // ref @0xc96e
    virtual const char *GetDescriptionString() const APPLE_KEXT_OVERRIDE { return "FireWire 1814"; }   // ref @0xc97c
    virtual const char *GetShortDescriptionString() const APPLE_KEXT_OVERRIDE { return "FW 1814"; }    // ref @0xc98a
    virtual IOReturn InitializeUserSettableProperties() APPLE_KEXT_OVERRIDE;                          // ref @0xe6ca
    virtual void ResendCurrentMixerSettings() APPLE_KEXT_OVERRIDE;                                    // ref @0xd014 (parent + the four settings)
    virtual void ResetToFactorySettings() APPLE_KEXT_OVERRIDE;                                        // ref @0xd190
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) APPLE_KEXT_OVERRIDE;                // ref @0xe25c
    virtual IOReturn SetBlankSlateClockSource() APPLE_KEXT_OVERRIDE;                                    // ref @0xe4f0
    virtual void SwitchDigitalSignals(UInt32 clk) APPLE_KEXT_OVERRIDE;                                  // ref @0xe5dc
    void ReconnectP2P();                                                                                // ref @0xe1ea
    virtual void BusReset() APPLE_KEXT_OVERRIDE;                                                        // ref @0xe53a: ReconnectP2P on a new thread
    // hardware control poll (A1 Q-ME §5)
    virtual UInt32 GetControlPacketSize() const APPLE_KEXT_OVERRIDE { return 0x58; }                     // ref @0xc962
    virtual void ReadHSCIData(const UInt32 *packet) APPLE_KEXT_OVERRIDE;                                // ref @0xd952
    void ReceiveControlPacket(const UInt8 *packet);                                                     // ref @0xe9ea
    // ---- mixer (AP-2.2: the reference's device members; 0.1.0 kept them in a separate FW1814Mixer object) ----
    virtual IOReturn MapChannelAndFunctionBlock(FunctionBlockMapping &m) APPLE_KEXT_OVERRIDE;            // ref @0xde84
    // ref @0xcfb2 tail-calls the base MapRoutingParameters: not overridden
    virtual IOReturn SetHeadphoneSelector(UInt32 pair, UInt32 which) APPLE_KEXT_OVERRIDE;                 // ref @0xd1f8
    virtual IOReturn AVCControlSetLEDStatus(UInt32 state) APPLE_KEXT_OVERRIDE;                            // ref @0xdaaa
    // register writers reached from FWSettings1814Levels / FWSettings1814AuxSend (ref vt[0xbb8] / vt[0xbc0] / vt[0xbc8]); 1-based channel / pair
    IOReturn SetInputRouting(UInt32 ch, UInt32 route);                                                  // ref @0xdbea
    IOReturn SetInputPan(UInt32 ch, UInt32 pan);                                                        // ref @0xdd3e
    IOReturn SetAuxMonitor(UInt32 bus, UInt32 on);                                                      // ref @0xcfc4
    IOReturn CustomMixerWriteWithRotatedAuxChannels(UInt32 off, UInt32 quads, void *data);              // ref @0xe982
    IOReturn CustomMixerWriteBatch(UInt32 off, UInt32 quads, void *data) { return CustomMixerWriteQuads(off, quads, data); }   // batch functor target
    // M-S deviation (user decision 2026-10-17): after an optical switch that changes the channel counts the main engine is re-published, because
    // the 10.13 Audio MIDI Setup builds its "N ins / M outs" sidebar label only when the device is added (Q-AVC-unit-and-O1 section 3, capture 82).
    // The reference changes the stream formats in place (SwitchDigitalSignals @0xe5dc) and never re-creates the engine.
    void RepublishAudioEngine();
    // A re-publish drops every CoreAudio client of the device (reAMP froze mid-playback, 2026-10-17), so it only runs while the engine is idle;
    // during playback the formats change in place (as the reference) and the re-publish is deferred until the engine has stopped.
    void RepublishIfIdle();                                                                             // runs on the device work loop
    void DeferredRepublish();                                                                           // thread body
    virtual void audioEngineStopped() APPLE_KEXT_OVERRIDE;
    // ref @0xcba4: input converter for the 1814 -- reorders the first 8 channels and delays 6 of them by 16 frames
    void WorkaroundFor1814InputDelay(UInt32 count, const UInt32 *src, SInt32 *dst);
private:
    // channel groups (ref device offsets): Output <4> +0x1318, Input <18> +0x13e0, SW-return <4> +0x1668, Headphone <4> +0x1730,
    // AuxSend <22> +0x17f8, AuxOut <2> +0x1b00
    FWConcreteChannelGroup<4>  fOutput, fSWReturn, fHeadphone;
    FWConcreteChannelGroup<18> fInput;
    FWConcreteChannelGroup<22> fAuxSend;
    FWConcreteChannelGroup<2>  fAuxOut;
    com_openaudio_FWSettingsLevels        *fLevels;      // +0x538 (FWSettings1814Levels)
    com_openaudio_FWSettingsAuxSend       *fAux;         // +0xf98 (FWSettings1814AuxSend)
    com_openaudio_FWSettingsRotaryControl *fRotary;      // +0x470
    com_openaudio_FWSettingsABSwitch      *fAB;          // +0x2d8
    // shadow registers: 16-bit pan words (+0x12b8, host order, sent as raw memory), routing (+0x1308), headphone selector (+0x1310),
    // aux monitor (+0x1314). Routing / selector / monitor hold the memory image of the big-endian wire word.
    UInt16 fPan[18];
    UInt32 fRouting, fHpSel, fAuxMon;
    bool fClockBusy;                               // ref [+0x1b88]: SetClockSourceInternal in progress
    bool fRepublishPending;                         // clone (M-S): SwitchDigitalSignals changed the channel counts
    bool fRepublishDeferred;                        // clone (M-S): counts differ from the published engine's but a client was streaming
    bool fRepublishThread;                          // clone (M-S): a deferred-republish thread is alive
    UInt32 fPubIn, fPubOut;                         // clone (M-S): channel counts the HAL currently knows (0 = not recorded yet)
    com_openaudio_FWBaseEngine *fMidiDonor;         // clone (M-S): engine whose MIDI ports the next SetupAudioEngine adopts
    com_openaudio_FWNumber32Prop *fStereoPairProp;  // MAEnableStereoPairEngines (ref value cell [+0x184])
    // ref device [+0x1b8c] index table (identity * 6, set in the constructor @0xf04a), [+0x1bcc] 16 frames x 6 raw quadlets, [+0x1d4c] counter
    UInt32 fDelayHist[16 * 6];
    UInt32 fDelayCounter;
};

#endif
