// FW410Device module (models/FW410) -- see FW410Device.h.
#include "FW410Device.h"
#include "FW410Settings.h"
#include "FW410Status.h"
#include "FW410MixerRegs.h"
#include "FWEngine2.h"
#include "FWSamples.h"
#include "FWBaseEngine.h"
#include "FWSettings.h"
#include "FWPropertyDict.h"
#include "FWAvcFrames.h"

#define super com_openaudio_FWRev2AudioDevice
OSDefineMetaClassAndAbstractStructors(com_m_audio_FW410Device, com_openaudio_FWRev2AudioDevice)
#undef super
#define super com_m_audio_FW410Device
OSDefineMetaClassAndStructors(com_openaudio_FW410Device, com_m_audio_FW410Device)
#undef super

void com_openaudio_FW410Device::free()
{
    if (fLevels) { fLevels->release(); fLevels = 0; }
    if (fAux)    { fAux->release();    fAux = 0; }
    if (fRotary) { fRotary->release(); fRotary = 0; }
    com_openaudio_FWRev2AudioDevice::free();
}

void com_openaudio_FW410Device::SetupAudioEngine(com_openaudio_FWBaseEngine *engine)
{
    com_openaudio_FWEngine2 *e2 = OSDynamicCast(com_openaudio_FWEngine2, engine);
    if (!e2) return;
    e2->SetMIDIInputPorts(this, 1, 0);                                         // ref SetMIDIInputPorts(dev, 1, NULL)
    e2->SetMIDIOutputPorts(this, 1, 0);
    e2->SetupFromDevice(kFW410Channels.out, kFW410Channels.in, 1, false, true, false);   // [+0x2e0]=10 [+0x2e4]=4 [+0x2e8]=1, [+0x2d6]=1
    e2->SetLegacyEngineFlags(true, true);                                      // [+0x34a]=1 (UID "com_m_audio_FWEngine1"), [+0x34b]=1
    // installed once, as on the Audiophile (the base ResetRearrangementProcs is empty); at 176.4/192 kHz the base's channel-map path replaces
    // the rearrange and input procs (DeviceChannels maps of kFW410Channels192)
    e2->SetInputConvertProc(FWConvertLLRR4SkipMIDI1, this);                    // _ConvertLLRR4SkipMIDI1
    e2->SetRearrangeProc(FWRearrangeLLRR10WithMIDI);                           // _RearrangeFrameLLRR10WithMIDI
    e2->SetPeakProc(FWSamplePeaks10);                                          // _SamplePeaks10
}

// asm 0x14eda-0x14f56: the main engine ([+0xe8]) paused around the command; `blank` is not read.
IOReturn com_openaudio_FW410Device::SetClockSourceInternal(UInt32 clk, bool)
{
    com_openaudio_FWBaseEngine *e = fMainEngine;
    if (e) e->pauseAudioEngine();
    IOReturn r;
    if (clk <= 0x3fffffff) r = (IOReturn)0xE00002C2;
    else if ((clk & 3) == 0) r = AVCSetupSyncStream(0, 1);                     // internal
    else if ((clk & 3) == 1) r = AVCSetupSyncStream((clk & 4) ? 4 : 3, 1);     // S/PDIF: coax (bit 2) type 4, optical type 3
    else r = (IOReturn)0xE00002C2;
    if (e) e->resumeAudioEngine();
    return r;
}

// ref ReadHSCIData @0x15198 (decompile)
void com_openaudio_FW410Device::ReadHSCIData(const UInt32 *packet)
{
    if (fMeters) {
        UpdatePeakMeters(packet, kFW410HwInPeaks);                             // slots 0-3 <- q0-q3
        if (fSwPeaks) for (UInt32 k = 0; k < kFW410SWReturnSlots; k++) fMeters[4 + k] = fSwPeaks[4 + k];   // slots 4-13: SW returns (host)
        for (UInt32 k = 0; k < kFW410HwOutPeaks; k++)                          // slots 14-27 <- q4-q17
            fMeters[kFW410FirstHwOutSlot + k] = FWRev2PeakFromQuadlet(packet[kFW410HwInPeaks + k]);
    }
    const FW410Status st = FW410DecodeStatus(packet[kFW410StatusQuadlet]);
    DetectExternalSyncChanges(st.locked, st.fdf);
    // TRotaryControl::RotaryValueChanged @0xc2ba (step 1): 1 -> +1, 2 -> -1, anything else -> no change (as the Audiophile's nibbles)
    const SInt32 delta = st.encoder == 1 ? 1 : st.encoder == 2 ? -1 : 0;
    if (delta && fRotary) fRotary->UserRotatedHardwareKnob(delta);
}

// ---- register writers ----------------------------------------------------------------------------------------------------------------
IOReturn com_openaudio_FW410Device::SetInputLevel(UInt32 ch, UInt32 v)        // @0x14cf8
{
    if (ch - 1 >= 4) return (IOReturn)0xE00002C2;
    const UInt16 w = FW410Level16(v);
    if (w == fInLevel[ch - 1]) return kIOReturnSuccess;
    fInLevel[ch - 1] = w;
    const UInt32 hi = ch > 2 ? 1 : 0;
    return CustomMixerWriteQuads(kFW410RegInputLevels + 4 * hi, 1, &fInLevel[2 * hi]);
}

IOReturn com_openaudio_FW410Device::SetInputPan(UInt32 ch, UInt32 v)          // @0x14e7a: one quadlet per input pair
{
    if (ch - 1 >= 4) return (IOReturn)0xE00002C2;
    fInPan[ch - 1] = FW410Pan16(v);
    const UInt32 pair = (ch - 1) >> 1;
    return CustomMixerWriteQuads(kFW410RegInputPan + 4 * pair, 1, &fInPan[2 * pair]);
}

IOReturn com_openaudio_FW410Device::SetInputRouting(UInt32 pair, UInt32 mask) // @0x14d5a
{
    if (pair == 0 || pair > (fInput.Count() >> 1)) return (IOReturn)0xE00002C2;
    FW410InputRoutingWords(pair, mask, fInRouting);
    CustomMixerWriteQuads(kFW410RegInputRouting, 2, fInRouting);
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FW410Device::SetSWReturnRouting(UInt32 pair, UInt32 mask)   // @0x14076: shadow only, BatchSetSWReturnRouting writes
{
    return FW410SWReturnRoutingWord(pair, mask, fSWRouting) ? kIOReturnSuccess : (IOReturn)0xE00002C2;
}

void com_openaudio_FW410Device::BatchSetSWReturnRouting()                      // @0x14268
{
    CustomMixerWriteQuads(kFW410RegSWRouting, 1, &fSWRouting);
}

IOReturn com_openaudio_FW410Device::SetHeadphoneSource(UInt32 mask)           // @0x144bc
{
    fPhonesWord = FW410PhonesSourceWord(mask, fPhonesWord);
    return CustomMixerWriteQuads(kFW410RegPhonesSource, 1, &fPhonesWord);
}

// ---- settings objects --------------------------------------------------------------------------------------------------------------
// ref @0x14fca (decompile + asm): base properties; Rotary(SW return, output, input, headphone, aux out); batch writers at 0x00 / 0x14 / 0x28
// (multiplier 0x80) on the SW-return / output / input groups; AuxSend(aux sends, aux out, output count / 2 + 1); 410 Levels; the
// MAHeadphoneSourceV2 property; then the input-level shadow written once (vt 0xba8(0x28, 2, +0x1100)).
IOReturn com_openaudio_FW410Device::InitializeUserSettableProperties()
{
    IOReturn r = com_openaudio_FWRev2AudioDevice::InitializeUserSettableProperties();
    if (r != kIOReturnSuccess) return r;
    const IOReturn kFail = (IOReturn)0xE00002BC;
    fRotary = OSTypeAlloc(com_openaudio_FWSettingsRotaryControl);
    fAux    = OSTypeAlloc(com_openaudio_FWSettingsAuxSend);
    fLevels = OSTypeAlloc(com_openaudio_FWSettings410Levels);
    if (!fRotary || !fAux || !fLevels) return kFail;
    if ((r = fRotary->Init(this, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut)) != kIOReturnSuccess) return r;
    typedef com_openaudio_FW410Device D;
    const FWCallBatch w = FWBindBatch<D, &D::CustomMixerWriteBatch>(this);
    fSWReturn.SetupBatch(w, kFW410RegSWReturnLevels);
    fOutput.SetupBatch(w, kFW410RegOutputLevels);
    fInput.SetupBatch(w, kFW410RegInputLevels);
    if ((r = fAux->Init(this, &fAuxSend, &fAuxOut, (fOutput.Count() >> 1) + 1)) != kIOReturnSuccess) return r;
    if ((r = fLevels->Init410(this, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut)) != kIOReturnSuccess) return r;
    fPhonesSource = fPropDict ? fPropDict->AddNumber32("MAHeadphoneSourceV2", FWBind1<D, &D::SetHeadphoneSource>(this)) : 0;
    if (!fPhonesSource) return kFail;
    return CustomMixerWriteQuads(kFW410RegInputLevels, 2, fInLevel);
}

void com_openaudio_FW410Device::ResendCurrentMixerSettings()                   // @0x14548
{
    com_openaudio_FWRev2AudioDevice::ResendCurrentMixerSettings();
    if (!fLevels) return;
    fRotary->SendToDevice();
    fAux->SendToDevice();
    fLevels->SendToDevice();
    if (fPhonesSource) SetHeadphoneSource(fPhonesSource->value());
}

void com_openaudio_FW410Device::ResetToFactorySettings()                       // @0x142de
{
    com_openaudio_FWRev2AudioDevice::ResetToFactorySettings();
    if (!fLevels) return;
    fRotary->ResetToFactorySettings();
    fAux->ResetToFactorySettings();
    fLevels->ResetToFactorySettings();                                         // ref calls FWSettingsLevels::ResetToFactorySettings (base) directly
    if (fPhonesSource) fPhonesSource->set(1);                                  // [+0x1174] = 1: phones on out 1/2 (manual p.28)
}
