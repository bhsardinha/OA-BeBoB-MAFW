#include "FWPropertyDict.h"
#include "Trace.h"
#include "FWKIPCUserClient.h"
#include <libkern/c++/OSCollectionIterator.h>
#include <libkern/OSAtomic.h>
#include <IOKit/audio/IOAudioLevelControl.h>
#include <IOKit/audio/IOAudioToggleControl.h>

static const IOReturn kErrNoMem      = (IOReturn)0xE00002BD;
static const IOReturn kErrBadArg     = (IOReturn)0xE00002C2;
static const IOReturn kErrNotReadable = (IOReturn)0xE00002CE;
static const IOReturn kErrNotWritable = (IOReturn)0xE00002CF;

// ---- FWBaseProp -------------------------------------------------------------------------------------------------------------------

OSDefineMetaClassAndAbstractStructors(com_openaudio_FWBaseProp, OSObject)

bool com_openaudio_FWBaseProp::InitProp(const char *key)
{
    if (!OSObject::init()) return false;
    fDict = 0; fImmediate = false; fControls = 0;
    fKey = const_cast<OSSymbol *>(OSSymbol::withCString(key));
    return fKey != 0;
}

void com_openaudio_FWBaseProp::free()
{
    if (fControls) { fControls->release(); fControls = 0; }
    if (fKey) { fKey->release(); fKey = 0; }
    OSObject::free();
}

// ---- audio controls (ref FWBaseProp::HookUpAudioControl and friends) --------------------------------------------------------------

void com_openaudio_FWBaseProp::HookUpAudioControl(IOAudioControl *c, IOAudioEngine *engine)
{
    c->setValueChangeHandler(&com_openaudio_FWBaseProp::StaticValueChanged, this);
    engine->addDefaultAudioControl(c);
    if (!fControls) fControls = OSSet::withCapacity(10);
    if (fControls) fControls->setObject(c);
    c->release();                                                             // engine and set hold it now
}

IOReturn com_openaudio_FWBaseProp::StaticValueChanged(OSObject *target, IOAudioControl *c, SInt32 oldValue, SInt32 newValue)
{
    com_openaudio_FWBaseProp *p = OSDynamicCast(com_openaudio_FWBaseProp, target);
    if (!p) return kErrBadArg;
    return p->ChangedValueThroughAudioControl(c, oldValue, newValue);
}

IOReturn com_openaudio_FWBaseProp::EstablishVolumeControl(IOAudioEngine *engine, UInt32 usage, UInt32 channelID, UInt32 controlID,
                                                          SInt32 minValue, SInt32 maxValue, SInt32 minDB, SInt32 maxDB)
{
    IOAudioControl *c = IOAudioLevelControl::createVolumeControl(0, minValue, maxValue, (IOFixed)(minDB << 16), (IOFixed)(maxDB << 16),
                                                                 channelID, 0, controlID, usage);
    if (!c) return kErrNoMem;
    HookUpAudioControl(c, engine);
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWBaseProp::EstablishMuteControl(IOAudioEngine *engine, UInt32 usage, UInt32 channelID, UInt32 controlID)
{
    IOAudioControl *c = IOAudioToggleControl::createMuteControl(false, channelID, 0, controlID, usage);
    if (!c) return kErrNoMem;
    HookUpAudioControl(c, engine);
    return kIOReturnSuccess;
}

// ref FWBaseProp::ChangedValueThroughAudioControl: a pass-through mute is stored inverted; the value goes back into the dictionary
IOReturn com_openaudio_FWBaseProp::ChangedValueThroughAudioControl(IOAudioControl *c, SInt32, SInt32 newValue)
{
    if (!c) return kIOReturnSuccess;
    if (c->getUsage() == kIOAudioControlUsagePassThru && c->getSubType() == kIOAudioToggleControlSubTypeMute) newValue = (newValue == 0);
    OSNumber *n = OSNumber::withNumber((UInt64)(UInt32)newValue, 32);
    if (!n) return kErrNoMem;
    if (fDict) fDict->InjectModifiedProperty(fKey, n, false);
    n->release();
    return kIOReturnSuccess;
}

void com_openaudio_FWBaseProp::UpdateAudioControlsWithNewValue(OSObject *v)
{
    if (!fControls) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fControls);
    if (!it) return;
    while (IOAudioControl *c = OSDynamicCast(IOAudioControl, it->getNextObject())) {
        OSObject *toSend = v;
        OSNumber *inv = 0;
        OSNumber *n = OSDynamicCast(OSNumber, v);
        if (n && c->getUsage() == kIOAudioControlUsagePassThru && c->getSubType() == kIOAudioToggleControlSubTypeMute) {
            inv = OSNumber::withNumber((UInt64)(n->unsigned32BitValue() == 0), 32);
            if (inv) toSend = inv;
        }
        c->hardwareValueChanged(toSend);
        if (inv) inv->release();
    }
    it->release();
}

void com_openaudio_FWBaseProp::NotifyAudioControlsOfChange()
{
    if (fControls && fDict) fDict->AddUpdateForAudioControl(this);
}

void com_openaudio_FWBaseProp::NotifyAudioControlsInWorkLoop()
{
    OSObject *v = 0;
    if (CallGetter(0, v) == kIOReturnSuccess && v) { UpdateAudioControlsWithNewValue(v); v->release(); }
}

void com_openaudio_FWBaseProp::DetachAudioControlsFromEngine(IOAudioEngine *engine)
{
    if (!fControls || !engine) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fControls);
    if (it) {
        while (IOAudioControl *c = OSDynamicCast(IOAudioControl, it->getNextObject())) engine->removeDefaultAudioControl(c);
        it->release();
    }
    fControls->flushCollection();
}

IOReturn com_openaudio_FWBaseProp::CallGetter(const char *, OSObject *&) { return kErrNotReadable; }
IOReturn com_openaudio_FWBaseProp::CallSetter(const OSSymbol *, OSObject *) { return kErrNotWritable; }

// ---- numbers ----------------------------------------------------------------------------------------------------------------------

OSDefineMetaClassAndStructors(com_openaudio_FWNumber32Prop, com_openaudio_FWBaseProp)

IOReturn com_openaudio_FWNumber32Prop::Init(com_openaudio_FWPropertyDict *dict, const char *key, const FWCall1 &setter, UInt32 *external)
{
    if (!InitProp(key)) return kErrBadArg;
    fSetter = setter; fExternal = external; fValue = 0;
    return dict->InstallProperty(this);
}

IOReturn com_openaudio_FWNumber32Prop::CallGetter(const char *, OSObject *&out)
{
    OSNumber *n = OSNumber::withNumber((UInt64)value(), 32);
    if (!n) return kErrNoMem;
    out = n;
    return kIOReturnSuccess;
}

// ref FW32BitNumberProp::CallSetter: the functor runs first; the value is stored only when it returned 0
IOReturn com_openaudio_FWNumber32Prop::CallSetter(const OSSymbol *, OSObject *v)
{
    OSNumber *n = OSDynamicCast(OSNumber, v);
    if (!n) return kErrBadArg;
    const UInt32 x = n->unsigned32BitValue();
    IOReturn r = fSetter(x);
    if (r != kIOReturnSuccess) return r;
    set(x);
    return kIOReturnSuccess;
}

OSDefineMetaClassAndStructors(com_openaudio_FWNumber64Prop, com_openaudio_FWBaseProp)

IOReturn com_openaudio_FWNumber64Prop::Init(com_openaudio_FWPropertyDict *dict, const char *key, const FWCall64 &setter)
{
    if (!InitProp(key)) return kErrBadArg;
    fSetter = setter; fValue = 0;
    return dict->InstallProperty(this);
}

IOReturn com_openaudio_FWNumber64Prop::CallGetter(const char *, OSObject *&out)
{
    OSNumber *n = OSNumber::withNumber(fValue, 64);
    if (!n) return kErrNoMem;
    out = n;
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWNumber64Prop::CallSetter(const OSSymbol *, OSObject *v)
{
    OSNumber *n = OSDynamicCast(OSNumber, v);
    if (!n) return kErrBadArg;
    const UInt64 x = n->unsigned64BitValue();
    IOReturn r = fSetter(x);
    if (r != kIOReturnSuccess) return r;
    fValue = x;
    return kIOReturnSuccess;
}

// ---- function ---------------------------------------------------------------------------------------------------------------------

OSDefineMetaClassAndStructors(com_openaudio_FWFunctionProp, com_openaudio_FWBaseProp)

IOReturn com_openaudio_FWFunctionProp::Init(com_openaudio_FWPropertyDict *dict, const char *key, const FWCallObj &fn)
{
    if (!InitProp(key)) return kErrBadArg;
    fFn = fn;
    fImmediate = true;                                                       // ref FWFunctionProp::init: this[0x28] = 1
    return dict->InstallProperty(this);
}

IOReturn com_openaudio_FWFunctionProp::CallSetter(const OSSymbol *, OSObject *v) { return fFn(v); }

// ---- arrays -----------------------------------------------------------------------------------------------------------------------

OSDefineMetaClassAndStructors(com_openaudio_FWArrayProp, com_openaudio_FWBaseProp)

IOReturn com_openaudio_FWArrayProp::Init(com_openaudio_FWPropertyDict *dict, const char *key, UInt32 n, const FWCall2 &perElement)
{
    if (n == 0) { OA_LOG("FWArrayProp %s: array with zero elements (the reference panics here)", key); return kErrBadArg; }
    if (!InitProp(key)) return kErrBadArg;
    fCount = n; fPer = perElement;
    fValues = static_cast<UInt32 *>(IOMalloc((size_t)n * 4));
    if (!fValues) return kErrNoMem;
    bzero(fValues, (size_t)n * 4);
    return dict->InstallProperty(this);
}

void com_openaudio_FWArrayProp::free()
{
    if (fValues) { IOFree(fValues, (size_t)fCount * 4); fValues = 0; }
    com_openaudio_FWBaseProp::free();
}

UInt32 com_openaudio_FWArrayProp::Get(UInt32 i) const { return (fValues && i < fCount) ? fValues[i] : 0; }
void   com_openaudio_FWArrayProp::Set(UInt32 i, UInt32 v) { if (fValues && i < fCount) fValues[i] = v; }
void   com_openaudio_FWArrayProp::Fill(UInt32 v) { if (fValues) for (UInt32 i = 0; i < fCount; i++) fValues[i] = v; }

UInt32 com_openaudio_FWArrayProp::Get32BitValueForArrayElement(UInt32 i) const { return Get(i); }

void com_openaudio_FWArrayProp::Set32BitValueFromArrayElement(UInt32 i, UInt32 v)
{
    if (i < fCount && fValues) { fValues[i] = v; fPer(i + 1, v); }
}

IOReturn com_openaudio_FWArrayProp::SendAllToDevice()
{
    if (!fPer.isSet()) return kIOReturnSuccess;
    for (UInt32 i = 0; i < fCount; i++) fPer(i + 1, Get(i));
    return kIOReturnSuccess;
}

// ref OSArrayToSeriatimValues @ FWBaseArrayProp: every element must be an OSNumber (anything else -> 0xE00002C2 after the earlier ones applied)
IOReturn com_openaudio_FWArrayProp::OSArrayToSeriatimValues(OSObject *arr, UInt32 n)
{
    OSArray *a = OSDynamicCast(OSArray, arr);
    if (!a) return kErrBadArg;
    if (a->getCount() < n) n = a->getCount();
    for (UInt32 i = 0; i < n; i++) {
        OSNumber *num = OSDynamicCast(OSNumber, a->getObject(i));
        if (!num) return kErrBadArg;
        Set32BitValueFromArrayElement(i, num->unsigned32BitValue());
    }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWArrayProp::SeriatimValuesToOSArray(UInt32 n, OSObject *&out)
{
    OSArray *a = OSArray::withCapacity(n);
    if (!a) { out = 0; return kErrNoMem; }
    for (UInt32 i = 0; i < n; i++) {
        OSNumber *num = OSNumber::withNumber((UInt64)Get32BitValueForArrayElement(i), 32);
        if (!num) { a->release(); out = 0; return kErrNoMem; }
        a->setObject(num);
        num->release();
    }
    out = a;
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWArrayProp::EstablishVolumeControls(IOAudioEngine *engine, UInt32 usage, UInt32 firstChannel, SInt32 minValue,
                                                            SInt32 maxValue, SInt32 minDB, SInt32 maxDB)
{
    fFirstChannel = firstChannel;
    for (UInt32 i = 0; i < fCount; i++) {
        IOReturn r = EstablishVolumeControl(engine, usage, firstChannel + i, i, minValue, maxValue, minDB, maxDB);
        if (r != kIOReturnSuccess) return r;
    }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWArrayProp::EstablishMuteControls(IOAudioEngine *engine, UInt32 usage, UInt32 firstChannel)
{
    fFirstChannel = firstChannel;
    for (UInt32 i = 0; i < fCount; i++) {
        IOReturn r = EstablishMuteControl(engine, usage, firstChannel + i, i);
        if (r != kIOReturnSuccess) return r;
    }
    return kIOReturnSuccess;
}

// ref FWBaseArrayProp::ChangedValueThroughAudioControl: a control with channel ID 0 is the master (every element), otherwise its control ID
// is the element index
IOReturn com_openaudio_FWArrayProp::ChangedValueThroughAudioControl(IOAudioControl *c, SInt32, SInt32 newValue)
{
    OSObject *cur = 0;
    IOReturn r = SeriatimValuesToOSArray(fCount, cur);
    if (r != kIOReturnSuccess) return r;
    OSArray *arr = OSDynamicCast(OSArray, cur);
    OSNumber *n = OSNumber::withNumber((UInt64)(UInt32)newValue, 32);
    if (!n || !arr) { if (n) n->release(); if (cur) cur->release(); return kErrNoMem; }
    if (c->getChannelID() == 0) { for (UInt32 i = 0; i < fCount; i++) arr->replaceObject(i, n); }
    else arr->replaceObject(c->getControlID(), n);
    if (fDict) fDict->InjectModifiedProperty(fKey, arr, false);
    n->release();
    arr->release();
    return kIOReturnSuccess;
}

void com_openaudio_FWArrayProp::UpdateAudioControlsForChannelWithNewValue(UInt32 channel, OSObject *v)
{
    if (!fControls) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fControls);
    if (!it) return;
    while (IOAudioControl *c = OSDynamicCast(IOAudioControl, it->getNextObject()))
        if (c->getChannelID() == channel) c->hardwareValueChanged(v);
    it->release();
}

void com_openaudio_FWArrayProp::UpdateAudioControlsWithNewValue(OSObject *v)
{
    OSArray *a = OSDynamicCast(OSArray, v);
    if (!a) return;
    for (UInt32 i = 0; i < a->getCount(); i++) UpdateAudioControlsForChannelWithNewValue(fFirstChannel + i, a->getObject(i));
}

IOReturn com_openaudio_FWArrayProp::CallGetter(const char *, OSObject *&out) { return SeriatimValuesToOSArray(fCount, out); }

IOReturn com_openaudio_FWArrayProp::CallSetter(const OSSymbol *, OSObject *value)
{
    IOReturn r = OSArrayToSeriatimValues(value, fCount);
    if (r == kIOReturnSuccess) fAllChanged();
    return r;
}

// ---- channel-group property -------------------------------------------------------------------------------------------------------

OSDefineMetaClassAndStructors(com_openaudio_FWChannelGroupProp, com_openaudio_FWArrayProp)

IOReturn com_openaudio_FWChannelGroupProp::Init(com_openaudio_FWPropertyDict *dict, const char *key, FWChannelGroup *group,
                                                UInt32 fieldOffset, FWChannelGroup *second)
{
    IOReturn r = com_openaudio_FWArrayProp::Init(dict, key, group->Count(), FWCall2());
    if (r != kIOReturnSuccess) return r;
    fGroup = group; fField = fieldOffset; fSecond = second;
    return kIOReturnSuccess;
}

// element access goes to the channel slot field (ref FWChannelGroupProperty::Get32BitValueForArrayElement / Set32BitValueFromArrayElement)
UInt32 com_openaudio_FWChannelGroupProp::Get32BitValueForArrayElement(UInt32 i) const
{
    return *reinterpret_cast<const SInt32 *>(reinterpret_cast<const UInt8 *>(fGroup->ChannelAtIndex(i)) + fField);
}

void com_openaudio_FWChannelGroupProp::Set32BitValueFromArrayElement(UInt32 i, UInt32 v)
{
    *reinterpret_cast<SInt32 *>(reinterpret_cast<UInt8 *>(fGroup->ChannelAtIndex(i)) + fField) = (SInt32)(SInt16)v;
}

// ref FWChannelGroupProperty::CallSetter @0x940e
IOReturn com_openaudio_FWChannelGroupProp::CallSetter(const OSSymbol *, OSObject *value)
{
    SInt32 *beforeA = fGroup->AdjustedLevels();
    if (!beforeA) return kIOReturnSuccess;                                    // ref returns 0 when the snapshot cannot be taken
    SInt32 *beforeB = fSecond ? fSecond->AdjustedLevels() : 0;
    IOReturn r = OSArrayToSeriatimValues(value, fCount);
    if (r == kIOReturnSuccess) {
        fGroup->SendLevelsToDevice(beforeA);
        if (fSecond) fSecond->SendLevelsToDevice(beforeB);
    }
    IOFree(beforeA, fGroup->AdjustedBytes());
    if (beforeB) IOFree(beforeB, fSecond->AdjustedBytes());
    return r;
}

// ---- FWPropertyDict ---------------------------------------------------------------------------------------------------------------

#define super IOEventSource
OSDefineMetaClassAndStructors(com_openaudio_FWPropertyDict, IOEventSource)

bool com_openaudio_FWPropertyDict::InitWithDevice(IOService *device)
{
    if (!device || !super::init(this, 0)) return false;
    fDevice = device; fImmediate = 0; fQueue = 0; fProps = 0; fAudioUpdates = 0; fBusy = 0;
    fLoop = device->getWorkLoop();
    if (!fLoop) return false;
    fLoop->retain();
    fLoop->addEventSource(this);
    return true;
}

void com_openaudio_FWPropertyDict::Detach()
{
    if (fLoop) { fLoop->removeEventSource(this); fLoop->release(); fLoop = 0; }
}

void com_openaudio_FWPropertyDict::free()
{
    if (fProps)        { fProps->release();        fProps = 0; }
    if (fQueue)        { fQueue->release();        fQueue = 0; }
    if (fImmediate)    { fImmediate->release();    fImmediate = 0; }
    if (fAudioUpdates) { fAudioUpdates->release(); fAudioUpdates = 0; }
    super::free();
}

// ref InstallProperty @ FWPropertyDict: dictionary (capacity 0x20) key -> prop; immediate props are also remembered in a set (capacity 4)
IOReturn com_openaudio_FWPropertyDict::InstallProperty(com_openaudio_FWBaseProp *p)
{
    if (!fProps) {
        fProps = OSDictionary::withCapacity(0x20);
        if (!fProps) return kErrNoMem;
    }
    fProps->setObject(p->key(), p);
    if (p->isImmediate()) {
        if (!fImmediate) {
            fImmediate = OSSet::withCapacity(4);
            if (!fImmediate) return kIOReturnSuccess;                        // ref returns 0 here
        }
        fImmediate->setObject(p->key());
    }
    p->attach(this);
    return kIOReturnSuccess;
}

// Factories: create, initialise and install a property. The dictionary keeps the only reference; the returned pointer stays valid for the
// life of the dictionary (the reference embeds the property objects in their owners instead).
com_openaudio_FWNumber32Prop *com_openaudio_FWPropertyDict::AddNumber32(const char *key, const FWCall1 &setter, UInt32 *external)
{
    com_openaudio_FWNumber32Prop *p = OSTypeAlloc(com_openaudio_FWNumber32Prop);
    if (!p) return 0;
    IOReturn r = p->Init(this, key, setter, external);
    p->release();
    return r == kIOReturnSuccess ? p : 0;
}
com_openaudio_FWNumber64Prop *com_openaudio_FWPropertyDict::AddNumber64(const char *key, const FWCall64 &setter)
{
    com_openaudio_FWNumber64Prop *p = OSTypeAlloc(com_openaudio_FWNumber64Prop);
    if (!p) return 0;
    IOReturn r = p->Init(this, key, setter);
    p->release();
    return r == kIOReturnSuccess ? p : 0;
}
com_openaudio_FWFunctionProp *com_openaudio_FWPropertyDict::AddFunction(const char *key, const FWCallObj &fn)
{
    com_openaudio_FWFunctionProp *p = OSTypeAlloc(com_openaudio_FWFunctionProp);
    if (!p) return 0;
    IOReturn r = p->Init(this, key, fn);
    p->release();
    return r == kIOReturnSuccess ? p : 0;
}
com_openaudio_FWArrayProp *com_openaudio_FWPropertyDict::AddArray(const char *key, UInt32 n, const FWCall2 &perElement)
{
    com_openaudio_FWArrayProp *p = OSTypeAlloc(com_openaudio_FWArrayProp);
    if (!p) return 0;
    IOReturn r = p->Init(this, key, n, perElement);
    p->release();
    return r == kIOReturnSuccess ? p : 0;
}
com_openaudio_FWChannelGroupProp *com_openaudio_FWPropertyDict::AddChannelGroup(const char *key, FWChannelGroup *group, UInt32 fieldOffset,
                                                                                 FWChannelGroup *second)
{
    com_openaudio_FWChannelGroupProp *p = OSTypeAlloc(com_openaudio_FWChannelGroupProp);
    if (!p) return 0;
    IOReturn r = p->Init(this, key, group, fieldOffset, second);
    p->release();
    return r == kIOReturnSuccess ? p : 0;
}

IOReturn com_openaudio_FWPropertyDict::SetProperties(OSObject *props)
{
    IOReturn r = kErrBadArg;
    if (OSDictionary *d = OSDynamicCast(OSDictionary, props)) {
        OSCollectionIterator *it = OSCollectionIterator::withCollection(d);
        if (it) {
            OSSymbol *k = OSDynamicCast(OSSymbol, it->getNextObject());
            if (k) {
                r = kIOReturnSuccess;
                do {
                    SetOneProperty(k, d->getObject(k));
                    k = OSDynamicCast(OSSymbol, it->getNextObject());
                } while (k);
            }
            it->release();
        }
    } else if (OSString *s = OSDynamicCast(OSString, props)) {
        const OSSymbol *sym = OSSymbol::withString(s);
        if (sym) { SetOneProperty(sym, 0); sym->release(); r = kIOReturnSuccess; }
    }
    signalWorkAvailable();
    return r;
}

IOReturn com_openaudio_FWPropertyDict::SetOneProperty(const OSSymbol *key, OSObject *value)
{
    if (fImmediate && fImmediate->containsObject(key)) {
        com_openaudio_FWBaseProp *p = OSDynamicCast(com_openaudio_FWBaseProp, fProps ? fProps->getObject(key) : 0);
        if (p) p->CallSetter(key, value);
    } else {
        ModifyProperty(key, value);
    }
    return kIOReturnSuccess;
}

void com_openaudio_FWPropertyDict::ModifyProperty(const OSSymbol *key, OSObject *value)
{
    if (!fQueue) fQueue = OSDictionary::withCapacity(10);
    if (!fQueue || !value) return;
    fQueue->setObject(key, value);
}

void com_openaudio_FWPropertyDict::InjectModifiedProperty(const OSSymbol *key, OSObject *value, bool notify)
{
    if (!fLoop || !fLoop->inGate()) return;                                  // ref: workloop vt[0x138] (on-loop test) gates the injection
    ModifyProperty(key, value);
    CheckModifiedProperties(notify);
}

void com_openaudio_FWPropertyDict::SetPropertyAndNotifyUserClients(const OSSymbol *key, OSObject *value)
{
    ModifyProperty(key, value);
    signalWorkAvailable();
}

void com_openaudio_FWPropertyDict::SetPropertyAndNotifyUserClients(const char *key, OSObject *value)
{
    const OSSymbol *s = OSSymbol::withCString(key);
    if (!s) return;
    SetPropertyAndNotifyUserClients(s, value);
    s->release();
}

void com_openaudio_FWPropertyDict::SetNumberAndNotifyUserClients(const char *key, UInt32 value)
{
    OSNumber *n = OSNumber::withNumber((UInt64)value, 32);
    if (!n) return;
    SetPropertyAndNotifyUserClients(key, n);
    n->release();
}

OSObject *com_openaudio_FWPropertyDict::CopyProperty(const char *key) const
{
    if (!fProps) return 0;
    com_openaudio_FWBaseProp *p = OSDynamicCast(com_openaudio_FWBaseProp, fProps->getObject(key));
    OSObject *out = 0;
    if (!p) {
        if (strcmp(key, "MAMetaAllProperties") == 0) return CopyAllProperties();
        return 0;
    }
    if (p->CallGetter(key, out) != kIOReturnSuccess) return 0;
    return out;                                                               // +1, owned by the caller
}

OSDictionary *com_openaudio_FWPropertyDict::CopyAllProperties() const
{
    OSDictionary *d = OSDictionary::withCapacity(fProps ? fProps->getCount() : 0);
    if (d) CopyAllPropertiesToDictionary(d);
    return d;
}

void com_openaudio_FWPropertyDict::CopyAllPropertiesToDictionary(OSDictionary *d) const
{
    if (!fProps || !d) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fProps);
    if (!it) return;
    while (OSSymbol *k = OSDynamicCast(OSSymbol, it->getNextObject())) {
        com_openaudio_FWBaseProp *p = OSDynamicCast(com_openaudio_FWBaseProp, fProps->getObject(k));
        OSObject *v = 0;
        if (p && p->CallGetter(k->getCStringNoCopy(), v) == kIOReturnSuccess && v) { d->setObject(k, v); v->release(); }
    }
    it->release();
}

void com_openaudio_FWPropertyDict::AddUpdateForAudioControl(com_openaudio_FWBaseProp *p)
{
    if (!fAudioUpdates) { fAudioUpdates = OSSet::withCapacity(10); if (!fAudioUpdates) return; }
    fAudioUpdates->setObject(p);
    signalWorkAvailable();
}

void com_openaudio_FWPropertyDict::CheckAudioControlUpdates()
{
    if (!fAudioUpdates || fAudioUpdates->getCount() == 0) return;
    // ref CheckAudioControlUpdates: push every pending property to its controls, then flush the set
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fAudioUpdates);
    if (it) {
        while (com_openaudio_FWBaseProp *p = OSDynamicCast(com_openaudio_FWBaseProp, it->getNextObject())) p->NotifyAudioControlsInWorkLoop();
        it->release();
    }
    fAudioUpdates->flushCollection();
}

void com_openaudio_FWPropertyDict::CheckModifiedProperties(bool notify)
{
    if (!fQueue) return;
    ApplyModifiedProperties(notify);
    SendPropertyChangedMessagesToUserClients();
}

// ref ApplyModifiedProperties: keys with no installed property are dropped; the rest are removed from the queue and applied in turn
// (a setter may queue further keys, so the queue is re-examined until empty; the cap is a kernel safety valve, not reference behaviour)
void com_openaudio_FWPropertyDict::ApplyModifiedProperties(bool notify)
{
    if (!OSCompareAndSwap(0, 1, &fBusy) || !fQueue) return;
    CacheModifiedPropertyDictionaryInUserClients(fQueue);
    for (int guard = 0; guard < 1024 && fQueue->getCount() != 0; guard++) {
        OSCollectionIterator *it = OSCollectionIterator::withCollection(fQueue);
        if (!it) break;
        OSSymbol *k = OSDynamicCast(OSSymbol, it->getNextObject());
        if (k) k->retain();
        it->release();
        if (!k) break;
        com_openaudio_FWBaseProp *p = OSDynamicCast(com_openaudio_FWBaseProp, fProps ? fProps->getObject(k) : 0);
        OSObject *v = fQueue->getObject(k);
        if (!p || !v) { fQueue->removeObject(k); k->release(); continue; }
        v->retain();
        fQueue->removeObject(k);
        IOReturn r = p->CallSetter(k, v);
        v->release();
        k->release();
        if (r == kIOReturnSuccess && notify) p->NotifyAudioControlsOfChange();
    }
    OSCompareAndSwap(1, 0, &fBusy);
}

bool com_openaudio_FWPropertyDict::checkForWork()
{
    CheckAudioControlUpdates();
    CheckModifiedProperties(true);
    if (fQueue && fQueue->getCount() != 0) signalWorkAvailable();
    return false;
}

// ---- user-client notification --------------------------------------------------------------------------------------------------------

static void CacheOne(com_openaudio_FWKIPCUserClient *c, void *dict) { c->AddModifiedProperties(static_cast<OSDictionary *>(dict)); }
static void SendOne(com_openaudio_FWKIPCUserClient *c, void *) { c->SendChangedPropertiesMessage(); }

void com_openaudio_FWPropertyDict::CacheModifiedPropertyDictionaryInUserClients(OSDictionary *d)
{
    if (d) com_openaudio_FWKIPCUserClient::ForEachClient(fDevice, &CacheOne, d);
}

void com_openaudio_FWPropertyDict::SendPropertyChangedMessagesToUserClients()
{
    com_openaudio_FWKIPCUserClient::ForEachClient(fDevice, &SendOne, 0);
}

// ref SendAllPropertiesToUserClients: the whole dictionary is queued for every client, then the work loop is poked
void com_openaudio_FWPropertyDict::SendAllPropertiesToUserClients()
{
    OSDictionary *all = CopyAllProperties();
    if (!all) return;
    CacheModifiedPropertyDictionaryInUserClients(all);
    all->release();
    signalWorkAvailable();
}

// ref SendArbitraryPropertyToUserClients: a one-entry dictionary, queued for every client
void com_openaudio_FWPropertyDict::SendArbitraryPropertyToUserClients(const char *key, OSObject *value)
{
    OSDictionary *d = OSDictionary::withCapacity(1);
    if (!d) return;
    d->setObject(key, value);
    CacheModifiedPropertyDictionaryInUserClients(d);
    d->release();
    signalWorkAvailable();
}
