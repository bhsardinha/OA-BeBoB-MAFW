// FW1814Device module (models/FW1814) -- see FW1814Device.h.
#include "FW1814Device.h"
#include "FW1814Settings.h"
#include "FW1814MixerRegs.h"
#include "FW1814Peaks.h"
#include "FWBaseEngine.h"
#include "FWEngine2.h"
#include "FWSamples.h"
#include "FWMetaNub.h"
#include "FWAvc.h"
#include "FWHsci.h"
#include "FWPropertyDict.h"
#include "Trace.h"
#include <libkern/c++/OSString.h>

#define super com_openaudio_FWRev1AudioDevice
OSDefineMetaClassAndAbstractStructors(com_m_audio_FW1814Device, com_openaudio_FWRev1AudioDevice)
#undef super
#define super com_m_audio_FW1814Device
OSDefineMetaClassAndStructors(com_openaudio_FW1814Device, com_m_audio_FW1814Device)
#undef super

void com_openaudio_FW1814Device::free()
{
    if (fLevels) { fLevels->release(); fLevels = 0; }
    if (fAux)    { fAux->release();    fAux = 0; }
    if (fRotary) { fRotary->release(); fRotary = 0; }
    if (fAB)     { fAB->release();     fAB = 0; }
    com_openaudio_FWRev1AudioDevice::free();
}

// ref FW1814Device::AllocateAudioEngine @0xd78a
com_openaudio_FWBaseEngine *com_openaudio_FW1814Device::AllocateAudioEngine()
{
    return OSTypeAlloc(com_openaudio_FWEngine2);
}

// ref FW1814Device::SetupAudioEngine @0xed34: MIDI ports 1/1 (M-H), flags, m = 1, output channels 6 or 12 (mode bit 4),
// input channels 10 or 16 (mode bit 3); then vt[0xaf8] ResetRearrangementProcs (the sample converters, M-D2).
void com_openaudio_FW1814Device::SetupAudioEngine(com_openaudio_FWBaseEngine *engine)
{
    com_openaudio_FWEngine2 *e2 = OSDynamicCast(com_openaudio_FWEngine2, engine);
    if (!e2) return;
    if (fMidiDonor) {                                                              // M-S: keep the ports (and their shared rings) of the replaced engine
        e2->AdoptMIDIPortsFrom(fMidiDonor);
    } else {
        e2->SetMIDIInputPorts(this, 1, 0);                                         // ref SetupAudioEngine @0xed34: SetMIDIInputPorts(dev, 1, NULL)
        e2->SetMIDIOutputPorts(this, 1, 0);
    }
    e2->SetupFromDevice((fMode & 0x10) ? 12 : 6, (fMode & 0x08) ? 16 : 10, 1, true, true, true);   // ref [+0x2d5] [+0x2d6] [+0x348] = 1
    ResetRearrangementProcs(engine);                                         // ref: vt[0xaf8] at the end of SetupAudioEngine
}

static void FW1814InputDelayThunk(UInt32 count, const UInt32 *src, SInt32 *dst, void *arg)
{
    static_cast<com_openaudio_FW1814Device *>(arg)->WorkaroundFor1814InputDelay(count, src, dst);
}

// ref FW1814Device::ResetRearrangementProcs @0xcad4 (decomp FW1814Device__ResetRearrangementProcs.c): output proc by output channel
// count (4/6/8/10/12/14, otherwise none), input proc by input channel count (2 -> one stereo pair, otherwise the 1814 workaround).
void com_openaudio_FW1814Device::ResetRearrangementProcs(com_openaudio_FWBaseEngine *engine)
{
    if (!engine) return;
    engine->SetRearrangeProc(FWRearrangeProcForChannels(engine->outChannels()));
    // Clone fix (user hardware report at 192 kHz, 2026-10-17): the reference installs the output peak proc only in SwitchDigitalSignals, so after a
    // rate change (e.g. to 4 channels at 192 kHz) it kept the old 6/12-channel map and read the 4-channel scratch buffer with the wrong stride:
    // garbage software peaks. The peak map follows the channel count whenever the formats are applied.
    engine->SetPeakProc(FW1814PeakProcForChannels(engine->outChannels()));
    if (engine->inChannels() == 2) engine->SetInputConvertProc(FWConvertInputOneStereoPairSkipMIDI, (void *)2);
    else                           engine->SetInputConvertProc(FW1814InputDelayThunk, this);
}

// ref FW1814Device::WorkaroundFor1814InputDelay @0xcba4. Per frame (source stride channels+1, the extra quadlet is MIDI):
//   out[0]=in[0] out[1]=in[4]; out[2..7] = in[1,5,2,6,3,7] of 16 frames ago (ring of raw quadlets, zero-initialised);
//   out[8..9]=in[8..9]; if channels > 10: out[10..11]=in[10..11]; if channels > 12: out[12..15]=in[12..15].
// Why the 1814 needs this is not documented in the reference (guess -- needs verification: converter group delay of the analog inputs).
void com_openaudio_FW1814Device::WorkaroundFor1814InputDelay(UInt32 count, const UInt32 *src, SInt32 *dst)
{
    com_openaudio_FWBaseEngine *e = mainEngine();
    const UInt32 ch = e ? e->inChannels() : 0;
    if (ch < 10 || count == 0) return;
    for (UInt32 left = count; left >= ch; left -= ch) {
        UInt32 *h = &fDelayHist[((++fDelayCounter) & 0xf) * 6];
        const UInt32 o0 = h[0], o1 = h[1], o2 = h[2], o3 = h[3], o4 = h[4], o5 = h[5];
        h[0] = src[1]; h[1] = src[2]; h[2] = src[3]; h[3] = src[5]; h[4] = src[6]; h[5] = src[7];
        dst[0] = FWUnpackAM824(src[0]);
        dst[1] = FWUnpackAM824(src[4]);
        dst[2] = FWUnpackAM824(o0);
        dst[3] = FWUnpackAM824(o3);
        dst[4] = FWUnpackAM824(o1);
        dst[5] = FWUnpackAM824(o4);
        dst[6] = FWUnpackAM824(o2);
        dst[7] = FWUnpackAM824(o5);
        dst[8] = FWUnpackAM824(src[8]);
        dst[9] = FWUnpackAM824(src[9]);
        if (ch > 10) { dst[10] = FWUnpackAM824(src[10]); dst[11] = FWUnpackAM824(src[11]); }
        if (ch > 12) { for (UInt32 i = 12; i < 16; i++) dst[i] = FWUnpackAM824(src[i]); }
        src += ch + 1;
        dst += ch;
    }
}

// ref FW1814Device::InitializeUserSettableProperties @0xe6ca: the AVC-level properties, then the mixer: settings objects in the reference order
// (Rotary, AuxSend, Levels), the batch writers, the A/B switch and its functors
IOReturn com_openaudio_FW1814Device::InitializeUserSettableProperties()
{
    IOReturn r = com_openaudio_FWAVCAudioDevice::InitializeUserSettableProperties();
    if (r != kIOReturnSuccess) return r;
    fStereoPairProp = OSDynamicCast(com_openaudio_FWNumber32Prop, fPropDict->CopyPropertyObject("MAEnableStereoPairEngines"));
    const IOReturn kFail = (IOReturn)0xE00002C2;
    bzero(fPan, sizeof(fPan)); fRouting = fHpSel = fAuxMon = 0;
    fLevels = OSTypeAlloc(com_openaudio_FWSettings1814Levels);
    fAux    = OSTypeAlloc(com_openaudio_FWSettings1814AuxSend);
    fRotary = OSTypeAlloc(com_openaudio_FWSettingsRotaryControl);
    fAB     = OSTypeAlloc(com_openaudio_FWSettingsABSwitch);
    if (!fLevels || !fAux || !fRotary || !fAB) return kFail;

    if (fRotary->Init(this, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut) != kIOReturnSuccess) return kFail;
    if (fAux->Init(this, &fAuxSend, &fAuxOut, (fOutput.Count() >> 1) + 1) != kIOReturnSuccess) return kFail;
    if (fLevels->Init(this, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut) != kIOReturnSuccess) return kFail;

    typedef com_openaudio_FW1814Device D;
    const FWCallBatch plain = FWBindBatch<D, &D::CustomMixerWriteBatch>(this);
    fOutput.SetupBatch(plain, kFWMixRegOutput);
    fInput.SetupBatch(plain, kFWMixRegInput);
    fSWReturn.SetupBatch(plain, kFWMixRegSWReturn);
    fHeadphone.SetupBatch(plain, kFWMixRegHeadphone);
    fAuxSend.SetupBatch(FWBindBatch<D, &D::CustomMixerWriteWithRotatedAuxChannels>(this), kFWMixRegAuxSend);   // ref +0x1838/+0x183c/+0x1830
    fAuxOut.SetupBatch(plain, kFWMixRegAuxOutput);

    if (fAB->Init(this) != kIOReturnSuccess) return kFail;
    typedef com_openaudio_FWAVCAudioDevice V;
    fAB->SetLEDFunctor(FWBind1<V, &V::AVCControlSetLEDStatus>(this));
    fAB->SetHeadphoneFunctor(FWBind2<V, &V::SetHeadphoneSelector>(this));
    fAB->UpdateLEDs();
    return kIOReturnSuccess;
}

void com_openaudio_FW1814Device::ResendCurrentMixerSettings()                                 // ref @0xd014
{
    com_openaudio_FWAVCAudioDevice::ResendCurrentMixerSettings();
    if (!fLevels) return;
    fAB->SendToDevice();
    // F4 (clone addition that wrote the headphone selectors here) REVERTED 2026-10-16: the reference leaves Reg 0x98 unwritten until a monitor
    // source is chosen in the panel, so the phones stay silent at start-up. The phone levels start at 0 dB (full scale) and are dangerously loud;
    // the user sets them in the panel first. Do not re-add.
    fRotary->SendToDevice();
    fAux->SendToDevice();
    fLevels->SendToDevice();
}

void com_openaudio_FW1814Device::ResetToFactorySettings()
{
    com_openaudio_FWRev1AudioDevice::ResetToFactorySettings();                  // ref @0xd190 calls the parent first (fMode = 0x40000000)
    if (fLevels) {                                                               // ref @0xd190: ABSwitch, Rotary, AuxSend, Levels
        fAB->ResetToFactorySettings();
        fRotary->ResetToFactorySettings();
        fAux->ResetToFactorySettings();
        fLevels->ResetToFactorySettings();
    }
    fMode = 0x40000003;
}

// ref FW1814Device::SetBlankSlateClockSource @0xe4f0: re-assert the clock source (keeping the digital-format bits) and let the device settle 2.5 s
IOReturn com_openaudio_FW1814Device::SetBlankSlateClockSource()
{
    if (!fMainEngine) return kIOReturnSuccess;
    IOReturn r = SetClockSourceInternal((fMode & 0x1c) + 0x40000003, true);
    IOSleep(0x9c4);
    return r;
}

// M-S: OFF by default (user decision 2026-10-17, after reAMP froze when the engine was re-published during playback). The reference - and every
// official driver - pauses the stream, changes the formats in place and resumes it, so the clients' I/O continues after a short interruption.
// A re-publish removes the device from CoreAudio and clients that do not re-attach never get their stream back. The code stays (verified: the AMS
// sidebar label follows, capture 83) for a build that wants the label more than live clients; the cost of "off" is a stale sidebar label in the
// 10.13 Audio MIDI Setup after an optical switch (the HAL counts and the Audio Devices pane are right).
static const bool kRepublishAtOpticalSwitch = false;

// ref FW1814Device::SetClockSourceInternal @0xe25c. HSCI frame (assembly 0xe34c..0xe425, NOT the decompile): 00 FF 00 04 00 04 [sel] [clk>>3&1]
// [clk>>4&1] [clk>>5&1] + six zero bytes; sel: clk&3 = 1 -> 1, 2 -> 2, 3 -> 0, 0 -> 3. The reference pauses the engine around the whole
// operation and brackets it with begin/complete (error: cancel) configuration change.
IOReturn com_openaudio_FW1814Device::SetClockSourceInternal(UInt32 clk, bool blank)
{
    com_openaudio_FWBaseEngine *e = fMainEngine;
    com_openaudio_FWEngine2 *e2 = OSDynamicCast(com_openaudio_FWEngine2, e);   // bench diagnostics (integrated run 2026-10-16)
    if (e2) e2->LogStreamState("SetClockSource before pause");
    if (e) e->pauseAudioEngine();
    if (e2) e2->LogStreamState("SetClockSource after pause");
    IOReturn r = 0;
    if (!fClockBusy) {
        e->BeginConfigChange();
        fClockBusy = true;
        if (clk < 0x40000000) {
            r = (IOReturn)0xE00002C2;
        } else {
            UInt8 frame[16]; bzero(frame, sizeof(frame));
            frame[0] = 0x00; frame[1] = 0xFF; frame[2] = 0x00; frame[3] = 0x04; frame[4] = 0x00; frame[5] = 0x04;
            switch (clk & 3) { case 1: frame[6] = 1; break; case 2: frame[6] = 2; break; case 3: frame[6] = 0; break; default: frame[6] = 3; break; }
            frame[7] = (clk >> 3) & 1; frame[8] = (clk >> 4) & 1; frame[9] = (clk >> 5) & 1;
            UInt8 resp[16]; UInt32 rl = 16;
            if (fAvcUnit) fAvcUnit->AVCCommandWithErrorResponse(frame, 16, resp, &rl);     // result ignored by the reference
            IOSleep(300);
            fMode = clk;
            r = com_openaudio_FWAVCAudioDevice::SetClockSourceInternal(clk, blank);         // ref vt[0xb70] on FWRev1AudioDevice
            if (r == kIOReturnSuccess) {
                IOSleep(300);
                if (!blank) {
                    fMode = clk;
                    SwitchDigitalSignals(clk);                                              // ref vt[0xbe0]
                    if (fLevels) fLevels->SendToDevice();
                }
                fClockBusy = false;
                e->CompleteConfigChange();
                if (e) e->resumeAudioEngine();
                if (fRepublishPending) {
                    fRepublishPending = false;
                    if (kRepublishAtOpticalSwitch) { fRepublishDeferred = true; RepublishIfIdle(); }
                }
                return r;
            }
        }
        fClockBusy = false;
        e->CancelConfigChange();
    }
    if (e) e->resumeAudioEngine();
    return r;
}

// ref FW1814Device::SwitchDigitalSignals @0xe5dc: the digital format bits changed -> new DeviceChannels row, peak proc, converters, names
void com_openaudio_FW1814Device::SwitchDigitalSignals(UInt32)
{
    com_openaudio_FWBaseEngine *e = fMainEngine;
    if (!e) return;
    FWDeviceChannels row;
    GetDeviceChannels(row);
    if (fPubIn == 0) { fPubIn = e->inChannels(); fPubOut = e->outChannels(); }               // M-S (clone): counts of the engine the HAL was given
    if (row.in != fPubIn || row.out != fPubOut) fRepublishPending = true;                      // the HAL must see the new counts
    e->RemoveAllSubEngines();                                                    // ref engine vt[0xc48]
    e->SetIOFormat(row, FetchIndexIntoSampleRateArray(row.outRates, e->rateHz()));
    e->SetPeakProc(FW1814PeakProcForChannels(e->outChannels()));
    ClearPeakMeters();
    ResetRearrangementProcs(e);
    SetChannelNamesForMainEngine(e);
    if (fStereoPairProp && fStereoPairProp->value()) CreateStereoPairSubEngines();   // ref: [+0x184] = MAEnableStereoPairEngines
}

// M-S (clone deviation, see the declaration). Order: remember the live state; quiesce the old engine completely; take its controls and MIDI
// ports; create + activate the new main engine from the already updated mode (createAudioEngine = deactivateAllAudioEngines, init,
// SetupAudioEngine, names, activate); restore the sample rate through the normal format path; re-establish the CoreAudio controls and the pair
// engines; register the engine with the HAL (ReallyRegisterService) - coreaudiod then sees the device leave and come back with the new counts;
// finally restart MIDI if it was running (the CoreMIDI plug-in does not know the engine changed).
void com_openaudio_FW1814Device::RepublishAudioEngine()
{
    com_openaudio_FWBaseEngine *old = fMainEngine;
    if (!old || !fReady) return;
    old->retain();
    fRepublishDeferred = false;
    const UInt32 rate = old->rateHz();
    const bool extSync = old->externalSync();
    const bool midiWas = old->midiRunning();
    IOLog(OA_TAG "M-S: re-publishing the audio engine (%u Hz, mode 0x%x)\n", (unsigned)rate, (unsigned)fMode);
    PauseReceptionOfControlPackets();
    old->Quiesce();
    old->RemoveAllSubEngines();
    if (fLevels) fLevels->DetachAudioControls(old);
    fMidiDonor = old;
    const bool ok = createAudioEngine();
    fMidiDonor = 0;
    if (!ok || !fMainEngine) {
        IOLog(OA_TAG "M-S: re-publish FAILED, the device has no audio engine until the next attach\n");
        old->release();
        ResumeReceptionOfControlPackets();
        return;
    }
    com_openaudio_FWBaseEngine *e = fMainEngine;
    fPubIn = e->inChannels(); fPubOut = e->outChannels();
    RequestFormatUpdateForSampleRate(rate);                                      // ref vt[0xa78]: also applies SMUX / ADAT counts for this rate
    e->CompletePendingStateChanges();
    if (extSync) e->SetExternalSyncEnabled(true);
    e->SetPeakProc(FW1814PeakProcForChannels(e->outChannels()));
    ClearPeakMeters();
    ResetRearrangementProcs(e);
    SetChannelNamesForMainEngine(e);
    if (fLevels) fLevels->EstablishAudioControls();
    if (fStereoPairProp && fStereoPairProp->value()) CreateStereoPairSubEngines();
    e->ReallyRegisterService();                                                  // ref engine vt[0xc70]
    if (midiWas) e->StartMIDI();
    old->release();
    ResumeReceptionOfControlPackets();
}

#include "FWBusReset.h"

static IOReturn RepublishAction(OSObject *owner, void *, void *, void *, void *)
{
    static_cast<com_openaudio_FW1814Device *>(owner)->RepublishIfIdle();
    return kIOReturnSuccess;
}

static void DeferredRepublishThread(void *arg)
{
    com_openaudio_FW1814Device *d = static_cast<com_openaudio_FW1814Device *>(arg);
    d->DeferredRepublish();
    d->release();
}

void com_openaudio_FW1814Device::DeferredRepublish()
{
    IOSleep(1500);                                                               // the engine stop that woke us is still unwinding
    if (IOWorkLoop *wl = getWorkLoop()) wl->runAction(&RepublishAction, this);
    fRepublishThread = false;
}

// Re-publish now if the HAL's counts are out of date and nothing is streaming; otherwise stay deferred (audioEngineStopped retries).
void com_openaudio_FW1814Device::RepublishIfIdle()
{
    com_openaudio_FWBaseEngine *e = fMainEngine;
    if (!fRepublishDeferred || fClockBusy || !e || !fReady) return;
    if (e->inChannels() == fPubIn && e->outChannels() == fPubOut) { fRepublishDeferred = false; return; }
    if (e->getState() != kIOAudioEngineStopped || e->ActiveUserClients() != 0) {
        IOLog(OA_TAG "M-S: counts changed during playback; the HAL label is refreshed when the engine stops\n");
        return;
    }
    RepublishAudioEngine();
}

void com_openaudio_FW1814Device::audioEngineStopped()
{
    com_openaudio_FWDiscreteDevice::audioEngineStopped();
    if (fRepublishDeferred && !fRepublishThread) {
        fRepublishThread = true;
        retain();
        if (!FWRunOnNewThread(&DeferredRepublishThread, this)) { fRepublishThread = false; release(); }
    }
}

// ref FW1814Device::ReconnectP2P @0xe1ea: drop and re-establish the connections after a bus reset, then re-assert the clock source
void com_openaudio_FW1814Device::ReconnectP2P()
{
    if (!fMainEngine) return;
    fMainEngine->DisconnectAllPlugs();
    fMainEngine->ConnectAllPlugs();
    IOSleep(0x44c);
    if (!fMainEngine->externalSync()) SetClockSourceInternal(fMode, true);
}

static void FW1814BusResetThread(void *arg)
{
    com_openaudio_FW1814Device *d = static_cast<com_openaudio_FW1814Device *>(arg);
    d->ReconnectP2P();
    d->release();
}

// ref FW1814Device::BusReset @0xe53a: the member at vtable slot 0xbf0 (= ReconnectP2P) on a new thread
void com_openaudio_FW1814Device::BusReset()
{
    retain();                                                                 // the thread uses `this`
    if (!FWRunOnNewThread(&FW1814BusResetThread, this)) release();
}

// ref ReadHSCIData @0xd952: word 20 carries the clock lock (bit 24) and the FDF (bits 16..18), then the packet is interpreted
void com_openaudio_FW1814Device::ReadHSCIData(const UInt32 *packet)
{
    static UInt32 sLogged = 0;                                                // bench diagnostics (integrated run 2026-10-16)
    if (sLogged < 3 || (sLogged < 40 && (sLogged % 10) == 0)) {
        OA_LOG("HSCI packet #%u: %08x %08x %08x %08x %08x %08x | w20 %08x", (unsigned)sLogged, (unsigned)packet[0], (unsigned)packet[1],
               (unsigned)packet[2], (unsigned)packet[3], (unsigned)packet[4], (unsigned)packet[5], (unsigned)packet[0x14]);
    }
    sLogged++;
    DetectExternalSyncChanges((packet[0x14] >> 24) & 1, (packet[0x14] >> 16) & 7);
    ReceiveControlPacket(reinterpret_cast<const UInt8 *>(packet));
}

// ref ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*) @0xe9ea (decomp FW1814Device__ReceiveControlPacket_0000e9ea.c).
// Byte order: the reference reads the quadlets as delivered by IOFWReadQuadCommand and byte-swaps the 16-bit meter halves, so the buffer
// is treated here exactly the same way (VERIFY at the integrated run that the 10.13 IOFWReadQuadCommand delivers the same memory image).
void com_openaudio_FW1814Device::ReceiveControlPacket(const UInt8 *pkt)
{
    UInt32 *m = fMeters, *sw = fSwPeaks;
    if (!m || !sw) return;
    const UInt32 w0 = *reinterpret_cast<const UInt32 *>(pkt);
    if (!fMainEngine || fMainEngine->fdfIndex() < 5) {                        // rates below 176.4 kHz
        for (UInt32 i = 0; i < 19; i++) {
            UInt16 a = *reinterpret_cast<const UInt16 *>(pkt + 4 + i * 4), b = *reinterpret_cast<const UInt16 *>(pkt + 6 + i * 4);
            m[2 * i]     = (UInt16)((a << 8) | (a >> 8));
            m[2 * i + 1] = (UInt16)((b << 8) | (b >> 8));
        }
        for (UInt32 k = 0; k < 14; k++) m[38 + k] = sw[38 + k] & 0xffff;
        if ((((UInt8)(fAuxMon >> 24)) & 4) == 0)           // ref device byte [+0x1317]: aux-monitor bus 3 select (bit 2)
            { m[22] = sw[42]; m[23] = sw[43]; }
        else                                                              { m[22] = m[36]; m[23] = m[37]; }
        for (UInt32 k = 0; k < 8; k++) m[24 + k] = sw[44 + k];
    } else {
        for (UInt32 k = 0; k < 4; k++) { const UInt32 v = sw[38 + k] & 0xffff; m[18 + k] = v; m[38 + k] = v; }
    }
    SInt32 delta = 0;
    if (fRotary) {
        const UInt32 r0 = (w0 >> 24) & 3, r1 = (w0 >> 16) & 3, r2 = (w0 >> 8) & 3;
        delta = (r0 == 1) ? 1 : (r0 == 2) ? -1 : 0;
        if (delta) fRotary->UserRotatedHardwareKnob(delta);
        for (int which = 0; which < 2; which++) {
            const UInt32 r = which == 0 ? r1 : r2;
            delta = (r == 1) ? 1 : (r == 2) ? -1 : 0;
            if (!delta) continue;
            OSArray *arr = 0;
            fRotary->SetPairLevelsFromRotary("MAHeadphoneLevels", which == 0 ? 1 : 0, arr, delta);
            if (arr) { fPropDict->SetPropertyAndNotifyUserClients("MAHeadphoneLevels", arr); arr->release(); }
        }
        if ((w0 & 3) != 0) fAB->MomentarySwitchPressed();
    }
}

// ref FW1814Device::MapChannelAndFunctionBlock @0xde84 (decomp FW1814Device__MapChannelAndFunctionBlock.c). Kinds 0, 1, 2 and 6 select from the
// SelectMap1814 tables (C_114..C_117 @0x2f928..0x2f988) through SelectFunctionBlock, not decoded (Q-ME §8): on the FW1814 they are reached only
// by the per-channel level setters and the generic SetInputPan, which the batch writers and FWSettings1814Levels replace. Unsupported here.
IOReturn com_openaudio_FW1814Device::MapChannelAndFunctionBlock(FunctionBlockMapping &m)
{
    switch (m.kind) {
    case 3: m.fb = 4; return kIOReturnSuccess;
    case 4: m.fb = 4; m.ch = (m.extra & 4) ? 1 : 0; return kIOReturnSuccess;    // clock selector, value = clock word bit 2
    case 5: m.fb = m.ch; return kIOReturnSuccess;
    case 7: m.fb = 9; return kIOReturnSuccess;                                  // aux output feature block
    case 8: m.fb = (m.ch == 0) ? 5 : 6; return kIOReturnSuccess;
    case 9: return (IOReturn)0xE00002C2;
    case 0: case 1: case 2: case 6: return (IOReturn)0xE00002C7;                // SelectMap1814 tables, see above
    default: return kIOReturnSuccess;                                           // ref: fb / ch unchanged, 0
    }
}

// ref CustomMixerWriteWithRotatedAuxChannels @0xe982: {in[9], in[10], in[0..8]} -- the last 8 bytes of the 44-byte block go first
IOReturn com_openaudio_FW1814Device::CustomMixerWriteWithRotatedAuxChannels(UInt32 off, UInt32 quads, void *data)
{
    UInt8 rot[44];
    const UInt8 *in = static_cast<const UInt8 *>(data);
    bcopy(in + 0x24, rot, 8);
    bcopy(in, rot + 8, 0x24);
    return CustomMixerWriteQuads(off, quads, rot);
}

// ref FW1814Device::SetInputPan @0xdd3e (decomp FW1814Device__SetInputPan.c). Channels 1..18 -> pan word; each register quadlet carries the
// words of channels (2k+1, 2k+2) in memory order.
IOReturn com_openaudio_FW1814Device::SetInputPan(UInt32 ch, UInt32 pan)
{
    const UInt32 idx = ch - 1;
    if (idx >= 18) return (IOReturn)0xE00002C2;
    fPan[idx] = (UInt16)((((UInt32)(0 - pan) >> 1) & 0xff) + (UInt32)((SInt32)(SInt16)pan * -0x8000));
    const UInt32 base = idx & ~1u;                                              // first word of the pair
    return CustomMixerWriteQuads(kFWMixRegPan + base * 2, 1, &fPan[base]);
}

static inline UInt32 MixBit(UInt32 n) { return OSSwapInt32(1u << (n & 0x1f)); }

static inline UInt32 MixPut(UInt32 shadow, UInt32 bit, bool on) { return on ? (shadow | bit) : (shadow & ~bit); }

// ref FW1814Device::SetInputRouting @0xdbea (decomp FW1814Device__SetInputRouting.c): route bit 0 / bit 1 of the value per channel
IOReturn com_openaudio_FW1814Device::SetInputRouting(UInt32 ch, UInt32 route)
{
    if (ch == 0 || ch > (fInput.Count() >> 1)) return (IOReturn)0xE00002C2;
    if (ch < 5) {
        fRouting = MixPut(fRouting, MixBit(ch - 1), route & 1);
        fRouting = MixPut(fRouting, MixBit(ch + 3), route & 2);
    } else if (ch == 5) {
        fRouting = MixPut(fRouting, 0x100, route & 1);                          // raw host bits 8 / 9 (= wire bits 16 / 17)
        fRouting = MixPut(fRouting, 0x200, route & 2);
    } else if (ch < 10) {
        fRouting = MixPut(fRouting, MixBit(ch + 2), route & 1);
        fRouting = MixPut(fRouting, MixBit(ch + 6), route & 2);
    }
    return CustomMixerWriteQuads(kFWMixRegInputRouting, 1, &fRouting);
}

// ref FW1814Device::SetAuxMonitor @0xcfc4
IOReturn com_openaudio_FW1814Device::SetAuxMonitor(UInt32 bus, UInt32 on)
{
    fAuxMon = MixPut(fAuxMon, MixBit(bus - 1), on != 0);
    return CustomMixerWriteQuads(kFWMixRegAuxMonitor, 1, &fAuxMon);
}

// ref FW1814Device::SetHeadphoneSelector @0xd1f8 (decomp FW1814Device__SetHeadphoneSelector.c, asm 0xd272..0xd296): clear the pair's old selector
// bits (the second assignment of the reference replaces the first, it does not accumulate), then set the bit for `which` (0..2)
IOReturn com_openaudio_FW1814Device::SetHeadphoneSelector(UInt32 pair, UInt32 which)
{
    UInt32 v = fHpSel & 0xfffff8ffu;
    if (pair == 0) v = fHpSel & 0xf8ffffffu;
    fHpSel = v;
    if (which <= 2) {
        const UInt32 a = (pair == 0) ? (1u << (0x18 + which)) : 0;
        const UInt32 b = (pair != 0) ? (1u << (8 + which)) : 0;
        fHpSel = (a + b) | v;
    }
    return CustomMixerWriteQuads(kFWMixRegHeadphoneSel, 1, &fHpSel);
}

// ref FW1814Device::AVCControlSetLEDStatus @0xdaaa: 8-byte vendor CONTROL 00 FF 00 03 00 01 [state != 1] 00 (asm 0xdaf0..0xdb72)
IOReturn com_openaudio_FW1814Device::AVCControlSetLEDStatus(UInt32 state)
{
    if (!fAvcUnit) return (IOReturn)0xE00002BC;
    const UInt8 cmd[8] = { 0x00, 0xFF, 0x00, 0x03, 0x00, 0x01, (UInt8)(state != 1), 0x00 };
    UInt8 resp[8]; UInt32 rl = 8;
    return fAvcUnit->AVCCommandWithErrorResponse(cmd, 8, resp, &rl);
}
