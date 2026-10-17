#include "FWSettings.h"
#include "FWMixer.h"
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

// ref FWSettingsLevels::Init (decomp FWSettingsLevels__Init.c). The per-channel (non-batch) level setters are not ported (never used on the
// FW1814, see the header); the batch writers are installed by the mixer. The CoreAudio controls are created below, as in the reference.
IOReturn com_openaudio_FWSettingsLevels::Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict, FWChannelGroup *swReturn,
                                              FWChannelGroup *output, FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut)
{
    fMixer = mixer; fDict = dict;
    fSWReturn = swReturn; fOutput = output; fInput = input; fHeadphone = headphone; fAuxOut = auxOut;
    bzero(fGroupProp, sizeof(fGroupProp));
    fPan = fInputRouting = fSWReturnRouting = 0;
    fDirectMon = fGlobalMute = fGlobalDim = 0;
    IOReturn r;

    if (swReturn) {
        swReturn->SetLimits(kFWMinLevel, 0);
        if ((r = InitGroupProp("SWReturn", 0, swReturn, true, input)) != kIOReturnSuccess) return r;
        fSWReturnRouting = dict->AddArray("MASWReturnRoutingV2", swReturn->Count() >> 1, FWBind2<com_openaudio_FWSettingsLevels, &com_openaudio_FWSettingsLevels::SetSWReturnRouting>(this));
        if (!fSWReturnRouting) return kErrNoMem;
    }
    if (output) {
        output->SetLimits(kFWMinLevel, 0);
        if ((r = InitGroupProp("Output", 1, output, true, 0)) != kIOReturnSuccess) return r;
    }
    if (input) {
        input->SetLimits(kFWMinLevel, 0);
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
    if (IOAudioEngine *eng = fMixer->engine()) {
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
        if (IOAudioEngine *eng = fMixer->engine()) fGlobalMute->EstablishMuteControl(eng, kIOAudioControlUsageOutput, 0, 0);
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

// ref FWSettingsLevels::SetSWReturnRouting -> SetAVCRouting(busType 1, n, idx, mask) @ decomp FWSettingsLevels__SetAVCRouting.c: for every output
// pair i of the SW-return group, connect (mask bit i set) or disconnect software-return stream pair `idx` to pair i+1 of mixer function
// block 1. FW1814Device::MapRoutingParameters returns {function block 1, b 0} for every request (the base MapRoutingParameters, decomp
// FWAVCAudioDevice__MapRoutingParameters.c). Deviation: the reference queues these as asynchronous AV/C commands; the clone sends them in
// sequence and waits (see FWAvc.h).
IOReturn com_openaudio_FWSettingsLevels::SetSWReturnRouting(UInt32 idx, UInt32 mask)
{
    if (!fSWReturn) return kErrBadArg;
    IOReturn r = kErrBadArg;
    for (UInt32 i = 0; i < (fSWReturn->Count() >> 1); i++) {
        r = fMixer->SetAvcRouting((mask >> (i & 0x1f)) & 1, (UInt8)idx, (UInt8)(i + 1));
        if (r != kIOReturnSuccess) return r;
    }
    return r;
}

IOReturn com_openaudio_FWSettingsLevels::SetInputPan(UInt32 idx, UInt32 v) { return fMixer->SetInputPan(idx, v); }
IOReturn com_openaudio_FWSettingsLevels::SetInputRouting(UInt32 idx, UInt32 v) { return fMixer->SetInputRouting(idx, v); }

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

IOReturn com_openaudio_FWSettingsAuxSend::Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict, FWChannelGroup *auxSend,
                                               FWChannelGroup *auxOut, UInt32 nMonitor)
{
    fMixer = mixer; fDict = dict; fAuxSend = auxSend; fAuxOut = auxOut; fMonitorCount = nMonitor;
    auxSend->SetLimits(kFWMinLevel, 0);
    auxOut->SetLimits(kFWMinLevel, 0);
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

// ref SetAuxPan -> SetAuxOutChannelFeature(idx, 3, -v) @ decomp FWSettingsAuxSend__SetAuxOutChannelFeature.c: AV/C feature function block 9 (the
// FW1814 MapChannelAndFunctionBlock case 7), control selector 3 (balance), channel idx, value (-v) << 7
IOReturn com_openaudio_FWSettingsAuxSend::SetAuxPan(UInt32 idx, UInt32 v)
{
    if (idx == 0 || idx > fAuxOut->Count()) return kErrBadArg;
    return fMixer->SetAuxOutChannelFeature(idx, 3, (UInt16)((0 - v) << 7));
}

IOReturn com_openaudio_FWSettingsAuxSend::SetAuxMonitor(UInt32 idx, UInt32 v) { return fMixer->SetAuxMonitor(idx, v); }

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

IOReturn com_openaudio_FWSettingsRotaryControl::Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict, FWChannelGroup *swReturn,
                                                     FWChannelGroup *output, FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut)
{
    fMixer = mixer; fDict = dict;
    fGroups[0] = swReturn; fGroups[1] = output; fGroups[2] = input; fGroups[3] = headphone; fGroups[4] = auxOut;
    fRotary = dict->AddNumber64("MARotaryControlV2", FWCall64());               // ref NullSetProperty64
    if (!fRotary) return kErrNoMem;
    fVirtual = dict->AddFunction("MAMetaVirtualRotateDataKnob", FWBindObj<com_openaudio_FWSettingsRotaryControl, &com_openaudio_FWSettingsRotaryControl::VirtualRotateKnob>(this));
    return fVirtual ? kIOReturnSuccess : kErrNoMem;
}

IOReturn com_openaudio_FWSettingsRotaryControl::ResetToFactorySettings()
{
    if (fRotary) fRotary->set(0xffff00000000ULL);                                // ref *(this+0x68) = 0xffff00000000
    return kIOReturnSuccess;
}

// ref AdjustOSNumberElement (decomp AdjustOSNumberElement.c): one detent changes the level by one step (0.5 dB); far below -12 steps are
// multiplied by 12. The floor is -0x80 (the lower clamp -0x100 of the reference can never trigger).
static void AdjustOSNumberElement(OSArray *arr, UInt32 idx, SInt32 delta)
{
    OSNumber *n = OSDynamicCast(OSNumber, arr->getObject(idx));
    if (!n) return;
    const SInt32 v = (SInt32)n->unsigned32BitValue();
    SInt32 step;
    if (v < -0xc && v + delta < -0xc) step = delta * 0xc;
    else step = (delta > 0) ? 1 : -1;
    SInt32 r = -0x80;
    if (v + step > -0x81) r = v + step;
    if (r < -0x100) r = -0x100; else if (r > 0) r = 0;
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
    AdjustOSNumberElement(arr, pair * 2, delta);
    AdjustOSNumberElement(arr, pair * 2 + 1, delta);
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

IOReturn com_openaudio_FWSettingsABSwitch::Init(com_openaudio_FW1814Mixer *mixer, com_openaudio_FWPropertyDict *dict)
{
    fMixer = mixer; fDict = dict; fMode = 0; fHeadphoneOverride = 0;
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
    UInt32 state;
    if (fHeadphoneOverride == 0) state = (fMode == 1) ? 2 : (fMode == 0) ? 1 : 3;
    else state = 0;
    fLed(state);
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
    if (fMode == 0) { a &= 0xfffffffe; b &= 0xffffffed; }
    else            { a |= 1;          b |= 0x12; }
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
    if (fMode == 1) fMode = 0; else if (fMode == 0) fMode = 1;
    SendMixerSettingsForCurrentMomentaryMode();
    UpdateLEDs();
}
