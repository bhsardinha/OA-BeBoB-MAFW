// FWAudiophileDevice module (models/FWAudiophile) -- see FWAudiophileDevice.h.
#include "FWAudiophileDevice.h"
#include "FWEngine2.h"
#include "FWSamples.h"
#include "FWBaseEngine.h"
#include "FWAudiophileStatus.h"
#include "FWSettings.h"

#define super com_openaudio_FWRev2AudioDevice
OSDefineMetaClassAndAbstractStructors(com_m_audio_FWAudiophileDevice, com_openaudio_FWRev2AudioDevice)
#undef super
#define super com_m_audio_FWAudiophileDevice
OSDefineMetaClassAndStructors(com_openaudio_FWAudiophileDevice, com_m_audio_FWAudiophileDevice)
#undef super

void com_openaudio_FWAudiophileDevice::free()
{
    if (fLevels) { fLevels->release(); fLevels = 0; }
    if (fAux)    { fAux->release();    fAux = 0; }
    if (fRotary) { fRotary->release(); fRotary = 0; }
    if (fAB)     { fAB->release();     fAB = 0; }
    com_openaudio_FWRev2AudioDevice::free();
}

void com_openaudio_FWAudiophileDevice::SetupAudioEngine(com_openaudio_FWBaseEngine *engine)
{
    com_openaudio_FWEngine2 *e2 = OSDynamicCast(com_openaudio_FWEngine2, engine);
    if (!e2) return;
    e2->SetMIDIInputPorts(this, 1, 0);                                         // ref SetMIDIInputPorts(dev, 1, NULL)
    e2->SetMIDIOutputPorts(this, 1, 0);
    e2->SetupFromDevice(kFWAudiophileChannels.out, kFWAudiophileChannels.in, 1, false, false, false);   // [+0x2e0]=6 [+0x2e4]=4 [+0x2e8]=1
    // AP-3.2: the reference installs the procs here once ([+0x310] input + [+0x318] = this, [+0x300] output, [+0x308] peaks); the device's
    // ResetRearrangementProcs is the empty base body (@0x1c8c), so they stay across format changes.
    e2->SetInputConvertProc(FWConvertLLRR4SkipMIDI1, this);                    // _ConvertLLRR4SkipMIDI1
    e2->SetRearrangeProc(FWRearrangeLLRR6);                                     // _RearrangeSampleFrameLLRR6
    e2->SetPeakProc(FWSamplePeaks6);                                            // _SamplePeaks6
}

// The main engine ([+0xe8]) is paused around the command (vt[0xa50] pauseAudioEngine / vt[0xa58] resumeAudioEngine; nested inside
// SetClockSource's own pause, as in the reference). Gate bit 30 clear -> 0xE00002C2; clk & 3 == 1 -> AVCSetupSyncStream(4, 1) = source FF 82
// (S/PDIF in); clk & 3 == 0 -> AVCSetupSyncStream(0, 1) = source 60 01 (internal); 2 or 3 -> 0xE00002C2. `blank` is not read.
IOReturn com_openaudio_FWAudiophileDevice::SetClockSourceInternal(UInt32 clk, bool)
{
    com_openaudio_FWBaseEngine *e = fMainEngine;
    if (e) e->pauseAudioEngine();
    IOReturn r;
    if (clk <= 0x3fffffff) r = (IOReturn)0xE00002C2;
    else if ((clk & 3) == 1) r = AVCSetupSyncStream(4, 1);
    else if ((clk & 3) == 0) r = AVCSetupSyncStream(0, 1);
    else r = (IOReturn)0xE00002C2;
    if (e) e->resumeAudioEngine();
    return r;
}

// ref ReadHSCIData @0xba92 (decomp FWAudiophileDevice__ReadHSCIData.c; asm 0xba92-0xbb99)
void com_openaudio_FWAudiophileDevice::ReadHSCIData(const UInt32 *packet)
{
    const UInt32 s = packet[kFWAudiophileStatusQuadlet];
    UpdatePeakMeters(packet, kFWAudiophileHwPeaks);                            // slots 0-13
    if (fMeters && fSwPeaks)
        for (UInt32 k = 0; k < 6; k++) fMeters[0xe + k] = fSwPeaks[0xe + k];    // SW returns 1-6 (host peaks, _SamplePeaks6)
    const FWAudiophileStatus st = FWAudiophileDecodeStatus(s);
    DetectExternalSyncChanges(st.locked, st.fdf);
    if (fRotary) {
        if (st.levelDelta) fRotary->UserRotatedHardwareKnob(st.levelDelta);
        if (st.headphoneDelta) fRotary->SetLevelsFromKnob(0x100000000000000ULL, "MAHeadphoneLevels", 0x38, &fHeadphone, st.headphoneDelta);
    }
    if (st.abPressed && fAB) fAB->MomentarySwitchPressed();
}

IOReturn com_openaudio_FWAudiophileDevice::SetHeadphoneSelector(UInt32 pair, UInt32 which)
{
    FunctionBlockMapping m = { 8, 0, (UInt8)pair, 0 };
    if (MapChannelAndFunctionBlock(m) != kIOReturnSuccess) return kIOReturnSuccess;   // ref returns void; nothing sent
    UInt8 frame[12];
    bzero(frame, sizeof(frame));
    frame[6] = 0x02; frame[7] = (UInt8)which; frame[8] = 0x01;                // bytes 9-11 stay 0
    AVCControlFunctionBlock(0x80, m.fb, frame, sizeof(frame));
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWAudiophileDevice::InitializeUserSettableProperties()
{
    IOReturn r = com_openaudio_FWRev2AudioDevice::InitializeUserSettableProperties();
    if (r != kIOReturnSuccess) return r;
    const IOReturn kFail = (IOReturn)0xE00002C2;
    fLevels = OSTypeAlloc(com_openaudio_FWSettingsLevels);
    fAux    = OSTypeAlloc(com_openaudio_FWSettingsAuxSend);
    fRotary = OSTypeAlloc(com_openaudio_FWSettingsRotaryControl);
    fAB     = OSTypeAlloc(com_openaudio_FWSettingsABSwitch);
    if (!fLevels || !fAux || !fRotary || !fAB) return kFail;
    if ((r = fRotary->Init(this, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut)) != kIOReturnSuccess) return r;
    if ((r = fAux->Init(this, &fAuxSend, &fAuxOut, fOutput.Count() >> 1)) != kIOReturnSuccess) return r;
    if ((r = fLevels->Init(this, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut)) != kIOReturnSuccess) return r;
    if ((r = fAB->Init(this)) != kIOReturnSuccess) return r;
    typedef com_openaudio_FWAVCAudioDevice V;
    fAB->SetLEDFunctor(FWBind1<V, &V::AVCControlSetLEDStatus>(this));           // ref [+0x2e8] tag 0xbc9 -> vt[0xbc8]
    fAB->SetHeadphoneFunctor(FWBind2<V, &V::SetHeadphoneSelector>(this));       // ref [+0x2f8]
    fAB->UpdateLEDs();
    return kIOReturnSuccess;
}

void com_openaudio_FWAudiophileDevice::ResendCurrentMixerSettings()
{
    com_openaudio_FWRev2AudioDevice::ResendCurrentMixerSettings();
    if (!fLevels) return;
    fAB->SendToDevice();
    fRotary->SendToDevice();
    fAux->SendToDevice();
    fLevels->SendToDevice();
}

void com_openaudio_FWAudiophileDevice::ResetToFactorySettings()
{
    com_openaudio_FWRev2AudioDevice::ResetToFactorySettings();
    if (!fLevels) return;
    fAB->ResetToFactorySettings();
    fRotary->ResetToFactorySettings();
    fAux->ResetToFactorySettings();
    fLevels->ResetToFactorySettings();
}

