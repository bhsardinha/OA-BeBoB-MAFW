#include "FWAudioDeviceChain.h"
#include "FWSamples.h"
#include "FWMetaNub.h"
#include "Trace.h"
#include "FWAvc.h"
#include "FWCMP.h"
#include "FWEngine2.h"
#include "FWMixer.h"
#include "FWPropertyDict.h"
#include "FWMixerRegs.h"

#include <libkern/c++/OSString.h>

#define super IOAudioDevice

OSDefineMetaClassAndAbstractStructors(com_openaudio_FWDiscreteDevice, IOAudioDevice)
#undef super
#define super com_openaudio_FWDiscreteDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWAudioDevice, com_openaudio_FWDiscreteDevice)
#undef super
#define super com_openaudio_FWAudioDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWAVCAudioDevice, com_openaudio_FWAudioDevice)
#undef super
#define super com_openaudio_FWAVCAudioDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWRev1AudioDevice, com_openaudio_FWAVCAudioDevice)
#undef super
#define super com_openaudio_FWRev1AudioDevice
OSDefineMetaClassAndAbstractStructors(com_m_audio_FW1814Device, com_openaudio_FWRev1AudioDevice)
#undef super
#define super com_m_audio_FW1814Device
OSDefineMetaClassAndStructors(com_openaudio_FW1814Device, com_m_audio_FW1814Device)
#undef super

void com_openaudio_FWDiscreteDevice::free() { IOAudioDevice::free(); }

// ref @0x22f78. Candidate rejection is the DESIGN of the reference's firmware-state handling (Q-SDK5):
//   score = 0; provider must be an FWMetaNub with a product name equal to ours; Model_ID equal if both present;
//   bootloader active -> reject; firmware match not yet started ([+0xf3]==0) -> reject; else score 0x7FFFFF.
IOService *com_openaudio_FWAVCAudioDevice::probe(IOService *provider, SInt32 *score)
{
    *score = 0;
    com_openaudio_FWMetaNub *nub = OSDynamicCast(com_openaudio_FWMetaNub, provider);
    OSString *theirs = OSDynamicCast(OSString, provider ? provider->getProperty("FireWire Product Name") : 0);
    if (!nub || !theirs) return 0;

    OSObject *mine = getProperty("FireWire Product Name");
    if (!mine || !theirs->isEqualTo(mine)) return 0;

    OSObject *nubModel = provider->getProperty("Model_ID");
    OSObject *myModel  = getProperty("Model_ID");
    if (nubModel && myModel && !nubModel->isEqualTo(myModel)) return 0;

    if (nub->bootloaderActive())        return 0;     // ref 0x23050
    if (!nub->firmwareMatchStarted())   return 0;     // ref 0x2305a: round 1 always ends here
    *score = 0x7FFFFF;                                // ref 0x23068
    return this;
}

// ref FWAVCAudioDevice::initHardware: the generic part (FWAudioDevice::initHardware, FWAudioDeviceProps.cpp), then the blank-slate clock
// source, the factory/current mixer settings resend and the start of the HSCI control-packet poll (Q3b §1).
bool com_openaudio_FWAVCAudioDevice::initHardware(IOService *provider)
{
    fMetaNub = OSDynamicCast(com_openaudio_FWMetaNub, provider);
    if (!fMetaNub) return false;
    if (!com_openaudio_FWAudioDevice::initHardware(provider)) return false;
    SetBlankSlateClockSource();                                  // ref vt[0xb58]
    ResendCurrentMixerSettings();                                // ref vt[0xb00]
    if (!StartReceptionOfControlPackets()) return false;         // ref vt[0xb78]
    OA_LOG("FW1814Device initHardware: up");
    return true;
}

// ref AllocateConnectionManager @0x23c72: the AV/C unit on the device nub, then the connection manager bound to this device.
bool com_openaudio_FWAVCAudioDevice::AllocateConnectionManager()
{
    IOFireWireNub *nub = fMetaNub ? fMetaNub->fireWireNub() : 0;
    if (!nub) return false;
    fAvcUnit = OSTypeAlloc(com_openaudio_FWAvcUnit);
    if (!fAvcUnit || !fAvcUnit->Setup(this, nub)) return false;           // ref: init(props of the nub), attach(this device), start(nub)
    fConnMgr = OSTypeAlloc(com_openaudio_FWAvcConnectionManager);
    if (!fConnMgr || !fConnMgr->InitWithDevice(this, nub)) return false;
    return true;
}

void com_openaudio_FWAVCAudioDevice::ReleaseConnectionManager()
{
    if (fConnMgr) { fConnMgr->release(); fConnMgr = 0; }
    if (fAvcUnit) { fAvcUnit->release(); fAvcUnit = 0; }
}

void com_openaudio_FWAVCAudioDevice::stop(IOService *provider)
{
    com_openaudio_FWAudioDevice::stop(provider);
    ReleaseConnectionManager();
}

void com_openaudio_FWAVCAudioDevice::free()
{
    ReleaseConnectionManager();
    fMetaNub = 0;
    com_openaudio_FWAudioDevice::free();
}

void com_openaudio_FWRev1AudioDevice::free() { com_openaudio_FWAVCAudioDevice::free(); }
void com_openaudio_FW1814Device::free()
{
    if (fMixer) { fMixer->release(); fMixer = 0; }
    com_openaudio_FWRev1AudioDevice::free();
}

// ---- M-D device-side rate parameters ----------------------------------------------------------------------------------

#include "FWBaseEngine.h"
#include "FWEngine2.h"

// ref GetBandwidthBytesAtSampleRate @0x1ee2
UInt32 com_openaudio_FWAudioDevice::GetBandwidthBytesAtSampleRate(UInt32 rateHz) const
{
    FWDeviceChannels ch;
    GetDeviceChannelsForSampleRate(rateHz, ch);
    return FWBandwidthBytesPerCycle(ch, fMainEngine ? fMainEngine->sytInterval() : 0);
}

// ref RequestFormatUpdateForSampleRate @0x2a84: the rate's index in the table's rate list (0 when absent), then SetIOFormat.
IOReturn com_openaudio_FWAudioDevice::RequestFormatUpdateForSampleRate(UInt32 rateHz)
{
    if (!fMainEngine) return kIOReturnNotReady;
    FWDeviceChannels ch;
    GetDeviceChannelsForSampleRate(rateHz, ch);
    UInt32 idx = 0;
    for (UInt32 i = 0; ch.outRates[i].whole != 0; i++) if (ch.outRates[i].whole == rateHz) { idx = i; break; }
    return fMainEngine->SetIOFormat(ch, idx);
}

// ref FWRev1AudioDevice::AVCControlPlugSignalFormat @0x13734
IOReturn com_openaudio_FWAVCAudioDevice::AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf)
{
    return FWAvcControlPlugSignalFormat(fAvcUnit, input, plug, fdf);
}

// ref createAudioEngine @0x2fd8
bool com_openaudio_FWAudioDevice::createAudioEngine()
{
    deactivateAllAudioEngines();                                 // ref vt[0x988]
    fMainEngine = 0;
    com_openaudio_FWBaseEngine *eng = AllocateAudioEngine();
    if (!eng) return false;
    if (!eng->init(0)) { eng->release(); return false; }
    SetupAudioEngine(eng);
    SetChannelNamesForMainEngine(eng);
    if (activateAudioEngine(eng) != kIOReturnSuccess) { eng->release(); return false; }
    if (OSString *uid = eng->getGlobalUniqueID()) { setProperty("MAMainEngineUniqueID", uid); uid->release(); }   // ref vt[0xb30]
    fMainEngine = eng;                                           // ref [+0xe8]
    eng->release();                                              // the device keeps its own reference (IOAudioDevice::audioEngines)
    return true;
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
    e2->SetupFromDevice((fMode & 0x10) ? 12 : 6, (fMode & 0x08) ? 16 : 10, 1, true);
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
    engine->SetPeakProc(FWPeakProcForChannels(engine->outChannels()));
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

// ---- M-E: mixer wiring, clock source, digital signals ------------------------------------------------------------------------------

#include <IOKit/firewire/IOFireWireNub.h>

// ref GetDeviceChannels @0x1e52: the row for the engine's current rate
void com_openaudio_FWAudioDevice::GetDeviceChannels(FWDeviceChannels &out) const
{
    GetDeviceChannelsForSampleRate(fMainEngine ? fMainEngine->rateHz() : 44100, out);
}

UInt32 com_openaudio_FWAudioDevice::FetchIndexIntoSampleRateArray(const FWSampleRateEntry *rates, UInt32 rate)
{
    for (UInt32 i = 0; rates[i].whole != 0; i++) if (rates[i].whole == rate) return i;
    return 0;
}

// ref CustomMixerWriteQuads @0x22c7a: a pinned buffer, one async block write to 0xFFC7:0x00700000 + off, submitted synchronously.
// No nub -> success (the reference returns 0).
IOReturn com_openaudio_FWAVCAudioDevice::CustomMixerWriteQuads(UInt32 off, UInt32 quads, const void *data)
{
    IOFireWireNub *nub = fMetaNub ? fMetaNub->fireWireNub() : 0;
    if (!nub) return kIOReturnSuccess;
    IOBufferMemoryDescriptor *buf = IOBufferMemoryDescriptor::withCapacity((vm_size_t)quads * 4, kIODirectionInOut, false);
    if (!buf) return (IOReturn)0xE00002BD;
    IOReturn r = buf->prepare();
    if (r == kIOReturnSuccess) {
        bcopy(data, buf->getBytesNoCopy(), (size_t)quads * 4);
        FWAddress addr(kFWMixerWindowHi, kFWMixerWindowLo + off);
        IOFWWriteCommand *cmd = nub->createWriteCommand(addr, buf, 0, 0, false);
        if (!cmd) {
            r = (IOReturn)0xE00002BE;
        } else {
            r = cmd->submit();
            cmd->release();
        }
        buf->complete();
    }
    buf->release();
    // Bench diagnostics (integrated run 2026-10-16): the first writes and every failure, rate-limited. Remove once the mixer is verified.
    static UInt32 sLogged = 0;
    if (sLogged < 60 || r != kIOReturnSuccess) {
        if (sLogged < 400) {
            const UInt32 *q = static_cast<const UInt32 *>(data);
            OA_LOG("mixer write off 0x%02x quads %u first 0x%08x -> 0x%x", (unsigned)off, (unsigned)quads, quads ? (unsigned)q[0] : 0u, r);
            sLogged++;
        }
    }
    return r;
}

// ref FWAVCAudioDevice::InitializeUserSettableProperties: the generic properties, MAFirmwareInfo (copied from the nub) and MAClockSource
// (a 32-bit property whose value cell is the device mode word [+0x144], setter SetClockSource).
IOReturn com_openaudio_FWAVCAudioDevice::InitializeUserSettableProperties()
{
    IOReturn r = com_openaudio_FWAudioDevice::InitializeUserSettableProperties();
    if (r != kIOReturnSuccess) return r;
    if (fMetaNub) {
        if (OSObject *info = fMetaNub->getProperty("MAFirmwareInfo")) setProperty("MAFirmwareInfo", info);
    }
    typedef com_openaudio_FWAVCAudioDevice D;
    if (!fPropDict->AddNumber32("MAClockSource", FWBind1<D, &D::SetClockSource>(this), &fMode)) return (IOReturn)0xE00002BC;
    return kIOReturnSuccess;
}

// ref FWAVCAudioDevice::SetClockSource @0x23858 (decomp FWAVCAudioDevice__SetClockSource.c)
IOReturn com_openaudio_FWAVCAudioDevice::SetClockSource(UInt32 clk)
{
    com_openaudio_FWBaseEngine *e = fMainEngine;
    if (e) e->pauseAudioEngine();
    if (clk < 0x40000000) {
        IOLog("M-FW: Converting old digital bitmap (%X)\n", (unsigned)clk);
        switch (clk) {
            case 0: clk = 0x40000000; break;
            case 1: clk = 0x40000004; break;
            case 2: clk = 0x40000001; break;
            case 3: clk = 0x40000005; break;
        }
    }
    IOReturn r = 0;
    if (e) {
        const bool wasExternal = e->externalSync();
        r = SetClockSourceInternal(clk, false);                                // ref vt[0xb60]
        if (r == kIOReturnSuccess) {
            const bool external = ((SInt32)clk < 0) ? ((clk & 0xf) != 0) : ((clk & 3) - 1 < 2);
            if (external != wasExternal) e->SetExternalSyncEnabled(external);   // ref engine vt[0xce8]
            e->CompletePendingStateChanges();
        }
    }
    if (e) e->resumeAudioEngine();
    return r;
}

// ref FWAVCAudioDevice::SetClockSourceInternal @0x23148 (FW1814Device::MapChannelAndFunctionBlock case 4: selector 4, value = clk bit 2)
IOReturn com_openaudio_FWAVCAudioDevice::SetClockSourceInternal(UInt32 clk, bool)
{
    if (clk < 0x40000000) return (IOReturn)0xE00002C2;
    FWAvcSelectorCommand(fAvcUnit, 4, (clk & 4) ? 1 : 0);
    return kIOReturnSuccess;
}

// ref FW1814Device::InitializeUserSettableProperties @0xe6ca: the AVC-level properties, then the mixer (groups, settings, batch writers, A/B switch)
IOReturn com_openaudio_FW1814Device::InitializeUserSettableProperties()
{
    IOReturn r = com_openaudio_FWAVCAudioDevice::InitializeUserSettableProperties();
    if (r != kIOReturnSuccess) return r;
    fStereoPairProp = OSDynamicCast(com_openaudio_FWNumber32Prop, fPropDict->CopyPropertyObject("MAEnableStereoPairEngines"));
    fMixer = OSTypeAlloc(com_openaudio_FW1814Mixer);
    if (!fMixer || !fMixer->Init(this, fPropDict)) return (IOReturn)0xE00002C2;
    return kIOReturnSuccess;
}

// ref FWAVCAudioDevice::ResendCurrentMixerSettings @0x23af8 (decomp FWAVCAudioDevice__ResendCurrentMixerSettings.c): resends closer than 1 s
// together are throttled (log + 1 s sleep), then the clock source is re-asserted through its property functor. The reference measures the gap
// with the LOW 32 BITS of the uptime (int subtraction + abs): kept, so a gap near 2^32 ticks is mis-read as "too often" exactly as there.
void com_openaudio_FWAVCAudioDevice::ResendCurrentMixerSettings()
{
    if (HasSoftwareControls()) {
        AbsoluteTime now; clock_get_uptime(&now);
        SInt32 d = (SInt32)((UInt32)now - (UInt32)fLastResend);
        UInt32 ad = (UInt32)((d + (d >> 31)) ^ (d >> 31));
        UInt64 ns; absolutetime_to_nanoseconds((UInt64)(SInt64)(SInt32)ad, &ns);
        if (ns < 1000000000ULL) {
            IOLog("M-FW: Trying to resend mixer settings too often (within %d second of previous resend)\n", 1);
            IOSleep(1000);
        }
        SetClockSource(fMode);                                                    // ref [+0x150] functor
    }
    AbsoluteTime t; clock_get_uptime(&t);
    fLastResend = t;
}

void com_openaudio_FW1814Device::ResendCurrentMixerSettings()                                 // ref @0xd014
{
    com_openaudio_FWAVCAudioDevice::ResendCurrentMixerSettings();
    if (fMixer) fMixer->ResendCurrentMixerSettings();
}
void com_openaudio_FW1814Device::ResetToFactorySettings()
{
    if (fMixer) fMixer->ResetToFactorySettings();
    fMode = 0x40000003;                                                          // ref @0xd190
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
                    if (fMixer) fMixer->levels()->SendToDevice();
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
    e->SetPeakProc(FWPeakProcForChannels(e->outChannels()));
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
    if (fMixer) fMixer->DetachAudioControls(old);
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
    e->SetPeakProc(FWPeakProcForChannels(e->outChannels()));
    ClearPeakMeters();
    ResetRearrangementProcs(e);
    SetChannelNamesForMainEngine(e);
    if (fMixer) fMixer->EstablishAudioControls();
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

// ---- bus reset, hardware control poll ----------------------------------------------------------------------------------------------

#include "FWBusReset.h"
#include "FWHsci.h"

static void WakeAfterSleepThread(void *arg)
{
    com_openaudio_FWAudioDevice *d = static_cast<com_openaudio_FWAudioDevice *>(arg);
    d->SystemWokeUp();
    d->release();
}

// ref FWAudioDevice::performPowerStateChange @0x1e96 (decomp FWAudioDevice__performPowerStateChange.c)
IOReturn com_openaudio_FWAudioDevice::performPowerStateChange(IOAudioDevicePowerState oldState, IOAudioDevicePowerState newState, UInt32 *us)
{
    *us = 0;
    if (oldState == kIOAudioDeviceSleep && newState > kIOAudioDeviceSleep) fWokeFromSleep = true;
    else if (oldState > kIOAudioDeviceSleep && newState == kIOAudioDeviceSleep) SystemWillSleep();
    return kIOReturnSuccess;
}

// ref FWAVCAudioDevice::SystemWillSleep @0x22b16: park the hardware in the blank-slate clock state, stop the poll, pause, drop the connections
void com_openaudio_FWAVCAudioDevice::SystemWillSleep()
{
    SetBlankSlateClockSource();
    if (!fMainEngine) return;
    PauseReceptionOfControlPackets();
    fMainEngine->pauseAudioEngine();
    if (fMainEngine->externalSync()) fMainEngine->SetExternalSyncEnabled(false);
    fMainEngine->DisconnectAllPlugs();
}

// ref FWAVCAudioDevice::SystemWokeUp @0x22d76
void com_openaudio_FWAVCAudioDevice::SystemWokeUp()
{
    IOSleep(500);
    if (!fMainEngine) return;
    ResendCurrentMixerSettings();
    SetBlankSlateClockSource();
    fMainEngine->ConnectAllPlugs();
    ResumeReceptionOfControlPackets();
    fMainEngine->resumeAudioEngine();
}

// ref FWAudioDevice::message @0x3344: after the device is ready, "service resumed" (the FireWire bus reset is over) triggers BusReset();
// when it follows a system sleep the reference instead runs WakeAfterSleep on a new thread (power management, a later item).
// Every other message is swallowed (the reference returns 0 without calling the superclass).
IOReturn com_openaudio_FWAudioDevice::message(UInt32 type, IOService *, void *)
{
    if (fReady && type == kIOMessageServiceIsResumed) {
        if (!fWokeFromSleep) BusReset();                                      // ref vt[0xb20]
        else {                                                                // ref: WakeAfterSleep (= SystemWokeUp) on a new thread
            retain();
            if (!FWRunOnNewThread(&WakeAfterSleepThread, this)) release();
            fWokeFromSleep = false;
        }
    }
    return kIOReturnSuccess;
}

void com_openaudio_FWAVCAudioDevice::BusReset()
{
    if (fConnMgr) fConnMgr->BusReset();                                       // ref FWAudioDevice::BusReset @0x1d3c: [+0xf0]->vt[0x130]
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

static void HsciPacketThunk(void *refcon, const UInt32 *packet)
{
    static_cast<com_openaudio_FW1814Device *>(refcon)->ReadHSCIData(packet);
}

// ref StartReceptionOfControlPackets @0xe064: an FWHSCI of GetControlPacketSize() bytes with ReadHSCIData as callback, started on the work loop
bool com_openaudio_FW1814Device::StartReceptionOfControlPackets()
{
    if (!fHsci) {
        IOFireWireNub *nub = fMetaNub ? fMetaNub->fireWireNub() : 0;
        fHsci = OSTypeAlloc(com_openaudio_FWHsci);
        if (!fHsci || !fHsci->Init(nub, GetControlPacketSize(), &HsciPacketThunk, this)) {
            if (fHsci) { fHsci->release(); fHsci = 0; }
            return false;
        }
    }
    return fHsci->Start(getWorkLoop());
}

void com_openaudio_FW1814Device::StopReceptionOfControlPackets()   { if (fHsci) fHsci->Stop();   ClearPeakMeters(); }
void com_openaudio_FW1814Device::PauseReceptionOfControlPackets()  { if (fHsci) fHsci->Pause();  ClearPeakMeters(); }
void com_openaudio_FW1814Device::ResumeReceptionOfControlPackets() { if (fHsci) fHsci->Resume(); }

// ref FW1814Device::stop @0xd9ce
void com_openaudio_FW1814Device::stop(IOService *provider)
{
    if (fHsci) { fHsci->Release(); fHsci->release(); fHsci = 0; }
    com_openaudio_FWRev1AudioDevice::stop(provider);
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
        if (((fMixer ? fMixer->auxMonitorByte3() : 0) & 4) == 0) { m[22] = sw[42]; m[23] = sw[43]; }
        else                                                              { m[22] = m[36]; m[23] = m[37]; }
        for (UInt32 k = 0; k < 8; k++) m[24 + k] = sw[44 + k];
    } else {
        for (UInt32 k = 0; k < 4; k++) { const UInt32 v = sw[38 + k] & 0xffff; m[18 + k] = v; m[38 + k] = v; }
    }
    SInt32 delta = 0;
    if (fMixer) {
        const UInt32 r0 = (w0 >> 24) & 3, r1 = (w0 >> 16) & 3, r2 = (w0 >> 8) & 3;
        delta = (r0 == 1) ? 1 : (r0 == 2) ? -1 : 0;
        if (delta) fMixer->rotary()->UserRotatedHardwareKnob(delta);
        for (int which = 0; which < 2; which++) {
            const UInt32 r = which == 0 ? r1 : r2;
            delta = (r == 1) ? 1 : (r == 2) ? -1 : 0;
            if (!delta) continue;
            OSArray *arr = 0;
            fMixer->rotary()->SetPairLevelsFromRotary("MAHeadphoneLevels", which == 0 ? 1 : 0, arr, delta);
            if (arr) { fPropDict->SetPropertyAndNotifyUserClients("MAHeadphoneLevels", arr); arr->release(); }
        }
        if ((w0 & 3) != 0) fMixer->abSwitch()->MomentarySwitchPressed();
    }
}

// ---- M-J: stereo-pair engines ------------------------------------------------------------------------------------------------------

#include "FWPairEngine.h"

// ref SetExportStereoPairEngines @0x1dd6: with an engine, 0 removes every pair engine and anything else creates them
IOReturn com_openaudio_FWAudioDevice::SetExportStereoPairEngines(UInt32 on)
{
    if (!fMainEngine) return kIOReturnSuccess;
    if (on == 0) fMainEngine->RemoveAllSubEngines();
    else CreateStereoPairSubEngines();
    return kIOReturnSuccess;
}

// ref CreateStereoPairSubEngines @0x30f2 (decomp FWAudioDevice__CreateStereoPairSubEngines.c). One pair engine per (signal type, pair index
// inside its group); an input and an output group of the same (type, index) share the engine. Engine bit numbers count up for EVERY pair
// visited (also when the engine already exists), starting at 1 -- bit 0 is the main engine. The channel offset of a pair is (first pair * 2 - 2)
// plus 2 per further pair of its group. Names (ref NameAndActivatePairEngines): "<device> <Analog|S/PDIF|ADAT> <2n-1>/<2n>[ In| Out]", where
// the suffix is present only for an engine with a single direction.
void com_openaudio_FWAudioDevice::CreateStereoPairSubEngines()
{
    if (!fMainEngine) return;
    FWDeviceChannels row;
    GetDeviceChannels(row);
    struct Trio { UInt32 type, idx; com_openaudio_FWPairEngine *engine; };
    Trio trios[16]; unsigned nTrios = 0;
    UInt32 bitIndex = 1;
    for (UInt32 g = 0; g < 5 && row.group[g][3] != 3; g++) {
        const bool input = row.group[g][0] != 0;
        const UInt32 type = row.group[g][3];
        UInt32 chan = row.group[g][1] * 2 - 2;
        UInt32 idx = 1;
        for (UInt32 pair = row.group[g][1]; pair <= row.group[g][2]; pair++, idx++, bitIndex++, chan += 2) {
            com_openaudio_FWPairEngine *pe = 0;
            for (unsigned k = 0; k < nTrios; k++) if (trios[k].type == type && trios[k].idx == idx) { pe = trios[k].engine; break; }
            if (!pe) {
                if (nTrios >= 16) return;
                pe = OSTypeAlloc(com_openaudio_FWPairEngine);
                if (!pe) return;
                if (!pe->init(0)) { pe->release(); return; }
                pe->SetMasterEngine(fMainEngine);
                pe->SetIndex(bitIndex);
                fMainEngine->AddSubEngine(pe);                                  // the master's set keeps it alive
                pe->release();
                trios[nTrios].type = type; trios[nTrios].idx = idx; trios[nTrios].engine = pe; nTrios++;
            }
            pe->AddDirection(!input, chan);
        }
    }
    for (unsigned k = 0; k < nTrios; k++) {
        com_openaudio_FWPairEngine *pe = trios[k].engine;
        const char *tname = (trios[k].type == 1) ? "S/PDIF" : (trios[k].type == 2) ? "ADAT" : "Analog";   // ref kSPDIFString / kADATString / kAnalogString @0x2da8f..
        char name[256];
        snprintf(name, sizeof(name), "%s %s %d/%d%s", GetDescriptionString(), tname, (int)(trios[k].idx * 2 - 1), (int)(trios[k].idx * 2),
                 !pe->hasInput() ? " Out" : !pe->hasOutput() ? " In" : "");
        pe->setDescription(name);
        activateAudioEngine(pe);
    }
}
