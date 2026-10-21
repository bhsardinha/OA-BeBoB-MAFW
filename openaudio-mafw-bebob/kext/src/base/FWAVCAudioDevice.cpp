// FWAVCAudioDevice module (base) -- see FWAVCAudioDevice.h for what the reference does in this layer.
#include "FWAVCAudioDevice.h"
#include "FWBaseEngine.h"
#include "FWMetaNub.h"
#include "FWAvc.h"
#include "FWAvcFrames.h"
#include "FWCMP.h"
#include "FWHsci.h"
#include "FWPropertyDict.h"
#include "FWRegisterWindows.h"
#include "Trace.h"

#define super com_openaudio_FWAudioDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWAVCAudioDevice, com_openaudio_FWAudioDevice)
#undef super

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
    OA_LOG("%s initHardware: up", getMetaClass()->getClassName());
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
    if (fHsci) { fHsci->Release(); fHsci->release(); fHsci = 0; }            // ref FW1814 stop @0xd9ce / Rev2 stop: HSCI first
    com_openaudio_FWAudioDevice::stop(provider);
    ReleaseConnectionManager();
}

void com_openaudio_FWAVCAudioDevice::free()
{
    ReleaseConnectionManager();
    fMetaNub = 0;
    com_openaudio_FWAudioDevice::free();
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

// ref FWAVCAudioDevice::SetClockSourceInternal @0x23148: map kind 4 with the clock word (FW1814: selector function block 4, value = clk bit 2),
// then the AV/C selector command. The reference returns the selector's result; the clone keeps 0.1.0's success (the async queue only
// fails before the unit has started).
IOReturn com_openaudio_FWAVCAudioDevice::SetClockSourceInternal(UInt32 clk, bool)
{
    if (clk < 0x40000000) return (IOReturn)0xE00002C2;
    FunctionBlockMapping m = { 4, 0, 0, clk };
    IOReturn r = MapChannelAndFunctionBlock(m);
    if (r != kIOReturnSuccess) return r;
    AVCSelectorCommand(m.fb, m.ch);
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWAVCAudioDevice::AVCSelectorCommand(UInt8 fb, UInt8 value)
{
    return FWAvcSelectorCommand(fAvcUnit, fb, value);
}

IOReturn com_openaudio_FWAVCAudioDevice::AVCControlFeatureFunctionBlock(UInt8 fb, UInt8 controlSelector, UInt8 channel, UInt16 value)
{
    return FWAvcFeatureFunctionBlock(fAvcUnit, fb, controlSelector, channel, value);
}

IOReturn com_openaudio_FWAVCAudioDevice::AVCMixerConnectPair(bool connect, UInt8 fb, UInt8 plug, UInt8 inPair, UInt8 outPair)
{
    return FWAvcMixerConnectPair(fAvcUnit, connect, fb, plug, inPair, outPair);
}

IOReturn com_openaudio_FWAVCAudioDevice::AVCControlFunctionBlock(UInt8 type, UInt8 fb, UInt8 *frame, UInt8 len)
{
    FWAvcFunctionBlockHeader(frame, type, fb);
    return fAvcUnit ? fAvcUnit->AVCCommandAsync(frame, len) : kIOReturnNotReady;     // ref vt[0x8b0], response ignored
}

IOReturn com_openaudio_FWAVCAudioDevice::AVCVendorCommand(UInt8 a, UInt8 b, UInt32 n, const UInt8 *data)
{
    UInt8 cmd[kFWAvcVendorLen], resp[kFWAvcVendorLen];
    if (!FWAvcVendorFrame(cmd, a, b, n, data)) return (IOReturn)0xE00002C2;
    if (!fAvcUnit) return kIOReturnNotReady;
    bzero(resp, sizeof(resp));
    UInt32 rl = kFWAvcVendorLen;
    return fAvcUnit->AVCCommandWithErrorResponse(cmd, kFWAvcVendorLen, resp, &rl);     // ref vt[0x8b8]
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

void com_openaudio_FWAVCAudioDevice::BusReset()
{
    if (fConnMgr) fConnMgr->BusReset();                                       // ref FWAudioDevice::BusReset @0x1d3c: [+0xf0]->vt[0x130]
}

// the HSCI read callback (ref functor tag 0xBB9 on Rev2, the FW1814's own on @0xe064): the model's ReadHSCIData
static void HsciPacketThunk(void *refcon, const UInt32 *packet)
{
    static_cast<com_openaudio_FWAVCAudioDevice *>(refcon)->ReadHSCIData(packet);
}

// ref FW1814 StartReceptionOfControlPackets @0xe064 = Rev2 @0x13138 (see the header)
bool com_openaudio_FWAVCAudioDevice::StartReceptionOfControlPackets()
{
    if (GetControlPacketSize() == 0) return true;                              // clone: a model without a status packet polls nothing
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

void com_openaudio_FWAVCAudioDevice::StopReceptionOfControlPackets()   { if (fHsci) fHsci->Stop();   ClearPeakMeters(); }

void com_openaudio_FWAVCAudioDevice::PauseReceptionOfControlPackets()  { if (fHsci) fHsci->Pause();  ClearPeakMeters(); }

void com_openaudio_FWAVCAudioDevice::ResumeReceptionOfControlPackets() { if (fHsci) fHsci->Resume(); }

// ref FWAVCAudioDevice::SetBlankSlateClockSource @0x22de8 (decomp FWAVCAudioDevice__SetBlankSlateClockSource.c)
IOReturn com_openaudio_FWAVCAudioDevice::SetBlankSlateClockSource()
{
    IOReturn r = kIOReturnSuccess;
    if (fMainEngine) r = SetClockSourceInternal(0x40000000, true);             // ref vt[0xb60]
    IOSleep(2500);                                                            // ref IOSleep(0x9c4)
    return r;
}
