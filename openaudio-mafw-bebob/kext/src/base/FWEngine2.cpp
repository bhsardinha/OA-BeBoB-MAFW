#include "FWEngine2.h"
#include "Trace.h"
#include "FWBusReset.h"

#define super com_openaudio_FWBaseEngine
OSDefineMetaClassAndStructors(com_openaudio_FWEngine2, com_openaudio_FWBaseEngine)

static const IOReturn kErrBadArg   = (IOReturn)0xE00002C2;
static const IOReturn kErrNotReady = (IOReturn)0xE00002BC;
static const IOReturn kErrNoBandw  = (IOReturn)0xE00002BE;

// ref FW1814Device::SetupAudioEngine @0xed34 (counts from the device mode byte at the call site):
// [+0x2d5] = [+0x2d6] = [+0x348] = 1, m = 1, outputs 6/12 (mode bit 4), inputs 10/16 (mode bit 3); stream counts equal.
void com_openaudio_FWEngine2::SetupFromDevice(UInt32 outChannels, UInt32 inChannels, UInt32 midiChannels, bool flagA, bool rxRefreshAtHighRates, bool twoPhase)
{
    fFlagA = flagA;
    fRxRefreshAtHighRates = rxRefreshAtHighRates;
    fTwoPhaseConnect = twoPhase;
    fMidiChannels = midiChannels;
    fOutStreamChannels = fOutChannels = outChannels;
    fInStreamChannels  = fInChannels  = inChannels;
}

// ref ClipPCMOutput @0x11d72 (decomp FWEngine2__ClipPCMOutput.c; asm 11d72-11ed0). Float mix -> scaled ints in `scratch`, then per TX packet
// the frames that fall into it are rearranged into AM824 quadlets in the packet pool (the pool slot of a sample index is fixed by the
// 2048-sample group / packet index tables the TX callback maintains).
// The peak proc ([+0x308], set by SwitchDigitalSignals) fills the device's software peak array. With an output map ([+0x328], the FW410's
// 176.4/192 kHz row) each packet's frames go through RemapSampleFramesForOutput (map + copy list [+0x330]) instead of the rearrange proc
// (asm 0x11e41-0x11eb4, R136).
// The per-packet count below is the reference's arithmetic literally (asm 0x11e2d..0x11e3e): framesLeft, or framesLeft+done-num when the
// packet would run past the request. That equals the remaining frame count only for the aligned requests CoreAudio issues; it never
// exceeds framesLeft, so it cannot write past the packet.
void com_openaudio_FWEngine2::ClipPCMOutput(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num)
{
    FWFloat32ToInt24Scaled(mix, scratch, fOutStreamChannels * num);
    if (fPeakProc && fDevice && fDevice->softwarePeaks()) {
        SInt32 *peaks = reinterpret_cast<SInt32 *>(fDevice->softwarePeaks()) + fDevice->GetFirstSWReturnPeakMeterIndex();
        bzero(peaks, (size_t)fDevice->GetSWReturnPeakMeterCount() * 4);       // ref: ClipPCMOutput zeroes the region, then runs the proc
        fPeakProc(scratch, peaks, num);
    }
    if (!fTx || (!fRearrangeProc && !fChannelMap[1])) return;
    UInt32 done = 0;
    while (done < num) {
        UInt32 framesLeft = 0, *pkt = 0, frameInPkt = 0;
        UInt32 *dstFrame = fTx->pool().GetPacketFrameContainingSampleIndex(done + first, fSytInterval, framesLeft, &pkt, &frameInPkt);
        UInt32 n = framesLeft;
        if (framesLeft + done > num) n = framesLeft + done - num;
        if (n == 0) break;                                                   // cannot happen with the arithmetic above; keeps the loop finite
        if (dstFrame) {
            const SInt32 *s = scratch + (size_t)fOutStreamChannels * done;
            if (fChannelMap[1]) FWRemapSampleFramesForOutput(s, dstFrame, fOutChannels, fOutStreamChannels, n, fChannelMap[1], fChannelMap[2], fMidiChannels != 0);
            else fRearrangeProc(s, dstFrame, n);
        }
        done += n;
    }
}

// ref ClipAC3Output @0x11c44 (decomp FWEngine2__ClipAC3Output.c + FWEngine2__LocalRearrangeSampleFrame16Bit2.c). `mix` holds the client's
// 16-bit stereo IEC 61937 words (non-mixable 'cac3' stream). Each frame becomes one packet data block: every audio quadlet an empty PCM
// sample (0x40), the S/PDIF pair (the model's IndexForAC3OutputChannels, wire slots: FW1814 4/5, Audiophile 2/5) the 16-bit words as 40 <hi> <lo> 00, the MIDI
// quadlets empty (0x80); the blocks are then copied into the TX packets exactly like the PCM clip. The reference returns an error for
// anything but a 2-channel non-mixable stream within 0x2000 frames; the caller (ClipOutputSamplesWithMixPtrAdjusted) ignores the value.
void com_openaudio_FWEngine2::ClipAC3Output(const float *mixF, SInt32 *scratch, UInt32 first, UInt32 num)
{
    const IOAudioStreamFormat *of = fOutStream ? fOutStream->getFormat() : 0;
    if (!of || fOutStreamBytes < (num + first) * 4 || of->fIsMixable || of->fNumChannels != 2 || num + first > 0x2000 || !mixF || !scratch || !fTx)
        return;
    const UInt16 *src = reinterpret_cast<const UInt16 *>(mixF);
    UInt32 left, right;
    fDevice->IndexForAC3OutputChannels(left, right);
    const UInt32 stride = fOutChannels + fMidiChannels;                      // quadlets per data block
    UInt32 *dst = reinterpret_cast<UInt32 *>(scratch);
    for (UInt32 f = 0; f < num; f++, dst += stride) {
        const UInt32 L = src[2 * f], R = src[2 * f + 1];
        if (fMidiChannels != stride) for (UInt32 k = 0; k < fOutChannels; k++) dst[k] = 0x40;
        dst[left]  = (((L & 0xff) << 16) | (L & 0xff00)) + 0x40;             // (CONCAT22(L,L) & 0xffff00) + 0x40
        dst[right] = (((R & 0xff) << 16) | (R & 0xff00)) + 0x40;
        for (UInt32 k = 0; k < fMidiChannels; k++) dst[fOutChannels + k] = 0x80;
    }
    UInt32 done = 0;
    while (done < num) {
        UInt32 framesLeft = 0, *pkt = 0, frameInPkt = 0;
        UInt32 *dstFrame = fTx->pool().GetPacketFrameContainingSampleIndex(done + first, fSytInterval, framesLeft, &pkt, &frameInPkt);
        UInt32 n = framesLeft;
        if (framesLeft + done > num) n = framesLeft + done - num;            // same literal arithmetic as ClipPCMOutput
        if (n == 0) break;
        if (dstFrame) bcopy(scratch + (size_t)done * stride, dstFrame, (size_t)n * stride * 4);
        done += n;
    }
}

// ref StartAC3Output @0x1185e (vt[0xd10]): MIDI cannot run next to the AC3 stream, then the AC3 activation
IOReturn com_openaudio_FWEngine2::StartAC3Output()
{
    if (midiRunning()) StopMIDI();
    return ActivateAC3Output();
}

// ref ActivateAC3Output @0x11fee: device-side state, then the TX program start (the RX program stays stopped, ActivatePCMInput refuses in AC3 mode)
IOReturn com_openaudio_FWEngine2::ActivateAC3Output()
{
    IOReturn r = SetAC3OutputState(true);
    if (r != kIOReturnSuccess) return r;
    return ActivatePCMOutput();
}

// ref DeactivateAC3Output @0x11fc6
void com_openaudio_FWEngine2::DeactivateAC3Output()
{
    DeactivatePCMOutput();
    SetAC3OutputState(false);
}

// ref SetAC3OutputState @0x11f28: only devices that WantsAC3MixerBypass() do anything (an AV/C selector on a mixer function block). Neither
// the FW1814 (vtable 0xab0 -> 0xc324) nor the Audiophile (base 0xa98e, Q-AP0 §7) does; a model that needs the bypass adds it in its module.
IOReturn com_openaudio_FWEngine2::SetAC3OutputState(bool on)
{
    (void)on;
    com_openaudio_FWAVCAudioDevice *d = OSDynamicCast(com_openaudio_FWAVCAudioDevice, fDevice);
    if (!d || !d->WantsAC3MixerBypass()) return kIOReturnSuccess;
    return kIOReturnUnsupported;                                             // not ported: no implemented model wants the bypass
}

// ref initHardware @0x12622: base, then the TX (0x620 B in the reference) and RX (0x518 B) DCL programs sharing the engine's
// FWDCLSharedSyncData; both primary; RX knows the TX program; both registered with the engine.
bool com_openaudio_FWEngine2::initHardware(IOService *provider)
{
    if (!super::initHardware(provider)) return false;
    fTx = OSTypeAlloc(com_openaudio_FWDCLOutputProgram);
    fRx = OSTypeAlloc(com_openaudio_FWDCLInputProgram);
    if (fTx && fRx && fNub && fTx->Initialize(fNub, &fShared) && fRx->Initialize(fNub, &fShared)) {
        fRx->SetTxProgram(fTx);
        fRx->SetPrimary(true);
        fTx->SetPrimary(true);
        com_openaudio_FWAVCAudioDevice *avc = OSDynamicCast(com_openaudio_FWAVCAudioDevice, fDevice);
        if (avc) { fTx->SetConnectionManager(avc->connectionManager()); fRx->SetConnectionManager(avc->connectionManager()); }
        if (AddDCLProgram(fTx) == kIOReturnSuccess && AddDCLProgram(fRx) == kIOReturnSuccess) return true;
    }
    if (fRx) { fRx->release(); fRx = 0; }
    if (fTx) { fTx->release(); fTx = 0; }
    return false;
}

void com_openaudio_FWEngine2::stop(IOService *provider)
{
    super::stop(provider);                                       // (ref repeats the base body verbatim)
    if (fRx) { fRx->release(); fRx = 0; }
    if (fTx) { fTx->release(); fTx = 0; }
}

// ref PerformSampleRateChangeCore @0x117e6 (decomp FWEngine2__PerformSampleRateChangeCore.c)
bool com_openaudio_FWEngine2::PerformSampleRateChangeCore(const IOAudioSampleRate *rate)
{
    if (!fConnected || fTwoPhaseConnect || (fExternalSync && !fExtSyncLocked)) {
        return super::PerformSampleRateChangeCore(rate);
    }
    DisconnectAllPlugs();                                       // vt[0xcc0]
    const bool r = super::PerformSampleRateChangeCore(rate);
    fPending |= kFWPendingReconnectPlugs;                       // ref [+0x345] |= 0x80
    return r;
}

// ref UpdateDCLProgramsWithNewPacketSizes @0x122d6: TX with (out channels, m) at the new rate, RX with (in channels, m) unless
// the rate is above 96 kHz and the device does not refresh RX at high rates (the FW1814 does: [+0x2d6] = 1).
bool com_openaudio_FWEngine2::UpdateDCLProgramsWithNewPacketSizes()
{
    LogStreamState("UpdateDCLProgramsWithNewPacketSizes");                 // bench diagnostics (integrated run 2026-10-16)
    fPending |= kFWPendingSettle;                               // ref [+0x347] |= 1
    IOReturn r = fTx->SetPacketParameters(fRate, fOutChannels, fMidiChannels);
    if (r == kIOReturnSuccess && (sampleRate.whole <= 96000 || fRxRefreshAtHighRates))
        r = fRx->SetPacketParameters(fRate, fInChannels, fMidiChannels);
    return r == kIOReturnSuccess;
}

// ref RecalculateBandwidth @0x1235a (decomp FWEngine2__RecalculateBandwidth.c)
IOReturn com_openaudio_FWEngine2::RecalculateBandwidth()
{
    IOReturn r = kIOReturnSuccess;
    if (!fTwoPhaseConnect) {
        if (!fExternalSync || fExtSyncLocked) {
            bool doConnect = true;
            if (!fConnected) {
                if (!(fPending & kFWPendingReconnectPlugs)) doConnect = false;
            } else {
                DisconnectAllPlugs();
            }
            if (doConnect) r = ConnectAllPlugs();
        }
    } else {
        // ref: kick INPUT then OUTPUT through the AV/C device (vt[0xb50]) with the engine's FDF index
        if (fDevice) {
            fDevice->AVCControlPlugSignalFormat(true, 0, (UInt8)fFdf);
            fDevice->AVCControlPlugSignalFormat(false, 0, (UInt8)fFdf);
        }
    }
    if (fDevice) fDevice->SetStaticPropertyAndNotifyUserClients("MABytesPerCycle", fDevice->GetBandwidthBytesAtSampleRate(fRate));
    return r;
}

// ref SetExternalSyncEnabled @0x11bc0: the TX program follows the receiver's SYTs when enabled.
IOReturn com_openaudio_FWEngine2::SetExternalSyncEnabled(bool on)
{
    IOReturn r = super::SetExternalSyncEnabled(on);
    if (r == kIOReturnSuccess && fTx) fTx->SetExternalSync(fExternalSync);
    return r;
}

// ---- PerformFormatChange @0x124cc (decomp FWEngine2__PerformFormatChange.c; AC3 fork documented in Q15c) -----------------
IOReturn com_openaudio_FWEngine2::PerformFormatChange(IOAudioStream *stream, const IOAudioStreamFormat *fmt, const IOAudioSampleRate *rate)
{
    (void)stream;
    if (fDevice) { char tr[120]; snprintf(tr, sizeof(tr), "PerformFormatChange: fmt %u ch rate %u (engine rate %u)", fmt ? (unsigned)fmt->fNumChannels : 0u, rate ? (unsigned)rate->whole : 0u, (unsigned)fRate); fDevice->DebugTrace(tr); }
    pauseAudioEngine();                                          // vt[0xa50]
    IOReturn result = kIOReturnSuccess;
    if (!fmt && !rate) { resumeAudioEngine(); return result; }

    const bool isAc3Tag = fmt && fmt->fSampleFormat == 0x63616333;               // 'cac3'
    const bool entering = isAc3Tag && !fAC3;
    const bool leaving  = fmt && !isAc3Tag && fAC3;
    const IOAudioSampleRate *committed = rate;
    IOAudioSampleRate ac3Rate = { 48000, 0 };
    if (!entering) {
        if (!fAC3) {
            const UInt32 want = rate ? rate->whole : fRate;
            fDevice->RequestFormatUpdateForSampleRate(want);                      // device vt[0xa78]
        }
        if (leaving) DisableAC3Format();
    } else {
        EnableAC3Format();
        committed = rate ? rate : &ac3Rate;
    }
    if (committed) {
        if (committed->whole != fRate || leaving) {
            IOAudioSampleRate cur = { fRate, 0 };
            PerformSampleRateChangeCore(committed ? committed : &cur);           // vt[0xce0]
        }
    } else if (leaving) {
        IOAudioSampleRate cur = { fRate, 0 };
        PerformSampleRateChangeCore(&cur);
    }
    result = CompletePendingStateChanges();
    resumeAudioEngine();                                                         // vt[0xa58]
    ScheduleVerifyStreamFormats();                                               // clone: see FWBaseEngine.h
    return result;
}

// ---- connect / start / stop -------------------------------------------------------------------------------------------

IOReturn com_openaudio_FWEngine2::ConnectAudioMIDIChannels()
{
    com_openaudio_FWAVCAudioDevice *avc = OSDynamicCast(com_openaudio_FWAVCAudioDevice, fDevice);
    if (!avc) return kErrNotReady;
    fStreamControl.Configure(avc, fTx, fRx, (UInt8)fFdf, fTwoPhaseConnect);
    return fStreamControl.ConnectAudioMIDIChannels();
}

// ref ConnectAllPlugs @0x12476
IOReturn com_openaudio_FWEngine2::ConnectAllPlugs()
{
    if (fConnected) return kErrNotReady;
    if (!FWStreamControl::SufficientBandwidthAvailable(fDevice->GetBandwidthBytesAtSampleRate(fRate))) return kErrNoBandw;
    IOReturn r = ConnectAudioMIDIChannels();
    fConnected = (r == kIOReturnSuccess);
    return r;
}

// ref DisconnectAllPlugs @0x11bfa: RX first, then TX.
void com_openaudio_FWEngine2::DisconnectAllPlugs()
{
    if (!fConnected) return;
    fConnected = false;
    if (fRx) fRx->Disconnect();
    if (fTx) fTx->Disconnect();
}

// ref StartAudio @0x12024: PCM -> StartPCMAudio, AC3 -> StartAC3Output; flag [+0x238] set on success.
IOReturn com_openaudio_FWEngine2::StartAudio()
{
    if (fAudioStarted) return (IOReturn)0xE00002C2;                                // ref -0x1ffffd3e
    IOReturn r;
    if (!fAC3) {
        r = StartPCMAudio();
    } else {
        fShared.lastBank = 0;                                                       // ref [+0x338] = 0
        r = StartAC3Output();
    }
    fAudioStarted = (r == kIOReturnSuccess);
    return r;
}

// ref StartPCMAudio @0x11b20: RX Start first (skipped above 96 kHz unless [+0x2d6]), then TX Start; roll back RX on a TX failure.
IOReturn com_openaudio_FWEngine2::StartPCMAudio()
{
    if (!fConnected) return kErrBadArg;
    IOReturn r = kIOReturnSuccess;
    if (!(sampleRate.whole > 96000 && !fRxRefreshAtHighRates)) r = ActivatePCMInput();
    if (r == kIOReturnSuccess) {
        r = ActivatePCMOutput();
        if (r != kIOReturnSuccess) DeactivatePCMInput();
    }
    return r;
}

IOReturn com_openaudio_FWEngine2::ActivatePCMInput()                     // ref @0x11a5e
{
    if (!fConnected || fAC3) return kErrNotReady;
    return fRx->Start();
}

IOReturn com_openaudio_FWEngine2::ActivatePCMOutput()                    // ref @0x11a8c
{
    if (!fConnected) return kErrNotReady;
    return fTx->Start();
}

void com_openaudio_FWEngine2::DeactivatePCMInput()  { if (fRx) fRx->Stop(); }     // ref @0x118a8
void com_openaudio_FWEngine2::DeactivatePCMOutput() { if (fTx) fTx->Stop(); }     // ref @0x11892

// ref StopAudio @0x11ab2: PCM: TX Stop, then RX Stop (same high-rate condition as the start).
void com_openaudio_FWEngine2::StopAudio()
{
    if (!fAudioStarted) return;
    fAudioStarted = false;
    if (fAC3) { DeactivateAC3Output(); return; }                              // ref vt[0xd30]
    DeactivatePCMOutput();
    if (sampleRate.whole <= 96000 || fRxRefreshAtHighRates) DeactivatePCMInput();
}

// ref StopIsochStreams @0x11ed2: the base decrement, then (when it reached zero) the device's vt[0xb80] = RestartReceptionOfControlPackets.
void com_openaudio_FWEngine2::StopIsochStreams()
{
    super::StopIsochStreams();
    // device->RestartReceptionOfControlPackets() when fIsochRefs == 0: HSCI poll restart arrives with M-E.
}

void com_openaudio_FWEngine2::LogStreamState(const char *where)
{
    OA_LOG("%s: engine state %d, isoch refs %u, active mask 0x%x, pauses %d, connected %d, TX running %d, RX running %d", where,
           (int)getState(), (unsigned)fIsochRefs, (unsigned)fActiveMask, (int)fPauseCount, (int)fConnected,
           fTx ? (int)fTx->isRunning() : -1, fRx ? (int)fRx->isRunning() : -1);
}
