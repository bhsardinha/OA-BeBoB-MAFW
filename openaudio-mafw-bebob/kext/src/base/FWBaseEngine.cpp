#include "FWBaseEngine.h"
#include "Trace.h"
#include "FWBusReset.h"
#include "FWMetaNub.h"
#include <IOKit/firewire/IOFWUtils.h>
#include <IOKit/IOLib.h>
#include <kern/clock.h>

#define super com_openaudio_FWDiscreteEngine
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWBaseEngine, com_openaudio_FWDiscreteEngine)

static const IOReturn kErrBadArg   = (IOReturn)0xE00002C2;
static const IOReturn kErrNoMem    = (IOReturn)0xE00002BD;
static const IOReturn kErrNotReady = (IOReturn)0xE00002BC;
static const IOReturn kErrBusy     = (IOReturn)0xE00002E6;

// ---------------------------------------------------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------------------------------------------------

// ref init @0x5dd4: default 44.1 kHz, FDF index 0 (initHardware sets 1), the 4 KiB SYT table (shared sync [+0x18] = [+0x258]).
// The reference also queries sysctl "hw.optional.sse3" and stores 0 in [+0x340] either way; the sysctl is not available to a
// kext and the stored value is constant, so it is omitted.
bool com_openaudio_FWBaseEngine::init(OSDictionary *properties)
{
    fAC3 = fFlagA = false;
    fLegacyUniqueID = fFlag34b = false;
    fConnected = false;
    fAudioStarted = false;
    fExternalSync = false;
    fRate = 44100;
    fFdf = 0;
    fNub = 0;
    fInStream = fOutStream = 0;
    fPrograms = 0;
    fTx = 0; fRx = 0;
    fShared.engine = this;                                      // ref [+0x2b8] = this
    fShared.outputRunning = 0;
    fShared.sytRead = fShared.sytWrite = 0;
    fSytTable = static_cast<UInt16 *>(IOMalloc(0x1000));
    if (!fSytTable) return false;
    bzero(fSytTable, 0x1000);
    fShared.sytTable = fSytTable;
    return super::init(properties);
}

// ref initHardware @0x56b0
bool com_openaudio_FWBaseEngine::initHardware(IOService *provider)
{
    fDevice = OSDynamicCast(com_openaudio_FWAudioDevice, provider);
    if (!fDevice || !super::initHardware(provider)) return false;
    com_openaudio_FWAVCAudioDevice *avc = OSDynamicCast(com_openaudio_FWAVCAudioDevice, fDevice);
    fNub = (avc && avc->metaNub()) ? avc->metaNub()->fireWireNub() : 0;     // ref [+0x140] = device[0xe0]
    char desc[256];
    snprintf(desc, sizeof(desc), "%s Multichannel", fDevice->GetDescriptionString());   // ref device GetDescriptionString vt[0xa50] (SL ioreg: "FireWire 1814 Multichannel")
    setDescription(desc);
    IOAudioSampleRate rate = { fRate, 0 };
    fDevice->setProperty("MASampleRate", (unsigned long long)fRate, 32);
    setSampleRate(&rate);
    setNumSampleFramesPerBuffer(0x2000);
    fFdf = 1;
    fSytInterval = 8;
    ResetLatency();
    fPending |= 0x00070000;                                                  // ref [+0x346] |= 7
    if (!RecreateAudioStreams(true, true)) return false;
    fPending |= kFWPendingResetBuffers | kFWPendingReconnectPlugs;           // ref [+0x344] |= 0x208000
    return true;
}

// ref stop @0x5cd6
void com_openaudio_FWBaseEngine::stop(IOService *provider)
{
    if (midiRunning()) StopMIDI();                                           // ref [+0x298]
    if (fIsochRefs != 0) StopIsochStreams();
    if (fConnected) DisconnectAllPlugs();
    if (fInStream)  { fInStream->release();  fInStream = 0; }
    if (fOutStream) { fOutStream->release(); fOutStream = 0; }
    if (fPrograms)  { fPrograms->release();  fPrograms = 0; }
    if (fSytTable)  { IOFree(fSytTable, 0x1000); fSytTable = 0; fShared.sytTable = 0; }
    super::stop(provider);
}

// M-S deviation, see the declaration. Same steps as stop() (@0x5cd6) but through the engine's own stop path first, so the audio-device
// running-engine count is balanced; every step is idempotent, so the later stop() finds nothing left to do.
void com_openaudio_FWBaseEngine::Quiesce()
{
    const IOAudioEngineState s = getState();
    if (s == kIOAudioEngineRunning || s == kIOAudioEngineResumed) stopAudioEngine();
    if (midiRunning()) StopMIDI();
    for (int guard = 0; fIsochRefs != 0 && guard < 8; guard++) StopIsochStreams();
    if (fConnected) DisconnectAllPlugs();
}

void com_openaudio_FWBaseEngine::free()
{
    if (fMidiIn)  { for (UInt32 i = 0; i < fShared.midiInPorts; i++) fMidiIn[i].Release();  IOFree(fMidiIn,  (size_t)fShared.midiInPorts * sizeof(FWMIDIPort));  fMidiIn = 0;  fShared.midiInPortArray = 0; }
    if (fMidiOut) { for (UInt32 i = 0; i < fShared.midiOutPorts; i++) fMidiOut[i].Release(); IOFree(fMidiOut, (size_t)fShared.midiOutPorts * sizeof(FWMIDIPort)); fMidiOut = 0; fShared.midiOutPortArray = 0; }
    if (fSytTable) { IOFree(fSytTable, 0x1000); fSytTable = 0; }
    fOutSampleRing.Invalidate(); fOutMixRing.Invalidate(); fInRing180.Invalidate(); fInRing1b0.Invalidate();
    super::free();
}

// ---------------------------------------------------------------------------------------------------------------------
// IOAudioEngine
// ---------------------------------------------------------------------------------------------------------------------

// ref getCurrentSampleFrame @0x3e0c: (lastBank + 1) mod 4 banks of 2048 frames (table {1,2,3,0}).
UInt32 com_openaudio_FWBaseEngine::getCurrentSampleFrame()
{
    static const UInt32 next[4] = { 1, 2, 3, 0 };
    return next[fShared.lastBank & 3] << 11;
}

// ref performAudioEngineStart @0x556e / performAudioEngineStop @0x543c
IOReturn com_openaudio_FWBaseEngine::performAudioEngineStart()
{
    return ActivateSubEngine(1);
}

IOReturn com_openaudio_FWBaseEngine::performAudioEngineStop()
{
    DeactivateSubEngine(1);
    // ref performAudioEngineStop @0x543c: after DeactivateSubEngine it zeroes both peak arrays of the device ([+0x108] the shared meter page,
    // [+0x110] the software peaks, GetPeakLevelCount x 4 bytes) = FWAudioDevice::ClearPeakMeters @0xda1e. Correction (2026-10-17): the first port left this
    // as an empty comment (the meter page did not exist yet), so the panel's meters stayed frozen at their last value after the audio stopped (R76).
    if (fDevice) fDevice->ClearPeakMeters();
    return kIOReturnSuccess;
}

// ref pauseAudioEngine @0x3d54 / resumeAudioEngine @0x3dae. The reference calls IOAudioEngine's (via the GOT), not
// FWDiscreteEngine's, and uses [+0x234] as a nesting counter, [+0x298]/[+0x230] for MIDI.
IOReturn com_openaudio_FWBaseEngine::pauseAudioEngine()
{
    if (++fPauseCount < 2) {
        if (midiRunning()) { fMidiWasRunning = true; StopMIDI(); }   // ref [+0x230] = [+0x298]; vt[0xcf8]
        return IOAudioEngine::pauseAudioEngine();
    }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWBaseEngine::resumeAudioEngine()
{
    if (--fPauseCount == 0) {
        if (fMidiWasRunning && StartMIDI() == kIOReturnSuccess) fMidiWasRunning = false;   // ref: vt[0xcf0] == 0 -> [+0x230] = 0
        return IOAudioEngine::resumeAudioEngine();
    }
    return kIOReturnSuccess;
}

// ref performFormatChange @0x3e54 + ActionPerformFormatChange @0x3e43 -> vt[0xc88]
static IOReturn ActionPerformFormatChange(OSObject *owner, void *stream, void *fmt, void *rate, void *)
{
    return static_cast<com_openaudio_FWBaseEngine *>(owner)->PerformFormatChange(
        static_cast<IOAudioStream *>(stream), static_cast<const IOAudioStreamFormat *>(fmt), static_cast<const IOAudioSampleRate *>(rate));
}

IOReturn com_openaudio_FWBaseEngine::performFormatChange(IOAudioStream *audioStream, const IOAudioStreamFormat *newFormat,
                                                         const IOAudioSampleRate *newSampleRate)
{
    IOWorkLoop *wl = getWorkLoop();
    if (!wl) return kErrNotReady;
    return wl->runAction(&ActionPerformFormatChange, this, audioStream, (void *)newFormat, (void *)newSampleRate);
}

// ---------------------------------------------------------------------------------------------------------------------
// Sample path (M-D2, Q-MD2)
// ---------------------------------------------------------------------------------------------------------------------
static const UInt32 kFWSampleFormatAC3 = 0x63616333;                         // 'cac3'

// ref MainEngineClipOutputSamples @0x3ed4 (vt[0xc38])
IOReturn com_openaudio_FWBaseEngine::MainEngineClipOutputSamples(UInt32 first, UInt32 num, const IOAudioStreamFormat *fmt, IOAudioStream *stream)
{
    if (!fOutStream || !fInStream) return kIOReturnSuccess;
    const IOAudioStreamFormat *of = fOutStream->getFormat();
    const IOAudioStreamFormat *inf = fInStream->getFormat();
    char *mix = (char *)fOutStream->getMixBuffer();                          // stream vt[0xa00]
    if (!of || !inf || !mix) return kIOReturnSuccess;
    if (fOutStreamChannels != of->fNumChannels || fInStreamChannels != inf->fNumChannels) return kIOReturnSuccess;
    {   // bench aid: are the clip requests contiguous? (a request that starts BEFORE the previous one ended = old audio converted again)
        AbsoluteTime tn; clock_get_uptime(&tn); UInt64 ns; absolutetime_to_nanoseconds(tn, &ns);
        ClipStats &c = fClip;
        if (!c.start) { c.start = ns; c.minNum = ~0u; }
        if (c.have && first != c.expect) {
            const UInt32 kind = ((first - c.expect) & 0x1fff) > 0x1000 ? 1 : 2;           // 1 = back (re-clip), 2 = forward (skip), modulo the 8192-frame ring
            if (kind == 1) c.back++; else c.fwd++;
            if (c.nev < 10) { c.ev[c.nev].atMs = (UInt32)((ns - c.start) / 1000000); c.ev[c.nev].expect = c.expect; c.ev[c.nev].first = first; c.ev[c.nev].num = num; c.ev[c.nev].kind = kind; c.nev++; }
        }
        c.have = true; c.expect = (first + num) & 0x1fff; c.calls++;
        if (num > c.maxNum) c.maxNum = num; if (num < c.minNum) c.minNum = num;
    }
    if (of->fIsMixable) mix += (size_t)of->fNumChannels * first * 4;         // ref: stream[0xc0] = format.fIsMixable
    return ClipOutputSamplesWithMixPtrAdjusted((const float *)mix, (SInt32 *)fOutSampleRing.data, first, num, fmt, stream);
}

void com_openaudio_FWBaseEngine::ClipStatsString(char *out, size_t n) const
{
    const ClipStats &c = fClip;
    int w = snprintf(out, n, "CLIP requests %u: frames/request %u..%u, jumps back %u forward %u; first jumps (ms@t expect->first n kind 1=back 2=fwd):",
                     (unsigned)c.calls, c.calls ? (unsigned)c.minNum : 0u, (unsigned)c.maxNum, (unsigned)c.back, (unsigned)c.fwd);
    for (UInt32 i = 0; i < c.nev && w > 0 && (size_t)w < n; i++) w += snprintf(out + w, n - w, " %u@%u %u->%u n%u k%u;", i, (unsigned)c.ev[i].atMs, (unsigned)c.ev[i].expect, (unsigned)c.ev[i].first, (unsigned)c.ev[i].num, (unsigned)c.ev[i].kind);
}

// ref ClipOutputSamplesWithMixPtrAdjusted @0x40aa (vt[0xc90])
IOReturn com_openaudio_FWBaseEngine::ClipOutputSamplesWithMixPtrAdjusted(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num,
                                                                         const IOAudioStreamFormat *, IOAudioStream *)
{
    const IOAudioStreamFormat *of = fOutStream ? fOutStream->getFormat() : 0;
    if (of && of->fSampleFormat == kFWSampleFormatAC3) {
        if (fAC3 && num < 0x601) ClipAC3Output(mix, scratch, first, num);    // vt[0xca8]
    } else {
        ClipPCMOutput(mix, scratch, first, num);                             // vt[0xca0]
    }
    return kIOReturnSuccess;
}

// ref ConvertInputSamplesCore @0x496a (vt[0xc40]). `first` indexes the 8192-frame input ring; the write position of the RX program
// decides whether the requested frames have arrived (underrun: ask for a resync and convert nothing).
void com_openaudio_FWBaseEngine::ConvertInputSamplesCore(UInt32 first, UInt32 num)
{
    if (num == 0 || fAC3 || !fOutStream || !fInStream) return;
    const IOAudioStreamFormat *of = fOutStream->getFormat();
    const IOAudioStreamFormat *inf = fInStream->getFormat();
    if (!of || !inf || fOutStreamChannels != of->fNumChannels || fInStreamChannels != inf->fNumChannels) return;
    IOSynchronizeDCLPrograms();
    if (num + first >= 0x2001) return;
    const UInt32 frameQuads = fMidiChannels + fInChannels;
    if (frameQuads == 0 || !fShared.inputRing.data || !fInRing1b0.data || !fInRing180.data) return;
    const UInt32 written = fShared.inputRing.writeOff / (frameQuads * 4);
    const UInt32 last = first - 1 + num;
    const SInt32 d = (SInt32)(written - last);
    SInt32 alt, mag;
    if (last < written) { alt = d - 0x2000; mag = d; }
    else                { alt = d + 0x2000; mag = -d; }
    if (mag < 0x801) alt = d;
    if (alt < 0) {                                                           // not enough input yet
        if (!fShared.resyncPending) fShared.resyncRequested = true;
        return;
    }
    IOSynchronizeDCLPrograms();
    ConvertInputSampleData(num, (const UInt32 *)(fShared.inputRing.data + (size_t)frameQuads * first * 4),
                           (SInt32 *)fInRing1b0.data, (float *)fInRing180.data);
    fInputConvertedEnd = num + first;
}

// ref ConvertInputSampleData @0x4624 (vt[0xc98], asm 0x4624-0x46cd). With an input map ([+0x320] = DeviceChannels +0x18, the FW410's
// 176.4/192 kHz row) ConvertSampleFramesFromInput(src, scratch, in, inStream, num, map, midi != 0) places the wire slots; otherwise the
// model's input proc runs. Then the inStream * num samples become floats.
void com_openaudio_FWBaseEngine::ConvertInputSampleData(UInt32 num, const UInt32 *src, SInt32 *scratch, float *dst)
{
    if (fChannelMap[0]) {
        FWConvertSampleFramesFromInput(src, scratch, fInChannels, fInStreamChannels, num, fChannelMap[0], fMidiChannels != 0);
    } else {
        if (!fInputProc) return;                                             // ref falls back to LLRR/LRLR default procs; every model installs one
        fInputProc(fInChannels * num, src, scratch, fInputProcArg);
    }
    FWInt24ScaledToFloat32(scratch, dst, fInStreamChannels * num);
}

// ref performErase @0x4e08: zero the output stream's MIX buffer (stream vt[0xa00] = getMixBuffer, vt[0xa08] = getMixBufferSize - the same slots
// MainEngineClipOutputSamples @0x3ed4 reads the audio from, Q-MD2 2) between the erase head and the current frame.
// Correction (2026-10-17): the first port zeroed getSampleBuffer(), which is not what is clipped or played. The mix buffer was therefore never erased:
// while a second client mixed in (e.g. the Finder's trash sound over a playing stream) old laps of the 8192-frame buffer were converted again - a ~186 ms
// loop in the output, levels above the signal, meters stuck (capture 88, R75).
void com_openaudio_FWBaseEngine::performErase()
{
    if (!getRunEraseHead() || getState() != kIOAudioEngineRunning || !outputStreams) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(outputStreams);
    if (!it) return;
    const UInt32 cur = getCurrentSampleFrame();
    const UInt32 erase = status->fEraseHeadSampleFrame;
    com_openaudio_FWStream *s;
    while ((s = OSDynamicCast(com_openaudio_FWStream, it->getNextObject()))) {
        s->lockStreamForIO();
        UInt8 *buf = static_cast<UInt8 *>(s->getMixBuffer());
        const UInt32 bpf = s->getFormat()->fNumChannels * 4;
        const UInt32 size = s->getMixBufferSize();
        if (cur < erase) {                                            // the head wrapped: clear [0, cur) and [erase, end)
            if (cur * bpf <= size && numSampleFramesPerBuffer * bpf <= size && buf && bpf) {
                bzero(buf, cur * bpf);
                bzero(buf + erase * bpf, (numSampleFramesPerBuffer - erase) * bpf);
            }
        } else if (cur * bpf <= size && buf && bpf) {
            bzero(buf + erase * bpf, (cur - erase) * bpf);
        }
        s->unlockStreamForIO();
    }
    status->fEraseHeadSampleFrame = cur;
    it->release();
}

// ---------------------------------------------------------------------------------------------------------------------
// rate / format chain (Q15c)
// ---------------------------------------------------------------------------------------------------------------------

// ref PerformSampleRateChangeCore @0x479e (decomp FWBaseEngine__PerformSampleRateChangeCore.c): cache the rate, derive SYT
// interval and FDF index, tell every DCL program (vt[0x128] = ApplyRateChange -- here: nothing to do until the engine's
// UpdateDCLProgramsWithNewPacketSizes runs from the pending-state machine), reset latency, mark dirty.
bool com_openaudio_FWBaseEngine::PerformSampleRateChangeCore(const IOAudioSampleRate *rate)
{
    fRate = rate->whole;
    const FWRateInfo &ri = FWRateLookup(fRate);                              // unknown rate -> 44.1 kHz row (ref LAB_0000484e)
    fSytInterval = ri.sytInterval;
    fFdf = ri.fdf;
    ResetLatency();
    fPending |= 0x001d0000;                                                  // ref [+0x346] |= 0x1d
    return true;
}

// ref ResetLatency @0x3f7a
void com_openaudio_FWBaseEngine::ResetLatency()
{
    if (!fDevice) return;
    const UInt32 rt = fDevice->GetRoundTripLatencyForFDF(fFdf);              // vt[0xab8]
    setInputSampleLatency((rt + 1) >> 1);                                    // vt[0xb70]
    setOutputSampleLatency(rt >> 1);                                         // vt[0xb68]
    setSampleOffset(fDevice->GetSafetyOffsetForFDF(fFdf));                   // vt[0xb78] <- vt[0xac0]
}

// ref ChangeSampleRateToFDFValue @0x52e8
void com_openaudio_FWBaseEngine::ChangeSampleRateToFDFValue(UInt32 fdf)
{
    bool valid;
    UInt32 rate = FWRateForFdfIndex(fdf, &valid);
    if (!valid) {
        IOLog(OA_TAG "M-FW: invalid sample rate detected (FDF:%X)\n", (unsigned)fdf);
        rate = fRate;
    }
    if (rate != fRate || fExtSyncLocked) {
        pauseAudioEngine();
        beginConfigurationChange();                                          // vt[0xb00]
        fPending |= 0x006f0000;                                              // ref [+0x346] |= 0x6f
        fDevice->RequestFormatUpdateForSampleRate(rate);                     // device vt[0xa78]
        CompletePendingStateChanges();
        completeConfigurationChange();                                       // vt[0xb08]
        resumeAudioEngine();
    }
}

// ref SetIOFormat @0x42aa (decomp FWBaseEngine__SetIOFormat.c)
IOReturn com_openaudio_FWBaseEngine::SetIOFormat(const FWDeviceChannels &ch, UInt32 rateIndex)
{
    const UInt32 newRate = ch.outRates[rateIndex].whole;
    const bool acThing = configurationChangeInProgress;                      // ref this[0xa5]
    if (!acThing) beginConfigurationChange();                                // vt[0xb00]
    if (ch.out != fOutChannels || ch.in != fInChannels || newRate != sampleRate.whole)
        fPending |= 0x006f0000;                                              // ref [+0x346] |= 0x6f
    fOutChannels       = ch.out;                                             // [+0x2e0]
    fOutStreamChannels = ch.outStream;                                       // [+0x2ec]
    fInChannels        = ch.in;                                              // [+0x2e4]
    fInStreamChannels  = ch.inStream;                                        // [+0x2f0]
    fDevice->ResetRearrangementProcs(this);                                  // ref device vt[0xaf8]: clip/convert procs for the new counts
    if (newRate != sampleRate.whole) {
        IOAudioSampleRate r = { newRate, 0 };
        setSampleRate(&r);                                                   // vt[0xb58]
        PerformSampleRateChangeCore(&r);                                     // vt[0xce0] (subclass hook)
    }
    char tr[160];
    snprintf(tr, sizeof(tr), "SetIOFormat start: want in %u out %u rate %u; streams now in %u out %u", (unsigned)ch.inStream, (unsigned)ch.outStream, (unsigned)newRate,
             fInStream && fInStream->getFormat() ? (unsigned)fInStream->getFormat()->fNumChannels : 0u,
             fOutStream && fOutStream->getFormat() ? (unsigned)fOutStream->getFormat()->fNumChannels : 0u);
    if (fDevice) fDevice->DebugTrace(tr);
    IOReturn r = SetStreamFormat(fInStream, ch, rateIndex);
    if (r == kIOReturnSuccess) r = SetStreamFormat(fOutStream, ch, rateIndex);
    snprintf(tr, sizeof(tr), "SetIOFormat done: result 0x%x; streams now in %u out %u", (unsigned)r,
             fInStream && fInStream->getFormat() ? (unsigned)fInStream->getFormat()->fNumChannels : 0u,
             fOutStream && fOutStream->getFormat() ? (unsigned)fOutStream->getFormat()->fNumChannels : 0u);
    if (fDevice) fDevice->DebugTrace(tr);
    if (r == kIOReturnSuccess) {
        fChannelMap[0] = ch.channelMap[0];
        fChannelMap[1] = ch.channelMap[1];
        fChannelMap[2] = ch.channelMap[2];
        if (!acThing) completeConfigurationChange();                         // vt[0xb08]
    } else if (!acThing) {
        cancelConfigurationChange();                                         // vt[0xb10]
    }
    return r;
}

static IOReturn VerifyFormatsAction(OSObject *owner, void *, void *, void *, void *)
{
    static_cast<com_openaudio_FWBaseEngine *>(owner)->VerifyStreamFormats();
    return kIOReturnSuccess;
}

static void VerifyFormatsThread(void *arg)
{
    com_openaudio_FWBaseEngine *e = static_cast<com_openaudio_FWBaseEngine *>(arg);
    if (IOWorkLoop *wl = e->getWorkLoop()) wl->runAction(&VerifyFormatsAction, e);   // blocks until IOAudioStream::setFormat has released the gate
    e->release();
}

void com_openaudio_FWBaseEngine::ScheduleVerifyStreamFormats()
{
    retain();
    if (!FWRunOnNewThread(&VerifyFormatsThread, this)) release();
}

void com_openaudio_FWBaseEngine::VerifyStreamFormats()
{
    if (!fDevice || !fInStream || !fOutStream || !fInStream->getFormat() || !fOutStream->getFormat()) return;
    FWDeviceChannels ch;
    fDevice->GetDeviceChannelsForSampleRate(fRate, ch);
    const UInt32 haveIn = fInStream->getFormat()->fNumChannels, haveOut = fOutStream->getFormat()->fNumChannels;
    if (haveIn == ch.inStream && haveOut == ch.outStream) return;
    char tr[140];
    snprintf(tr, sizeof(tr), "VerifyStreamFormats: streams in %u out %u but the row for %u Hz is in %u out %u: re-applying", (unsigned)haveIn, (unsigned)haveOut,
             (unsigned)fRate, (unsigned)ch.inStream, (unsigned)ch.outStream);
    fDevice->DebugTrace(tr);
    const bool already = configurationChangeInProgress;
    if (!already) beginConfigurationChange();
    IOReturn r = SetStreamFormat(fInStream, ch, 0);
    if (r == kIOReturnSuccess) r = SetStreamFormat(fOutStream, ch, 0);
    if (!already) { if (r == kIOReturnSuccess) completeConfigurationChange(); else cancelConfigurationChange(); }
}

// ref SetStreamFormat @0x4192: lpcm / signed int, 24 valid bits in 32, high-byte aligned, big-endian, mixable; channel count
// and rate list depend on the stream direction (output stream: outStream channels + out rates).
IOReturn com_openaudio_FWBaseEngine::SetStreamFormat(IOAudioStream *stream, const FWDeviceChannels &ch, UInt32 rateIndex)
{
    (void)rateIndex;
    if (!stream) return kErrBadArg;
    const bool output = (stream->getDirection() == kIOAudioStreamDirectionOutput);
    IOAudioStreamFormat fmt;
    bzero(&fmt, sizeof(fmt));
    fmt.fNumChannels = output ? ch.outStream : ch.inStream;
    fmt.fSampleFormat = kIOAudioStreamSampleFormatLinearPCM;                 // 'lpcm' 0x6c70636d
    fmt.fNumericRepresentation = kIOAudioStreamNumericRepresentationSignedInt;   // 'sint' 0x73696e74
    fmt.fBitDepth = 0x18;
    fmt.fBitWidth = 0x20;
    // Correction (2026-10-16, ADAT/S-PDIF switch work): the reference writes literal 0 here, and the 10.13 header defines
    // kIOAudioStreamAlignmentLowByte = 0, kIOAudioStreamAlignmentHighByte = 1 (the clone assumed "byte 0 = high byte" and published 1).
    // Evidence: the official engine on the SL rig publishes IOAudioStreamAlignment = 0 (B5 capture 67c, FW1814 streams).
    fmt.fAlignment = kIOAudioStreamAlignmentLowByte;
    fmt.fByteOrder = kIOAudioStreamByteOrderBigEndian;                       // ref byte 0
    fmt.fIsMixable = 1;
    fmt.fDriverTag = 0;
    const FWSampleRateEntry *rates = output ? ch.outRates : ch.inRates;
    // The official FW1814 streams carry the format extension on every PCM format: FormatFlags 0, FramesPerPacket 1, BytesPerPacket =
    // channels * 4 (B5 capture 67c: 12 ch -> 48, 16 ch -> 64). The clone published the formats without it.
    IOAudioStreamFormatExtension ext = { kFormatExtensionCurrentVersion, 0, 1, fmt.fNumChannels * 4 };
    stream->clearAvailableFormats();                                         // ref vt[0xa68]
    for (; rates->whole != 0; rates++) {                                     // ref ReplicateFormatForEachSampleRate @0x3a9a
        IOAudioSampleRate sr = { rates->whole, rates->fraction };
        stream->addAvailableFormat(&fmt, &ext, &sr, &sr);
    }
    if (output && !fExternalSync && fDevice && fDevice->SupportsAC3()) AddAC3Format(stream);   // ref SetStreamFormat @0x4192: output stream, not in external sync
    return stream->setFormat(&fmt, &ext, false);                             // ref vt[0xa38] (format, extension, callDriver = false)
}

// ---------------------------------------------------------------------------------------------------------------------
// pending-state machine (ref CompletePendingStateChanges @0x3ae6 / Action @0x3b2c)
// ---------------------------------------------------------------------------------------------------------------------

static IOReturn ActionCompletePending(OSObject *owner, void *, void *, void *, void *);

IOReturn com_openaudio_FWBaseEngine::CompletePendingStateChanges()
{
    IOWorkLoop *wl = getWorkLoop();
    if (!wl) return kErrNotReady;
    return wl->runAction(&ActionCompletePending, this);
}

static IOReturn ActionCompletePending(OSObject *owner, void *, void *, void *, void *)
{
    com_openaudio_FWBaseEngine *e = static_cast<com_openaudio_FWBaseEngine *>(owner);
    return e->CompletePendingStateChangesAction();
}

IOReturn com_openaudio_FWBaseEngine::CompletePendingStateChangesAction()
{
    UInt32 p = fPending;
    const bool needsReconnect = fConnected && (p & (kFWPendingPacketSizes | kFWPendingReconnectPlugs));    // ref 0x48000
    if (p & kFWPendingBusy) return kIOReturnSuccess;                        // ref returns 0 without clearing
    if (needsReconnect) {
        DisconnectAllPlugs();                                                // vt[0xcc0]
        fPending |= kFWPendingReconnectPlugs;
        p = fPending;
    }
    IOReturn result = kIOReturnSuccess;
    if (p & kFWPendingRecreateStreams) {
        if (!RecreateAudioStreams(true, true)) { result = kErrNoMem; goto done; }
        p = fPending;
    }
    if (p & kFWPendingResetBuffers) {
        if (!ResetSampleBuffers()) { result = kErrNoMem; goto done; }
        p = fPending;
    }
    if (p & kFWPendingResetPairFormats) { ResetStereoPairIOFormats(); p = fPending; }
    if (p & kFWPendingPacketSizes)      { UpdateDCLProgramsWithNewPacketSizes(); p = fPending; }   // vt[0xc78]
    {
        const bool skipConnect = (p & kFWPendingBandwidth) && !fExternalSync && !fTwoPhaseConnect;   // ref 0x3c0d..0x3c1c
        bool doRest = true;
        if ((p & kFWPendingReconnectPlugs) && !skipConnect) {
            if (ConnectAllPlugs() != kIOReturnSuccess) doRest = false;       // vt[0xcb8]; the reference still returns 0 and clears
            else p = fPending;
        }
        if (doRest) {
            if (p & kFWPendingBandwidth) { RecalculateBandwidth(); p = fPending; }   // vt[0xcc8]
            if (p & kFWPendingSettle)    { IOSleep(1000); p = fPending; }
            if (p & kFWPendingRateNotify) fDevice->SetStaticPropertyAndNotifyUserClients("MASampleRate", sampleRate.whole);
            if (fConnected && fMidiWasRunning) fMidiWasRunning = (StartMIDI() != kIOReturnSuccess);   // ref [+0x230] <- (vt[0xcf0] != 0)
        }
    }
done:
    fPending = 0;
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// DCL programs and isoch start/stop
// ---------------------------------------------------------------------------------------------------------------------

IOReturn com_openaudio_FWBaseEngine::AddDCLProgram(com_openaudio_FWDCLProgram *p)
{
    if (fConnected) return kErrNotReady;
    if (!fPrograms) {
        fPrograms = OSArray::withCapacity(8);
        if (!fPrograms) return kErrNoMem;
    }
    fPrograms->setObject(p);
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWBaseEngine::RemoveDCLProgram(com_openaudio_FWDCLProgram *p)
{
    if (fConnected) return kErrNotReady;
    if (fPrograms) {
        for (unsigned i = 0; i < fPrograms->getCount(); i++)
            if (fPrograms->getObject(i) == p) { fPrograms->removeObject(i); return kIOReturnSuccess; }
    }
    return kErrBadArg;
}

// ref IOSynchronizeDCLPrograms @0x4902: vt[0x148] on each program (IOSynchronize -- a no-op for the TX program, RX resync hook)
void com_openaudio_FWBaseEngine::IOSynchronizeDCLPrograms()
{
    if (!fPrograms) return;
    for (unsigned i = 0; i < fPrograms->getCount(); i++) {
        com_openaudio_FWDCLProgram *p = OSDynamicCast(com_openaudio_FWDCLProgram, fPrograms->getObject(i));
        if (p) p->IOSynchronize();
    }
}

// ref StartIsochStreams @0x54b0 (decomp FWBaseEngine__StartIsochStreams.c)
IOReturn com_openaudio_FWBaseEngine::StartIsochStreams()
{
    if (!fConnected) return kErrNotReady;
    if (fIsochRefs++ == 0) {
        bzero(&fClip, sizeof(fClip));                                        // bench aid
        fShared.resyncRequested = false;                                     // ref [+0x250]
        fShared.resyncPending = false;                                       // ref [+0x251]
        fShared.lastBank = 0;                                                // ref [+0x248]
        if (fShared.inputRing.data) bzero(fShared.inputRing.data, fShared.inputRing.size);
        fShared.inputRing.readOff = fShared.inputRing.writeOff = 0;
        fShared.inputRing.readWraps = fShared.inputRing.writeWraps = 0;
        IOReturn r = StartAudio();                                           // vt[0xcd0]
        if (r != kIOReturnSuccess) { fIsochRefs--; return r; }
    }
    return kIOReturnSuccess;
}

// ref StopIsochStreams @0x416c
void com_openaudio_FWBaseEngine::StopIsochStreams()
{
    if (fIsochRefs != 0 && --fIsochRefs == 0) StopAudio();                   // vt[0xcd8]
}

// ref StartAudio @0x4bca (base version; FWEngine2 overrides it, Q-START): ResetPort(start cycle) on every program, Start on every
// program, then takeTimeStamp(false). The start cycle is now + 0xA0000 (A.K.A. 160 cycles) for the input program, 0 for the output.
IOReturn com_openaudio_FWBaseEngine::StartAudio()
{
    if (!fNub || !fPrograms) return kErrNotReady;
    UInt32 now = 0;
    fNub->getBus()->getCycleTime(now);
    const UInt32 startCycle = AddFWCycleTimeToFWCycleTime(now, 0xA0000);
    IOReturn r = kIOReturnSuccess;
    for (unsigned i = 0; i < fPrograms->getCount() && r == kIOReturnSuccess; i++) {
        com_openaudio_FWDCLProgram *p = OSDynamicCast(com_openaudio_FWDCLProgram, fPrograms->getObject(i));
        if (p) r = p->ResetPort(p->isTx() ? 0 : startCycle);
    }
    for (unsigned i = 0; i < fPrograms->getCount() && r == kIOReturnSuccess; i++) {
        com_openaudio_FWDCLProgram *p = OSDynamicCast(com_openaudio_FWDCLProgram, fPrograms->getObject(i));
        if (p) r = p->Start();
    }
    if (r == kIOReturnSuccess) takeTimeStamp(false);                          // vt[0xae0](0, &deadline)
    return r;
}

// ref StopAudio @0x4738: vt[0x140] (Stop) on every program
void com_openaudio_FWBaseEngine::StopAudio()
{
    if (!fPrograms) return;
    for (unsigned i = 0; i < fPrograms->getCount(); i++) {
        com_openaudio_FWDCLProgram *p = OSDynamicCast(com_openaudio_FWDCLProgram, fPrograms->getObject(i));
        if (p) p->Stop();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// streams and sample buffers
// ---------------------------------------------------------------------------------------------------------------------

// ref createNewAudioStream @0x55b6. `attachToEngine` mirrors the reference's engine argument being NULL for a stream that is
// only created (not added). The stream starts with one placeholder format (ref C_105), the sample buffer is set, and the
// stream is added to the engine.
com_openaudio_FWStream *com_openaudio_FWBaseEngine::createNewAudioStream(bool attachToEngine, IOAudioStreamDirection dir, void *buffer, UInt32 bytes)
{
    com_openaudio_FWStream *s = OSTypeAlloc(com_openaudio_FWStream);
    if (!s) return 0;
    if (!s->initWithAudioEngine(this, dir, 1, 0, 0)) { s->release(); return 0; }
    IOAudioStreamFormat fmt;
    bzero(&fmt, sizeof(fmt));
    fmt.fNumChannels = 2;
    fmt.fSampleFormat = kIOAudioStreamSampleFormatLinearPCM;
    fmt.fNumericRepresentation = kIOAudioStreamNumericRepresentationSignedInt;
    fmt.fBitDepth = 0x18; fmt.fBitWidth = 0x20;
    fmt.fAlignment = kIOAudioStreamAlignmentHighByte;
    fmt.fByteOrder = kIOAudioStreamByteOrderBigEndian;
    fmt.fIsMixable = 1;
    s->addAvailableFormat(&fmt, &sampleRate, &sampleRate);
    s->setSampleBuffer(buffer, bytes);
    if (attachToEngine) addAudioStream(s);
    return s;
}

// ref RecreateAudioStreams @0x4546
bool com_openaudio_FWBaseEngine::RecreateAudioStreams(bool createInput, bool createOutput)
{
    detachAudioStreams();                                                    // vt[0xbf0]
    if (fOutStream) { fOutStream->release(); fOutStream = 0; }
    if (fInStream)  { fInStream->release();  fInStream = 0; }
    fOutStream = createNewAudioStream(createOutput, kIOAudioStreamDirectionOutput, fOutSampleRing.data, fOutStreamBytes);
    if (!fOutStream) return false;
    fInStream = createNewAudioStream(createInput, kIOAudioStreamDirectionInput, fShared.inputRing.data, fInStreamBytes);
    if (!fInStream) return false;
    fPending |= 0x00200000;                                                  // ref [+0x346] |= 0x20
    return true;
}

// ref ResetSampleBuffers @0x57f8: five rings of (channels + midi) * 0x8000 bytes (32768 bytes = 8192 frames * 4 per channel,
// matching setNumSampleFramesPerBuffer(0x2000) in initHardware).
bool com_openaudio_FWBaseEngine::ResetSampleBuffers()
{
    if (!fOutStream) return true;
    const UInt32 m = fMidiChannels;
    fOutStreamBytes = (fOutStreamChannels + m) * 0x8000;                     // [+0x2d8]
    fInStreamBytes  = (fInStreamChannels + m) * 0x8000;                      // [+0x2dc]
    const UInt32 inBytes  = (fInChannels + m) * 0x8000;
    const UInt32 outBytes = (fOutChannels + m) * 0x8000;
    if (!fOutSampleRing.InitializeWithSizeInBytes(outBytes) || !fOutMixRing.InitializeWithSizeInBytes(outBytes) ||
        !fShared.inputRing.InitializeWithSizeInBytes(inBytes) || !fInRing180.InitializeWithSizeInBytes(inBytes) ||
        !fInRing1b0.InitializeWithSizeInBytes(inBytes))
        return false;
    fOutStream->setSampleBuffer(fOutSampleRing.data, fOutSampleRing.size);
    fOutStream->setMixBuffer(fOutMixRing.data, fOutMixRing.size);
    fInStream->setSampleBuffer(fShared.inputRing.data, fShared.inputRing.size);
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------

// ref SetExternalSyncEnabled @0x53ec: refused while streams run.
IOReturn com_openaudio_FWBaseEngine::SetExternalSyncEnabled(bool on)
{
    if (fExternalSync != on) {
        if (fIsochRefs != 0) {
            IOLog("\nM-Audio FireWire: Cannot change sync mode while audio/MIDI streams are running.\n");
            return kErrBusy;
        }
        fExternalSync = on;
    }
    return kIOReturnSuccess;
}

void com_openaudio_FWBaseEngine::TellClientsSampleRateChanged(UInt32 rate)
{
    if (fDevice) fDevice->SetStaticPropertyAndNotifyUserClients("MASampleRate", rate);
}

// ---- unique IDs ------------------------------------------------------------------------------------------------------------------

OSString *com_openaudio_FWBaseEngine::getLocalUniqueID()
{
    OSNumber *guid = fNub ? OSDynamicCast(OSNumber, fNub->getProperty("GUID")) : 0;
    char buf[32];
    if (!guid) snprintf(buf, sizeof(buf), "<unknown>");
    else {
        const UInt64 g = guid->unsigned64BitValue();
        if (g < 0x100000000ULL) snprintf(buf, sizeof(buf), "0x%lX", (unsigned long)g);
        else snprintf(buf, sizeof(buf), "0x%lX%08lX", (unsigned long)(g >> 32), (unsigned long)(g & 0xffffffff));
    }
    return OSString::withCString(buf);
}

OSString *com_openaudio_FWBaseEngine::getGlobalUniqueID()
{
    const char *cls = UniqueIDClassName();
    // Correction (2026-10-17): the reference's middle segment is vt[0x3b8](0) (asm 0x5b84), which the clone had mapped to getName(). The SL registry
    // disproves that: the official engine's name is its class name, yet its IOAudioEngineGlobalUniqueID is "com_m_audio_FWEngine2:0x<GUID>" with NO
    // middle segment (capture 79b). The slot is getLocation() (NULL for an engine), so the segment is skipped, as on the official driver.
    const char *name = getLocation();
    OSString *local = getLocalUniqueID();
    char buf[160];
    snprintf(buf, sizeof(buf), "%s:", cls);
    if (name) { strlcat(buf, name, sizeof(buf)); strlcat(buf, ":", sizeof(buf)); }
    if (local) { strlcat(buf, local->getCStringNoCopy(), sizeof(buf)); local->release(); }
    return OSString::withCString(buf);
}

// ref FWUserClient::CoreStartMIDI @0x717c: while the engine is paused (pause count > 0) MIDI is only remembered, otherwise started
IOReturn com_openaudio_FWBaseEngine::UserStartMIDI()
{
    if (fPauseCount > 0 && !fMidiWasRunning) { fMidiWasRunning = true; return kIOReturnSuccess; }
    return StartMIDI();
}

// ref CoreStopMIDI @0x714a
void com_openaudio_FWBaseEngine::UserStopMIDI()
{
    if (!midiRunning()) {
        if (fPauseCount > 0 && fMidiWasRunning) fMidiWasRunning = false;
        return;
    }
    StopMIDIAndClearBuffers();
}

// ---- AC3 (M-I) -----------------------------------------------------------------------------------------------------------------------

// ref AddAC3Format @0x39a0: the stream also offers 'cac3' (IEC 61937 in 16-bit little-endian words, 1536 frames = 0x1800 bytes per packet,
// non-mixable) at 44.1, 48 and 96 kHz
void com_openaudio_FWBaseEngine::AddAC3Format(IOAudioStream *stream)
{
    IOAudioStreamFormat fmt;
    bzero(&fmt, sizeof(fmt));
    fmt.fNumChannels = 2;
    fmt.fSampleFormat = 0x63616333;                                          // 'cac3'
    fmt.fNumericRepresentation = kIOAudioStreamNumericRepresentationSignedInt;
    fmt.fBitDepth = 0x10; fmt.fBitWidth = 0x10;
    fmt.fAlignment = 0;
    fmt.fByteOrder = kIOAudioStreamByteOrderLittleEndian;
    fmt.fIsMixable = 0;
    IOAudioStreamFormatExtension ext = { 1, 0, 0x600, 0x1800 };
    static const UInt32 rates[3] = { 48000, 44100, 96000 };
    for (int i = 0; i < 3; i++) {
        IOAudioSampleRate r = { rates[i], 0 };
        stream->addAvailableFormat(&fmt, &ext, &r, &r);
    }
}

// ref EnableAC3Format @0x445e: only while the transport is idle; plugs are dropped (and re-made by the pending-state machine) and the
// output stream becomes 2 channels wide
void com_openaudio_FWBaseEngine::EnableAC3Format()
{
    if (fAC3 || fAudioStarted) return;
    UInt32 p = fPending;
    if (fConnected) { DisconnectAllPlugs(); p = fPending | kFWPendingReconnectPlugs; }
    fAC3 = true;
    fOutStreamChannels = 2;
    fPending = p | 0x00270000;
    fDevice->AC3Activated(true);
}

// ref DisableAC3Format @0x596a
void com_openaudio_FWBaseEngine::DisableAC3Format()
{
    const bool wasConnected = fConnected;
    FWDeviceChannels row;
    fDevice->GetDeviceChannels(row);
    if (!fAC3 || fAudioStarted) return;
    if (wasConnected) DisconnectAllPlugs();
    fAC3 = false;
    fOutStreamChannels = row.outStream;
    ResetSampleBuffers();
    UpdateDCLProgramsWithNewPacketSizes();
    if (wasConnected) ConnectAllPlugs();
    fDevice->AC3Activated(false);
}
