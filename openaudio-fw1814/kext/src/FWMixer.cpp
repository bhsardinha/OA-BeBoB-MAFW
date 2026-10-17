#include "FWMixer.h"
#include "FWAudioDeviceChain.h"
#include "FWAvc.h"
#include "FWBaseEngine.h"
#include "Trace.h"

static const IOReturn kErrNoMem  = (IOReturn)0xE00002BD;
static const IOReturn kErrBadArg = (IOReturn)0xE00002C2;

OSDefineMetaClassAndStructors(com_openaudio_FW1814Mixer, OSObject)

static IOReturn BatchThunk(void *o, UInt32 off, UInt32 n, void *d) { return static_cast<com_openaudio_FW1814Mixer *>(o)->WriteQuads(off, n, d); }
static IOReturn RotatedThunk(void *o, UInt32 off, UInt32 n, void *d) { return static_cast<com_openaudio_FW1814Mixer *>(o)->WriteRotatedAux(off, n, d); }

bool com_openaudio_FW1814Mixer::Init(com_openaudio_FWAVCAudioDevice *device, com_openaudio_FWPropertyDict *dict)
{
    if (!OSObject::init() || !device || !dict) return false;
    fDevice = device;
    bzero(fPan, sizeof(fPan)); fRouting = fHpSel = fAuxMon = 0;
    fLevels = OSTypeAlloc(com_openaudio_FWSettingsLevels);
    fAux    = OSTypeAlloc(com_openaudio_FWSettingsAuxSend);
    fRotary = OSTypeAlloc(com_openaudio_FWSettingsRotaryControl);
    fAB     = OSTypeAlloc(com_openaudio_FWSettingsABSwitch);
    if (!fLevels || !fAux || !fRotary || !fAB) return false;

    // order of the reference: Rotary, AuxSend, Levels, batch writers, A/B switch
    if (fRotary->Init(this, dict, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut) != kIOReturnSuccess) return false;
    if (fAux->Init(this, dict, &fAuxSend, &fAuxOut, (fOutput.Count() >> 1) + 1) != kIOReturnSuccess) return false;
    if (fLevels->Init(this, dict, &fSWReturn, &fOutput, &fInput, &fHeadphone, &fAuxOut) != kIOReturnSuccess) return false;

    FWCallBatch plain;   plain.obj = this;   plain.fn = &BatchThunk;
    FWCallBatch rotated; rotated.obj = this; rotated.fn = &RotatedThunk;
    fOutput.SetupBatch(plain, kFWMixRegOutput);
    fInput.SetupBatch(plain, kFWMixRegInput);
    fSWReturn.SetupBatch(plain, kFWMixRegSWReturn);
    fHeadphone.SetupBatch(plain, kFWMixRegHeadphone);
    fAuxSend.SetupBatch(rotated, kFWMixRegAuxSend);                              // ref stores 0x64 / 0x80 / rotated writer at +0x1838/+0x183c/+0x1830
    fAuxOut.SetupBatch(plain, kFWMixRegAuxOutput);

    if (fAB->Init(this, dict) != kIOReturnSuccess) return false;
    fAB->SetLEDFunctor(FWBind1<com_openaudio_FW1814Mixer, &com_openaudio_FW1814Mixer::SetLEDStatus>(this));
    fAB->SetHeadphoneFunctor(FWBind2<com_openaudio_FW1814Mixer, &com_openaudio_FW1814Mixer::SetHeadphoneSelector>(this));
    fAB->UpdateLEDs();
    return true;
}

void com_openaudio_FW1814Mixer::free()
{
    if (fLevels) { fLevels->release(); fLevels = 0; }
    if (fAux)    { fAux->release();    fAux = 0; }
    if (fRotary) { fRotary->release(); fRotary = 0; }
    if (fAB)     { fAB->release();     fAB = 0; }
    OSObject::free();
}

// ref FW1814Device::ResendCurrentMixerSettings (derived part): ABSwitch, Rotary, AuxSend, Levels
void com_openaudio_FW1814Mixer::ResendCurrentMixerSettings()
{
    fAB->SendToDevice();
    // F4 (clone addition that wrote the headphone selectors here) REVERTED 2026-10-16: the reference leaves Reg 0x98 unwritten until a monitor
    // source is chosen in the panel, so the phones stay silent at start-up. The phone levels start at 0 dB (full scale) and are dangerously loud;
    // the user sets them in the panel first. Do not re-add.
    fRotary->SendToDevice();
    fAux->SendToDevice();
    fLevels->SendToDevice();
}

void com_openaudio_FW1814Mixer::ResetToFactorySettings()
{
    fAB->ResetToFactorySettings();
    fRotary->ResetToFactorySettings();
    fAux->ResetToFactorySettings();
    fLevels->ResetToFactorySettings();
}

IOReturn com_openaudio_FW1814Mixer::WriteQuads(UInt32 off, UInt32 quads, void *data)
{
    return fDevice->CustomMixerWriteQuads(off, quads, data);
}

// ref CustomMixerWriteWithRotatedAuxChannels @0xe982: {in[9], in[10], in[0..8]} -- the last 8 bytes of the 44-byte block go first
IOReturn com_openaudio_FW1814Mixer::WriteRotatedAux(UInt32 off, UInt32 quads, void *data)
{
    UInt8 rot[44];
    const UInt8 *in = static_cast<const UInt8 *>(data);
    bcopy(in + 0x24, rot, 8);
    bcopy(in, rot + 8, 0x24);
    return WriteQuads(off, quads, rot);
}

// ref FW1814Device::SetInputPan @0xdd3e (decomp FW1814Device__SetInputPan.c). Channels 1..18 -> pan word; each register quadlet carries the
// words of channels (2k+1, 2k+2) in memory order.
IOReturn com_openaudio_FW1814Mixer::SetInputPan(UInt32 ch, UInt32 pan)
{
    const UInt32 idx = ch - 1;
    if (idx >= 18) return kErrBadArg;
    fPan[idx] = (UInt16)((((UInt32)(0 - pan) >> 1) & 0xff) + (UInt32)((SInt32)(SInt16)pan * -0x8000));
    const UInt32 base = idx & ~1u;                                              // first word of the pair
    return WriteQuads(kFWMixRegPan + base * 2, 1, &fPan[base]);
}

static inline UInt32 Bit(UInt32 n) { return OSSwapInt32(1u << (n & 0x1f)); }
static inline UInt32 Put(UInt32 shadow, UInt32 bit, bool on) { return on ? (shadow | bit) : (shadow & ~bit); }

// ref FW1814Device::SetInputRouting @0xdbea (decomp FW1814Device__SetInputRouting.c): route bit 0 / bit 1 of the value per channel
IOReturn com_openaudio_FW1814Mixer::SetInputRouting(UInt32 ch, UInt32 route)
{
    if (ch == 0 || ch > (fInput.Count() >> 1)) return kErrBadArg;
    if (ch < 5) {
        fRouting = Put(fRouting, Bit(ch - 1), route & 1);
        fRouting = Put(fRouting, Bit(ch + 3), route & 2);
    } else if (ch == 5) {
        fRouting = Put(fRouting, 0x100, route & 1);                             // raw host bits 8 / 9 (= wire bits 16 / 17)
        fRouting = Put(fRouting, 0x200, route & 2);
    } else if (ch < 10) {
        fRouting = Put(fRouting, Bit(ch + 2), route & 1);
        fRouting = Put(fRouting, Bit(ch + 6), route & 2);
    }
    return WriteQuads(kFWMixRegInputRouting, 1, &fRouting);
}

// ref FW1814Device::SetAuxMonitor @0xcfc4
IOReturn com_openaudio_FW1814Mixer::SetAuxMonitor(UInt32 bus, UInt32 on)
{
    fAuxMon = Put(fAuxMon, Bit(bus - 1), on != 0);
    return WriteQuads(kFWMixRegAuxMonitor, 1, &fAuxMon);
}

// ref FW1814Device::SetHeadphoneSelector @0xd1f8 (decomp FW1814Device__SetHeadphoneSelector.c, asm 0xd272..0xd296): clear the pair's old selector
// bits (the second assignment of the reference replaces the first, it does not accumulate), then set the bit for `which` (0..2)
IOReturn com_openaudio_FW1814Mixer::SetHeadphoneSelector(UInt32 pair, UInt32 which)
{
    UInt32 v = fHpSel & 0xfffff8ffu;
    if (pair == 0) v = fHpSel & 0xf8ffffffu;
    fHpSel = v;
    if (which <= 2) {
        const UInt32 a = (pair == 0) ? (1u << (0x18 + which)) : 0;
        const UInt32 b = (pair != 0) ? (1u << (8 + which)) : 0;
        fHpSel = (a + b) | v;
    }
    return WriteQuads(kFWMixRegHeadphoneSel, 1, &fHpSel);
}

// ref FW1814Device::AVCControlSetLEDStatus @0xdaaa: 8-byte vendor CONTROL 00 FF 00 03 00 01 [state != 1] 00 (asm 0xdaf0..0xdb72)
IOReturn com_openaudio_FW1814Mixer::SetLEDStatus(UInt32 state)
{
    com_openaudio_FWAvcUnit *unit = fDevice->avcUnit();
    if (!unit) return (IOReturn)0xE00002BC;
    const UInt8 cmd[8] = { 0x00, 0xFF, 0x00, 0x03, 0x00, 0x01, (UInt8)(state != 1), 0x00 };
    UInt8 resp[8]; UInt32 rl = 8;
    return unit->AVCCommandWithErrorResponse(cmd, 8, resp, &rl);
}

// ref FWSettingsLevels::SetAVCRouting + FW1814Device::MapRoutingParameters (-> {function block 1, b 0}) + AVCMixerConnectPair
IOReturn com_openaudio_FW1814Mixer::SetAvcRouting(bool connect, UInt8 srcPair, UInt8 dstPair)
{
    return FWAvcMixerConnectPair(fDevice->avcUnit(), connect, 1, 0, srcPair, dstPair);
}

// ref SetAuxOutChannelFeature: MapChannelAndFunctionBlock(type 7) -> function block 9, channel = the aux-out channel number
IOReturn com_openaudio_FW1814Mixer::SetAuxOutChannelFeature(UInt32 channel, UInt8 controlSelector, UInt16 value)
{
    return FWAvcFeatureFunctionBlock(fDevice->avcUnit(), 9, controlSelector, (UInt8)channel, value);
}

IOAudioEngine *com_openaudio_FW1814Mixer::engine() const { return fDevice ? fDevice->mainEngine() : 0; }
