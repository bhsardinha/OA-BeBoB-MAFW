#include "FWSettings.h"
#include "FWFrontPanelLaw.h"
#include "FWAudioDeviceChain.h"
#include "FWBaseEngine.h"
#include "Trace.h"
#include <libkern/libkern.h>

static const IOReturn kErrBadArg   = (IOReturn)0xE00002C2;
static const IOReturn kErrNoMem    = (IOReturn)0xE00002BD;
static const SInt32 kFWMinLevel = (SInt32)0xffffff01;        // -255 (0.5 dB steps) -- group [+0x14]

OSDefineMetaClassAndAbstractStructors(com_openaudio_FWSettings, OSObject)
OSDefineMetaClassAndStructors(com_openaudio_FWSettingsLevels, com_openaudio_FWSettings)
OSDefineMetaClassAndStructors(com_openaudio_FWSettingsAuxSend, com_openaudio_FWSettings)
OSDefineMetaClassAndStructors(com_openaudio_FWSettingsRotaryControl, com_openaudio_FWSettings)
OSDefineMetaClassAndStructors(com_openaudio_FWSettingsABSwitch, com_openaudio_FWSettings)

typedef com_openaudio_FWAVCAudioDevice::FunctionBlockMapping FWMap;
typedef com_openaudio_FWAVCAudioDevice::Routing FWRoute;
static const IOReturn kErrMapSkip = (IOReturn)0xE00002D7;   // a map result the reference treats as success (SetAVCRouting, SetAuxSendLevel)

// =====================================================================================================================================
// Levels
// =====================================================================================================================================

// ref FWSettingsLevels::InitGroupProp @ decomp FWSettingsLevels__InitGroupProp.c: Levels (field 0), Mute (8), [Solo (0x10)], Link (0x18)
IOReturn com_openaudio_FWSettingsLevels::InitGroupProp(const char *name, int slot, FWChannelGroup *g, bool solo, FWChannelGroup *soloSecond)
{
    char key[64];
    snprintf(key, sizeof(key), "MA%sLevels", name);
    if (!(fGroupProp[slot][0] = fDict->AddChannelGroup(key, g, 0, 0))) return kErrNoMem;
    snprintf(key, sizeof(key), "MA%sMute", name);
    if (!(fGroupProp[slot][1] = fDict->AddChannelGroup(key, g, 8, 0))) return kErrNoMem;
    if (solo) {
        snprintf(key, sizeof(key), "MA%sSolo", name);
        if (!(fGroupProp[slot][2] = fDict->AddChannelGroup(key, g, 0x10, soloSecond))) return kErrNoMem;
    }
    snprintf(key, sizeof(key), "MA%sLink", name);
    if (!(fGroupProp[slot][3] = fDict->AddChannelGroup(key, g, 0x18, 0))) return kErrNoMem;
    return kIOReturnSuccess;
}

// ref FWSettingsLevels::Init (decomp FWSettingsLevels__Init.c): limits, per-channel setter and properties per group. A model with a register
// mixer (FW1814) installs batch writers afterwards. The CoreAudio controls are created below, as in the reference.
IOReturn com_openaudio_FWSettingsLevels::Init(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *swReturn,
                                              FWChannelGroup *output, FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut)
{
    typedef com_openaudio_FWSettingsLevels L;
    com_openaudio_FWPropertyDict *dict = device->propDict();
    fDevice = device; fDict = dict;
    fSWReturn = swReturn; fOutput = output; fInput = input; fHeadphone = headphone; fAuxOut = auxOut;
    bzero(fGroupProp, sizeof(fGroupProp));
    fPan = fInputRouting = fSWReturnRouting = 0;
    fDirectMon = fGlobalMute = fGlobalDim = 0;
    IOReturn r;

    if (swReturn) {
        swReturn->SetLimits(kFWMinLevel, 0);
        swReturn->SetPerChannel(FWBind2<L, &L::SetSWReturnLevel>(this));
        if ((r = InitGroupProp("SWReturn", 0, swReturn, true, input)) != kIOReturnSuccess) return r;
        fSWReturnRouting = dict->AddArray("MASWReturnRoutingV2", swReturn->Count() >> 1, FWBind2<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetSWReturnRouting>(this));
        if (!fSWReturnRouting) return kErrNoMem;
    }
    if (output) {
        output->SetLimits(kFWMinLevel, 0);
        output->SetPerChannel(FWBind2<L, &L::SetOutputLevel>(this));
        if ((r = InitGroupProp("Output", 1, output, true, 0)) != kIOReturnSuccess) return r;
    }
    if (input) {
        input->SetLimits(kFWMinLevel, 0);
        input->SetPerChannel(FWBind2<L, &L::SetInputLevel>(this));
        if ((r = InitGroupProp("Input", 2, input, true, swReturn)) != kIOReturnSuccess) return r;
        fPan = dict->AddArray("MAInputPan2", input->Count(), FWBind2<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetInputPan>(this));
        if (!fPan) return kErrNoMem;
        fDirectMon = dict->AddNumber32("MADirectMonitoringActive", FWBind1<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetDirectMonitoringActive>(this));
        if (!fDirectMon) return kErrNoMem;
        fDirectMon->set(1);                                                       // ref *(this+0x11c) = 1
        fInputRouting = dict->AddArray("MAInputRoutingV2", input->Count() >> 1, FWBind2<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetInputRouting>(this));
        if (!fInputRouting) return kErrNoMem;
    }
    if (headphone) {
        headphone->SetLimits(kFWMinLevel, 0);
        headphone->SetPerChannel(FWBind2<L, &L::SetHeadphonesLevel>(this));
        if ((r = InitGroupProp("Headphone", 3, headphone, false, 0)) != kIOReturnSuccess) return r;
    }
    if (swReturn || input) fUnionIn.Init(swReturn, input, 0);
    if (output || auxOut || headphone) fUnionOut.Init(output, auxOut, headphone);
    if (output) {
        fGlobalDim = dict->AddNumber32("MAGlobalDim", FWBind1<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetGlobalDim>(this));
        if (!fGlobalDim) return kErrNoMem;
        fGlobalMute = dict->AddNumber32("MAGlobalMute", FWBind1<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetGlobalMute>(this));
        if (!fGlobalMute) return kErrNoMem;
    }
    EstablishAudioControls();                                                   // ref: created inside Init; factored out for the M-S re-publish
    return kIOReturnSuccess;
}

void com_openaudio_FWSettingsLevels::EstablishAudioControls()
{
    // CoreAudio controls (ref FWSettingsLevels::Init): output master volume + one per channel + per-channel mutes, input volumes + mutes,
    // and the global mute as an output mute control. Range -0x80..0 in 0.5 dB steps = -64..0 dB.
    if (IOAudioEngine *eng = fDevice->mainEngine()) {                           // the device's main engine (ref device [+0xe8])
        const UInt32 kOutp = kIOAudioControlUsageOutput, kInpt = kIOAudioControlUsageInput;
        if (fOutput) {
            fGroupProp[1][0]->EstablishVolumeControl(eng, kOutp, 0, 0, -0x80, 0, -0x40, 0);
            fGroupProp[1][0]->EstablishVolumeControls(eng, kOutp, 1, -0x80, 0, -0x40, 0);
            fGroupProp[1][1]->EstablishMuteControls(eng, kOutp, 1);
        }
        if (fInput) {
            fGroupProp[2][0]->EstablishVolumeControls(eng, kInpt, 1, -0x80, 0, -0x40, 0);
            // M-U (user decision 2026-10-17): the v1.9 pass-through ("play-through") controls. NOT in v1.11.2 (the clone's spec): v1.9
            // FWSettingsLevels::Init asm 0x23cad creates a second set of input-level volume controls with usage 'pass' on the same array property,
            // and 0x23d84 a 'pass' master mute (channel 0) on MADirectMonitoringActive (value stored inverted, see ChangedValueThroughAudioControl).
            // Evidence: SL registry (capture 81) 65 controls = 46 + 18 'pass' level + 1 'pass' mute.
            fGroupProp[2][0]->EstablishVolumeControls(eng, kIOAudioControlUsagePassThru, 1, -0x80, 0, -0x40, 0);
            fGroupProp[2][1]->EstablishMuteControls(eng, kInpt, 1);
            if (fDirectMon) fDirectMon->EstablishMuteControl(eng, kIOAudioControlUsagePassThru, 0, 0);
        }
    }
    if (fGlobalMute) {
        if (IOAudioEngine *eng = fDevice->mainEngine()) fGlobalMute->EstablishMuteControl(eng, kIOAudioControlUsageOutput, 0, 0);
    }
}

void com_openaudio_FWSettingsLevels::DetachAudioControls(IOAudioEngine *engine)
{
    for (int g = 0; g < 4; g++)
        for (int k = 0; k < 4; k++)
            if (fGroupProp[g][k]) fGroupProp[g][k]->DetachAudioControlsFromEngine(engine);
    if (fDirectMon)  fDirectMon->DetachAudioControlsFromEngine(engine);
    if (fGlobalMute) fGlobalMute->DetachAudioControlsFromEngine(engine);
}

// ref per-channel level setters (decomp FWSettingsLevels__Set{SWReturn,Output,Headphones}Level.c): channel 1..Count() of the group, map
// {kind, ch} -> {function block, channel}, AV/C feature block control selector 2 (volume), value << 7
static IOReturn FeatureLevel(com_openaudio_FWAVCAudioDevice *dev, FWChannelGroup *g, UInt32 kind, UInt32 ch, UInt32 v)
{
    if (!g || ch == 0 || ch > g->Count()) return kErrBadArg;
    FWMap m = { kind, 0, (UInt8)ch, 0 };
    IOReturn r = dev->MapChannelAndFunctionBlock(m);
    if (r != kIOReturnSuccess) return r;
    return dev->AVCControlFeatureFunctionBlock(m.fb, 2, m.ch, (UInt16)(v << 7));
}
IOReturn com_openaudio_FWSettingsLevels::SetSWReturnLevel(UInt32 ch, UInt32 v)   { return FeatureLevel(fDevice, fSWReturn, 3, ch, v); }
IOReturn com_openaudio_FWSettingsLevels::SetOutputLevel(UInt32 ch, UInt32 v)     { return FeatureLevel(fDevice, fOutput, 2, ch, v); }
IOReturn com_openaudio_FWSettingsLevels::SetHeadphonesLevel(UInt32 ch, UInt32 v) { return FeatureLevel(fDevice, fHeadphone, 1, ch, v); }
// ref SetInputLevel @0x21890 / SetInputPan @0x21878: through SetInputChannelFeature, selector 2 (volume) / 3 (balance, value negated)
IOReturn com_openaudio_FWSettingsLevels::SetInputLevel(UInt32 ch, UInt32 v)      { return SetInputChannelFeature(ch, 2, v); }
IOReturn com_openaudio_FWSettingsLevels::SetInputPan(UInt32 ch, UInt32 v)        { return SetInputChannelFeature(ch, 3, 0 - v); }

// ref SetInputChannelFeature @0x2254c: map kind 6, no range check, the map result is not checked (as the reference)
IOReturn com_openaudio_FWSettingsLevels::SetInputChannelFeature(UInt32 ch, UInt8 controlSelector, UInt32 v)
{
    FWMap m = { 6, 0, (UInt8)ch, 0 };
    fDevice->MapChannelAndFunctionBlock(m);
    fDevice->AVCControlFeatureFunctionBlock(m.fb, controlSelector, m.ch, (UInt16)(v << 7));
    return kIOReturnSuccess;                                                    // ref returns void
}

// ref SetAVCRouting @0x225b4: for each destination mixer i = 1..nDst, MapRoutingParameters({kind, srcPair, i}) (0xE00002D7 counts as success)
// then AVCMixerConnectPair(mask bit i-1, fb, plug, srcPair', dstMixer') with the pairs as the map left them. 0xE00002BC when nDst = 0.
IOReturn com_openaudio_FWSettingsLevels::SetAVCRouting(UInt32 kind, UInt32 nDst, UInt32 srcPair, UInt32 mask)
{
    IOReturn r = (IOReturn)0xE00002BC;
    for (UInt32 i = 0; i < nDst; i++) {
        FWRoute rt = { kind, srcPair, i + 1, 0, 0 };
        r = fDevice->MapRoutingParameters(rt);
        if (r == kErrMapSkip) r = kIOReturnSuccess;
        if (r != kIOReturnSuccess) return r;
        r = fDevice->AVCMixerConnectPair((mask >> (i & 0x1f)) & 1, rt.fb, rt.plug, (UInt8)rt.srcPair, (UInt8)rt.dstMixer);
        if (r != kIOReturnSuccess) return r;
    }
    return r;
}

// ref SetSWReturnRouting @0x218a6 (bus kind 1) / SetInputRouting @0x218f6 (bus kind 0): both use nDst = SW-return Count() / 2
IOReturn com_openaudio_FWSettingsLevels::SetSWReturnRouting(UInt32 idx, UInt32 mask)
{
    return SetAVCRouting(1, fSWReturn ? fSWReturn->Count() >> 1 : 0, idx, mask);
}
IOReturn com_openaudio_FWSettingsLevels::SetInputRouting(UInt32 idx, UInt32 mask)
{
    return SetAVCRouting(0, fSWReturn ? fSWReturn->Count() >> 1 : 0, idx, mask);
}

IOReturn com_openaudio_FWSettingsLevels::SetDirectMonitoringActive(UInt32)
{
    if (fDirectMon) fDirectMon->set(1);                                          // ref *(this+0x11c) = 1 (overwritten by the property's own store)
    if (fInput) { fInput->SetMuteAll(false); fInput->SendLevelsToDevice(0); }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWSettingsLevels::SetGlobalMute(UInt32 v)
{
    if (fGlobalMute) fGlobalMute->set(v != 0);
    if (fOutput) { fOutput->SetMuteAll(v != 0); fOutput->SendLevelsToDevice(0); }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWSettingsLevels::SetGlobalDim(UInt32 v)
{
    if (fGlobalDim) fGlobalDim->set(v != 0);
    if (fOutput) { fOutput->SetDim(v != 0); fOutput->SendLevelsToDevice(0); }
    return kIOReturnSuccess;
}

// ref FWSettingsLevels::SendToDevice (decomp FWSettingsLevels__SendToDevice.c). The pan array is resent for its first FOUR elements only
// (the reference loop); the routing arrays are resent in full.
IOReturn com_openaudio_FWSettingsLevels::SendToDevice()
{
    if (fUnionIn.Enabled())  fUnionIn.SendLevelsToDevice(0);
    if (fUnionOut.Enabled()) fUnionOut.SendLevelsToDevice(0);
    if (fHeadphone) fHeadphone->SendLevelsToDevice(0);
    if (fInput && fPan) {
        for (UInt32 i = 0; i < 4; i++) fPan->perElement()(i + 1, fPan->Get(i));
        if (fInputRouting) fInputRouting->SendAllToDevice();
    }
    if (fSWReturn && fSWReturnRouting) fSWReturnRouting->SendAllToDevice();
    return kIOReturnSuccess;
}

// ref FWSettingsLevels::ResetToFactorySettings (decomp FWSettingsLevels__ResetToFactorySettings.c)
IOReturn com_openaudio_FWSettingsLevels::ResetToFactorySettings()
{
    if (fDirectMon) fDirectMon->set(1);
    if (fInput) fInput->SetMuteAll(false);
    if (fUnionOut.Enabled()) fUnionOut.ResetAll();
    if (fUnionIn.Enabled())  fUnionIn.ResetAll();
    if (fInput) {
        if (fPan) for (UInt32 i = 0; i < fPan->count(); i += 2) { fPan->Set(i, 0xffffff01); fPan->Set(i + 1, 0xff); }
        if (fInputRouting) for (UInt32 i = 0; i < (fInput->Count() >> 1); i++) fInputRouting->Set(i, 0);
    }
    if (fSWReturn && fSWReturnRouting) for (UInt32 i = 0; i < (fSWReturn->Count() >> 1); i++) fSWReturnRouting->Set(i, (UInt32)(1ULL << (i & 0x3f)));
    return kIOReturnSuccess;
}

// =====================================================================================================================================
// AuxSend
// =====================================================================================================================================

// ref FWSettingsAuxSend::Init (decomp FWSettingsAuxSend__Init.c)
IOReturn com_openaudio_FWSettingsAuxSend::Init(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *auxSend, FWChannelGroup *auxOut, UInt32 nMonitor)
{
    typedef com_openaudio_FWSettingsAuxSend A;
    com_openaudio_FWPropertyDict *dict = device->propDict();
    fDevice = device; fDict = dict; fAuxSend = auxSend; fAuxOut = auxOut; fMonitorCount = nMonitor;
    auxSend->SetLimits(kFWMinLevel, 0);
    auxOut->SetLimits(kFWMinLevel, 0);
    auxSend->SetPerChannel(FWBind2<A, &A::SetAuxSendLevel>(this));
    auxOut->SetPerChannel(FWBind2<A, &A::SetAuxOutputLevel>(this));
    if (!(fSendLevels = dict->AddChannelGroup("MAAuxSendLevels", auxSend, 0, 0))) return kErrNoMem;
    if (!(fOutLevels  = dict->AddChannelGroup("MAAuxOutputLevels", auxOut, 0, 0))) return kErrNoMem;
    if (!(fOutMute    = dict->AddChannelGroup("MAAuxOutputMute", auxOut, 8, 0))) return kErrNoMem;
    if (!(fOutLink    = dict->AddChannelGroup("MAAuxOutputLink", auxOut, 0x18, 0))) return kErrNoMem;
    fPan = dict->AddArray("MAAuxOutputPan2", auxOut->Count(), FWBind2<com_openaudio_FWSettingsAuxSend, &com_openaudio_FWSettingsAuxSend::SetAuxPan>(this));
    if (!fPan) return kErrNoMem;
    fMonitor = dict->AddArray("MAMonitorAuxOnOutput", nMonitor, FWBind2<com_openaudio_FWSettingsAuxSend, &com_openaudio_FWSettingsAuxSend::SetAuxMonitor>(this));
    if (!fMonitor) return kErrNoMem;
    return kIOReturnSuccess;
}

// ref SetAuxSendLevel: map kind 0 (0xE00002D7 counts as success), feature selector 2, no range check, result ignored
IOReturn com_openaudio_FWSettingsAuxSend::SetAuxSendLevel(UInt32 ch, UInt32 v)
{
    FWMap m = { 0, 0, (UInt8)ch, 0 };
    IOReturn r = fDevice->MapChannelAndFunctionBlock(m);
    if (r == kErrMapSkip) r = kIOReturnSuccess;
    if (r == kIOReturnSuccess) fDevice->AVCControlFeatureFunctionBlock(m.fb, 2, m.ch, (UInt16)(v << 7));
    return kIOReturnSuccess;                                                    // ref returns void
}

// ref SetAuxOutChannelFeature (decomp FWSettingsAuxSend__SetAuxOutChannelFeature.c): channel 1..auxOut Count(), map kind 7 (FW1814: function
// block 9, channel unchanged), feature block with the given control selector, value << 7
IOReturn com_openaudio_FWSettingsAuxSend::SetAuxOutChannelFeature(UInt32 ch, UInt8 controlSelector, UInt32 v)
{
    if (ch == 0 || ch > fAuxOut->Count()) return kErrBadArg;
    FWMap m = { 7, 0, (UInt8)ch, 0 };
    IOReturn r = fDevice->MapChannelAndFunctionBlock(m);
    if (r != kIOReturnSuccess) return r;
    return fDevice->AVCControlFeatureFunctionBlock(m.fb, controlSelector, m.ch, (UInt16)(v << 7));
}
IOReturn com_openaudio_FWSettingsAuxSend::SetAuxOutputLevel(UInt32 ch, UInt32 v) { return SetAuxOutChannelFeature(ch, 2, v); }
IOReturn com_openaudio_FWSettingsAuxSend::SetAuxPan(UInt32 ch, UInt32 v)         { return SetAuxOutChannelFeature(ch, 3, 0 - v); }

// ref FWSettingsAuxSend::SetAuxMonitor: value 0/1 only, map kind 5 (channel = the output pair), AV/C selector command on the mapped block
IOReturn com_openaudio_FWSettingsAuxSend::SetAuxMonitor(UInt32 idx, UInt32 v)
{
    if (v >= 2) return kErrBadArg;
    FWMap m = { 5, 0, (UInt8)idx, 0 };
    IOReturn r = fDevice->MapChannelAndFunctionBlock(m);
    if (r != kIOReturnSuccess) return r;
    return fDevice->AVCSelectorCommand(m.fb, v != 0);
}

// ref FWSettingsAuxSend::SendToDevice: the aux-send group, the monitor array in full, the first two pan elements
IOReturn com_openaudio_FWSettingsAuxSend::SendToDevice()
{
    fAuxSend->SendLevelsToDevice(0);
    for (UInt32 i = 0; i < fMonitorCount; i++) fMonitor->perElement()(i + 1, fMonitor->Get(i));
    fPan->perElement()(1, fPan->Get(0));
    fPan->perElement()(2, fPan->Get(1));
    return kIOReturnSuccess;
}

// ref FWSettingsAuxSend::ResetToFactorySettings
IOReturn com_openaudio_FWSettingsAuxSend::ResetToFactorySettings()
{
    for (UInt32 i = 0, n = fAuxSend->Count(); i < n; i++) fAuxSend->ChannelAtIndex(i)->level = kFWMinLevel;
    fPan->Set(0, 0xffffff01);
    fPan->Set(1, 0xff);
    fMonitor->Fill(0);
    return kIOReturnSuccess;
}

// =====================================================================================================================================
// Rotary control
// =====================================================================================================================================

IOReturn com_openaudio_FWSettingsRotaryControl::Init(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *swReturn,
                                                     FWChannelGroup *output, FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut)
{
    com_openaudio_FWPropertyDict *dict = device->propDict();
    fDevice = device; fDict = dict;
    fGroups[0] = swReturn; fGroups[1] = output; fGroups[2] = input; fGroups[3] = headphone; fGroups[4] = auxOut;
    fRotary = dict->AddNumber64("MARotaryControlV2", FWCall64());               // ref NullSetProperty64
    if (!fRotary) return kErrNoMem;
    fVirtual = dict->AddFunction("MAMetaVirtualRotateDataKnob", FWBindObj<com_openaudio_FWSettingsRotaryControl, &com_openaudio_FWSettingsRotaryControl::VirtualRotateKnob>(this));
    if (!fVirtual) return kErrNoMem;
    // clone key (plan v7 AP-5.4), not in the reference
    fMonitorStep = dict->AddNumber32("MAMonitorKnobStep", FWBind1<com_openaudio_FWSettingsRotaryControl, &com_openaudio_FWSettingsRotaryControl::SetMonitorKnobStep>(this));
    if (!fMonitorStep) return kErrNoMem;
    fMonitorStep->set(3);                                                        // 1.5 dB per detent (user, 2026-10-20)
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWSettingsRotaryControl::SetMonitorKnobStep(UInt32 v)
{
    return (v == 0 || v == 3 || v == 6) ? kIOReturnSuccess : kErrBadArg;     // the property stores the value after a successful setter
}

IOReturn com_openaudio_FWSettingsRotaryControl::ResetToFactorySettings()
{
    if (fRotary) fRotary->set(0xffff00000000ULL);                                // ref *(this+0x68) = 0xffff00000000
    if (fMonitorStep) fMonitorStep->set(3);                                      // clone: 1.5 dB per detent (AP-5.4, user default 2026-10-20)
    return kIOReturnSuccess;
}

// ref AdjustOSNumberElement (decomp AdjustOSNumberElement.c): one detent changes the level by one step (0.5 dB); far below -12 steps are
// multiplied by 12. The floor is -0x80 (the lower clamp -0x100 of the reference can never trigger).
// Clone (plan v7 AP-5.4): with a monitor step (> 0) the monitor law FWMonitorStep replaces it.
static void AdjustOSNumberElement(OSArray *arr, UInt32 idx, SInt32 delta, SInt32 monitorStep)
{
    OSNumber *n = OSDynamicCast(OSNumber, arr->getObject(idx));
    if (!n) return;
    const SInt32 v = (SInt32)n->unsigned32BitValue();
    const SInt32 r = monitorStep > 0 ? FWMonitorStep(v, delta, monitorStep) : FWRotaryStep(v, delta);   // FWFrontPanelLaw.h
    OSNumber *nn = OSNumber::withNumber((UInt64)(UInt32)r, 32);
    if (!nn) return;
    arr->replaceObject(idx, nn);
    nn->release();
}

void com_openaudio_FWSettingsRotaryControl::SetPairLevelsFromRotary(const char *key, UInt32 pair, OSArray *&arr, SInt32 delta)
{
    if (!arr) {
        OSObject *cur = fDict->CopyProperty(key);                                 // +1
        arr = OSDynamicCast(OSArray, cur);
        if (!arr) { if (cur) cur->release(); return; }
    }
    // clone (AP-5.4): the monitor keys use the per-unit monitor step; inputs, SW returns keep the official law
    const bool monitorKey = !strcmp(key, "MAOutputLevels") || !strcmp(key, "MAHeadphoneLevels") || !strcmp(key, "MAAuxOutputLevels");
    const SInt32 step = (monitorKey && fMonitorStep) ? (SInt32)fMonitorStep->value() : 0;
    AdjustOSNumberElement(arr, pair * 2, delta, step);
    AdjustOSNumberElement(arr, pair * 2 + 1, delta, step);
}

void com_openaudio_FWSettingsRotaryControl::SetLevelsFromKnob(UInt64 mask, const char *key, UInt32 firstBit, FWChannelGroup *g, SInt32 delta)
{
    if (!g) return;
    OSArray *arr = 0;
    const UInt32 pairs = g->Count() >> 1;
    for (UInt32 i = 0; i < pairs; i++)
        if ((mask >> ((firstBit + i) & 0x3f)) & 1) SetPairLevelsFromRotary(key, i, arr, delta);
    if (arr) { fDict->SetPropertyAndNotifyUserClients(key, arr); arr->release(); }
}

void com_openaudio_FWSettingsRotaryControl::UserRotatedHardwareKnob(SInt32 delta)
{
    const UInt64 mask = fRotary ? fRotary->value() : 0;
    if (mask == 0) return;
    SetLevelsFromKnob(mask, "MASWReturnLevels", 0,    fGroups[0], delta);
    SetLevelsFromKnob(mask, "MAOutputLevels",   0x20, fGroups[1], delta);
    SetLevelsFromKnob(mask, "MAInputLevels",    0x10, fGroups[2], delta);
    SetLevelsFromKnob(mask, "MAHeadphoneLevels",0x38, fGroups[3], delta);
    SetLevelsFromKnob(mask, "MAAuxOutputLevels",0x32, fGroups[4], delta);
}

IOReturn com_openaudio_FWSettingsRotaryControl::VirtualRotateKnob(OSObject *v)
{
    OSNumber *n = OSDynamicCast(OSNumber, v);
    if (!n) return (IOReturn)0xE00002BC;
    UserRotatedHardwareKnob((SInt32)n->unsigned32BitValue());
    return kIOReturnSuccess;
}

// =====================================================================================================================================
// A/B switch
// =====================================================================================================================================

IOReturn com_openaudio_FWSettingsABSwitch::Init(com_openaudio_FWAVCAudioDevice *device)
{
    com_openaudio_FWPropertyDict *dict = device->propDict();
    fDevice = device; fDict = dict; fMode = 0; fHeadphoneOverride = 0;
    typedef com_openaudio_FWSettingsABSwitch S;
    fRoutings = dict->AddArray("MAMomentarySwitchRoutings", 4, FWBind2<S, &S::SetMomentarySwitchRoutingSource>(this));
    if (!fRoutings) return kErrNoMem;
    fHeadphoneSource = dict->AddNumber32("MAHeadphoneSourceV2", FWBind1<S, &S::SetHeadphoneSource>(this));
    if (!fHeadphoneSource) return kErrNoMem;
    fHeadphoneSecond = dict->AddNumber32("MAHeadphoneSecondSource", FWBind1<S, &S::SetHeadphoneSecondSource>(this));
    if (!fHeadphoneSecond) return kErrNoMem;
    fMomentarySource = dict->AddNumber32("MAMomentarySwitchSource", FWBind1<S, &S::SetMomentarySwitchSource>(this));
    if (!fMomentarySource) return kErrNoMem;
    FWCall1 nullSet; nullSet.fn = 0;                                             // ref NullSetProperty32
    fLatent = dict->AddNumber32("MAMomentarySwitchLatent", nullSet);
    return fLatent ? kIOReturnSuccess : kErrNoMem;
}

// ref ABSwitch::SendToDevice: normalises the override flag
IOReturn com_openaudio_FWSettingsABSwitch::SendToDevice() { fHeadphoneOverride = (fHeadphoneOverride != 0); return kIOReturnSuccess; }

IOReturn com_openaudio_FWSettingsABSwitch::ResetToFactorySettings()
{
    fHeadphoneOverride = 0;
    fMode = 0;
    fRoutings->Set(0, 1); fRoutings->Set(1, 2); fRoutings->Set(2, 3); fRoutings->Set(3, 4);
    fHeadphoneSource->set(1);                                                    // ref *(this+0x144) = 1
    fHeadphoneSecond->set(1);                                                    // ref *(this+0x184) = 1
    return kIOReturnSuccess;
}

// ref FWSettingsABSwitch::SourceMaskToHeadphoneSelector @0x1dd...: index of the lowest set bit (0 when none)
UInt32 com_openaudio_FWSettingsABSwitch::SourceMaskToHeadphoneSelector(UInt32 mask) const
{
    for (UInt32 i = 0; i < 32; i++) if ((mask >> i) & 1) return i;
    return 0;
}

// ref UpdateLEDs @0xe96a: override active -> LEDs off (0); else A -> 1, B -> 2, anything else -> 3
void com_openaudio_FWSettingsABSwitch::UpdateLEDs()
{
    fLed(FWABLedValue(fHeadphoneOverride, fMode));
}

void com_openaudio_FWSettingsABSwitch::SubmitHeadphoneSourceChange()
{
    const UInt32 src = fHeadphoneSource->value();
    OSNumber *n = OSNumber::withNumber((UInt64)src, 32);
    if (!n) return;
    fHpSelect(0, SourceMaskToHeadphoneSelector(src));
    fDict->SendArbitraryPropertyToUserClients("MAHeadphoneSourceV2", n);
    n->release();
}

IOReturn com_openaudio_FWSettingsABSwitch::SetMomentarySwitchRoutingSource(UInt32 idx, UInt32 v)
{
    if (idx - 1 == fMode) { fHeadphoneSource->set(v); SubmitHeadphoneSourceChange(); }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWSettingsABSwitch::SetHeadphoneSource(UInt32 v)
{
    fHeadphoneOverride = 1;
    UpdateLEDs();
    fHpSelect(0, SourceMaskToHeadphoneSelector(v));
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWSettingsABSwitch::SetHeadphoneSecondSource(UInt32 v)
{
    fHpSelect(1, SourceMaskToHeadphoneSelector(v));
    return kIOReturnSuccess;
}

// ref SetMomentarySwitchSource: runs before the property stores `v`, so SendMixerSettingsForCurrentMomentaryMode sees the OLD value
IOReturn com_openaudio_FWSettingsABSwitch::SetMomentarySwitchSource(UInt32)
{
    SendMixerSettingsForCurrentMomentaryMode();
    return kIOReturnSuccess;
}

void com_openaudio_FWSettingsABSwitch::SendMixerSettingsForCurrentMomentaryMode()
{
    if (fMomentarySource->value() == 0) {
        fHeadphoneSource->set(fRoutings->Get(fMode));
        SubmitHeadphoneSourceChange();
        return;
    }
    UInt32 a = 0, b = 0;
    OSObject *curObj = fDict->CopyProperty("MAInputRoutingV2");
    OSArray *cur = OSDynamicCast(OSArray, curObj);
    if (!cur && curObj) curObj->release();
    if (cur) {
        OSNumber *n0 = OSDynamicCast(OSNumber, cur->getObject(0));
        OSNumber *n1 = OSDynamicCast(OSNumber, cur->getObject(1));
        if (n0) a = n0->unsigned32BitValue();
        if (n1) b = n1->unsigned32BitValue();
        cur->release();
    }
    FWABDirectMonitoringBits(fMode, a, b);
    OSNumber *na = OSNumber::withNumber((UInt64)a, 32);
    OSNumber *nb = OSNumber::withNumber((UInt64)b, 32);
    if (na && nb) {
        const OSObject *objs[2] = { na, nb };
        OSArray *arr = OSArray::withObjects(objs, 2, 0);
        if (arr) { fDict->SetPropertyAndNotifyUserClients("MAInputRoutingV2", arr); arr->release(); }
    }
    if (na) na->release();
    if (nb) nb->release();
}

// ref MomentarySwitchPressed: the hardware A/B button
void com_openaudio_FWSettingsABSwitch::MomentarySwitchPressed()
{
    fHeadphoneOverride = 0;
    fMode = FWABToggle(fMode);
    SendMixerSettingsForCurrentMomentaryMode();
    UpdateLEDs();
}
