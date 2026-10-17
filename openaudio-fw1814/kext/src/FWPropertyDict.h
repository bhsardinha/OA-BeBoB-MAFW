// FWPropertyDict + typed properties. Mirrors m_audio_b_FWPropertyDict (an IOEventSource), m_audio_b_FWBaseProp and its subclasses
// FW32BitNumberProp / FW64BitNumberProp / FWFunctionProp / FWBaseArrayProp / FWChannelGroupProperty (A1 Q-ME §1; decomp
// FWPropertyDict__*.c, FWBaseProp__*.c, FWBaseArrayProp__*.c, FW32BitNumberProp__*.c, FW64BitNumberProp__*.c, FWFunctionProp__*.c,
// FWChannelGroupProperty__*.c).
//
// Not ported here (named hooks, no-ops until their milestone): user-client caching / notification (M-F), IOAudioControl hookup (the
// Establish*Control(s) / NotifyAudioControls* family, M-E audio-control slice).
// Deviations (10.13 forces / safety): properties are heap OSObjects (the reference embeds them in other objects); the "array with zero
// elements" and "index out of range" panics of TArrayProperty.cpp become error returns.
#ifndef OA_FWPROPERTYDICT_H
#define OA_FWPROPERTYDICT_H

#include <IOKit/IOEventSource.h>
#include <IOKit/audio/IOAudioControl.h>
#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/IOService.h>
#include <IOKit/IOWorkLoop.h>
#include <libkern/c++/OSArray.h>
#include <libkern/c++/OSDictionary.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSSet.h>
#include <libkern/c++/OSSymbol.h>
#include "FWFunctor.h"
#include "FWChannelGroup.h"

class com_openaudio_FWPropertyDict;

class com_openaudio_FWBaseProp : public OSObject
{
    OSDeclareAbstractStructors(com_openaudio_FWBaseProp)
public:
    // ref FWBaseProp::init(const char*) @0x8b0c: the key symbol; returns false if it cannot be created
    bool InitProp(const char *key);
    virtual void free() APPLE_KEXT_OVERRIDE;
    // ref vt[0x120] / vt[0x128]. Base returns 0xE00002CE / 0xE00002CF (not readable / not writable).
    virtual IOReturn CallGetter(const char *key, OSObject *&out);
    virtual IOReturn CallSetter(const OSSymbol *key, OSObject *value);
    // ---- IOAudioControl hookup (ref FWBaseProp::Establish*/HookUpAudioControl/StaticValueChanged...; decomp FWBaseProp__*.c) ----
    // A property can back CoreAudio controls: changes made through the control (StaticValueChanged -> ChangedValueThroughAudioControl) are
    // injected back into the dictionary; changes made through the dictionary are pushed to the controls on the work loop.
    IOReturn EstablishVolumeControl(IOAudioEngine *engine, UInt32 usage, UInt32 channelID, UInt32 controlID,
                                    SInt32 minValue, SInt32 maxValue, SInt32 minDB, SInt32 maxDB);          // ref @ FWBaseProp
    IOReturn EstablishMuteControl(IOAudioEngine *engine, UInt32 usage, UInt32 channelID, UInt32 controlID);
    virtual IOReturn ChangedValueThroughAudioControl(IOAudioControl *c, SInt32 oldValue, SInt32 newValue);
    virtual void UpdateAudioControlsWithNewValue(OSObject *v);
    virtual void NotifyAudioControlsOfChange();                                // queue a push to the controls (ref FWBaseProp::NotifyAudioControlsOfChange)
    void NotifyAudioControlsInWorkLoop();                                      // ref: re-read the value and push it
    void DetachAudioControlsFromEngine(IOAudioEngine *engine);
    const OSSymbol *key() const { return fKey; }
    bool isImmediate() const { return fImmediate; }
    void attach(com_openaudio_FWPropertyDict *d) { fDict = d; }
protected:
    OSSymbol *fKey;                         // [+0x18]
    com_openaudio_FWPropertyDict *fDict;    // [+0x20]
    bool fImmediate;                        // [+0x28] set only by FWFunctionProp: applied on the caller's thread
    OSSet *fControls;                       // [+0x10] the IOAudioControls backing this property
    void HookUpAudioControl(IOAudioControl *c, IOAudioEngine *engine);
    static IOReturn StaticValueChanged(OSObject *target, IOAudioControl *c, SInt32 oldValue, SInt32 newValue);
};

// ref FW32BitNumberProp: value [+0x2c], setter [+0x38]. `external` lets a property live in storage owned by someone else
// (MAClockSource's value cell is device[+0x144]).
class com_openaudio_FWNumber32Prop : public com_openaudio_FWBaseProp
{
    OSDeclareDefaultStructors(com_openaudio_FWNumber32Prop)
public:
    IOReturn Init(com_openaudio_FWPropertyDict *dict, const char *key, const FWCall1 &setter, UInt32 *external = 0);
    virtual IOReturn CallGetter(const char *key, OSObject *&out) APPLE_KEXT_OVERRIDE;
    virtual IOReturn CallSetter(const OSSymbol *key, OSObject *value) APPLE_KEXT_OVERRIDE;
    UInt32 value() const { return fExternal ? *fExternal : fValue; }
    void   set(UInt32 v) { if (fExternal) *fExternal = v; else fValue = v; }
private:
    UInt32  fValue;                         // [+0x2c]
    UInt32 *fExternal;
    FWCall1 fSetter;                        // [+0x38]
};

// ref FW64BitNumberProp: value [+0x30], setter [+0x40]
class com_openaudio_FWNumber64Prop : public com_openaudio_FWBaseProp
{
    OSDeclareDefaultStructors(com_openaudio_FWNumber64Prop)
public:
    IOReturn Init(com_openaudio_FWPropertyDict *dict, const char *key, const FWCall64 &setter);
    virtual IOReturn CallGetter(const char *key, OSObject *&out) APPLE_KEXT_OVERRIDE;
    virtual IOReturn CallSetter(const OSSymbol *key, OSObject *value) APPLE_KEXT_OVERRIDE;
    UInt64 value() const { return fValue; }
    void   set(UInt64 v) { fValue = v; }
private:
    UInt64  fValue;
    FWCall64 fSetter;
};

// ref FWFunctionProp: a write-only trigger; the functor receives the raw OSObject. Immediate (applied synchronously).
class com_openaudio_FWFunctionProp : public com_openaudio_FWBaseProp
{
    OSDeclareDefaultStructors(com_openaudio_FWFunctionProp)
public:
    IOReturn Init(com_openaudio_FWPropertyDict *dict, const char *key, const FWCallObj &fn);
    virtual IOReturn CallSetter(const OSSymbol *key, OSObject *value) APPLE_KEXT_OVERRIDE;
private:
    FWCallObj fFn;                          // [+0x38]
};

// ref FWBaseArrayProp (n x uint32). The element accessors are virtual because FWChannelGroupProperty stores its elements elsewhere.
class com_openaudio_FWArrayProp : public com_openaudio_FWBaseProp
{
    OSDeclareDefaultStructors(com_openaudio_FWArrayProp)
public:
    IOReturn Init(com_openaudio_FWPropertyDict *dict, const char *key, UInt32 n, const FWCall2 &perElement);
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual IOReturn CallGetter(const char *key, OSObject *&out) APPLE_KEXT_OVERRIDE;
    virtual IOReturn CallSetter(const OSSymbol *key, OSObject *value) APPLE_KEXT_OVERRIDE;

    virtual UInt32 Get32BitValueForArrayElement(UInt32 i) const;                // vt 0x158
    virtual void   Set32BitValueFromArrayElement(UInt32 i, UInt32 v);           // vt 0x160 (stores, then calls the per-element functor)
    UInt32 Get(UInt32 i) const;                                                 // ref Get (bounds checked)
    void   Set(UInt32 i, UInt32 v);                                             // ref Set (no functor)
    void   Fill(UInt32 v);
    UInt32 count() const { return fCount; }
    IOReturn SendAllToDevice();                                                 // ref SendAllToDevice: functor(i+1, Get(i)) for all i
    void   SetAllChanged(const FWCall0 &f) { fAllChanged = f; }                // ref [+0x48]
    const FWCall2 &perElement() const { return fPer; }
    // ref FWBaseArrayProp::EstablishVolumeControls / EstablishMuteControls: one control per element, control ID = element index,
    // channel ID = firstChannel + index
    IOReturn EstablishVolumeControls(IOAudioEngine *engine, UInt32 usage, UInt32 firstChannel, SInt32 minValue, SInt32 maxValue, SInt32 minDB, SInt32 maxDB);
    IOReturn EstablishMuteControls(IOAudioEngine *engine, UInt32 usage, UInt32 firstChannel);
    virtual IOReturn ChangedValueThroughAudioControl(IOAudioControl *c, SInt32 oldValue, SInt32 newValue) APPLE_KEXT_OVERRIDE;
    virtual void UpdateAudioControlsWithNewValue(OSObject *v) APPLE_KEXT_OVERRIDE;
    void UpdateAudioControlsForChannelWithNewValue(UInt32 channel, OSObject *v);
    IOReturn OSArrayToSeriatimValues(OSObject *arr, UInt32 n);
    IOReturn SeriatimValuesToOSArray(UInt32 n, OSObject *&out);
protected:
    UInt32  fCount;                         // [+0x2c]
    UInt32 *fValues;                        // [+0x50]
    UInt32  fFirstChannel;                  // [+0x58]
    FWCall2 fPer;                           // [+0x38]
    FWCall0 fAllChanged;                    // [+0x48]
};

// ref FWChannelGroupProperty: the array's elements are one field of every channel of a group (+ optionally a second group).
class com_openaudio_FWChannelGroupProp : public com_openaudio_FWArrayProp
{
    OSDeclareDefaultStructors(com_openaudio_FWChannelGroupProp)
public:
    IOReturn Init(com_openaudio_FWPropertyDict *dict, const char *key, FWChannelGroup *group, UInt32 fieldOffset, FWChannelGroup *second);
    virtual IOReturn CallSetter(const OSSymbol *key, OSObject *value) APPLE_KEXT_OVERRIDE;
    virtual UInt32 Get32BitValueForArrayElement(UInt32 i) const APPLE_KEXT_OVERRIDE;
    virtual void   Set32BitValueFromArrayElement(UInt32 i, UInt32 v) APPLE_KEXT_OVERRIDE;
private:
    FWChannelGroup *fGroup;                 // [+0x60]
    UInt32          fField;                 // [+0x68] byte offset inside the 0x20-byte channel slot
    FWChannelGroup *fSecond;                // [+0x70]
};

// ref FWPropertyDict
class com_openaudio_FWPropertyDict : public IOEventSource
{
    OSDeclareDefaultStructors(com_openaudio_FWPropertyDict)
public:
    // ref init(IOService*) @0x9...: owner, work loop (owner->getWorkLoop()), added as an event source
    bool InitWithDevice(IOService *device);
    void Detach();                                                              // removes the event source (ref release() override)
    virtual void free() APPLE_KEXT_OVERRIDE;

    IOReturn InstallProperty(com_openaudio_FWBaseProp *p);                      // ref @0x8e..
    com_openaudio_FWNumber32Prop *AddNumber32(const char *key, const FWCall1 &setter, UInt32 *external = 0);
    com_openaudio_FWNumber64Prop *AddNumber64(const char *key, const FWCall64 &setter);
    com_openaudio_FWFunctionProp *AddFunction(const char *key, const FWCallObj &fn);
    com_openaudio_FWArrayProp    *AddArray(const char *key, UInt32 n, const FWCall2 &perElement);
    com_openaudio_FWChannelGroupProp *AddChannelGroup(const char *key, FWChannelGroup *group, UInt32 fieldOffset, FWChannelGroup *second);
    IOReturn SetProperties(OSObject *props);                                    // ref @0xa0e0
    IOReturn SetOneProperty(const OSSymbol *key, OSObject *value);              // ref SetOneProperty
    void     ModifyProperty(const OSSymbol *key, OSObject *value);              // queue (replaces an older value)
    void     InjectModifiedProperty(const OSSymbol *key, OSObject *value, bool notify);
    void     SetPropertyAndNotifyUserClients(const char *key, OSObject *value);
    void     SetPropertyAndNotifyUserClients(const OSSymbol *key, OSObject *value);
    void     SetNumberAndNotifyUserClients(const char *key, UInt32 value);     // convenience: OSNumber(value, 32)
    OSObject *CopyPropertyObject(const char *key) const { return fProps ? fProps->getObject(key) : 0; }   // the property OBJECT (not its value), borrowed
    OSObject *CopyProperty(const char *key) const;                              // caller releases (the reference leaks one reference here)
    OSDictionary *CopyAllProperties() const;
    void     CopyAllPropertiesToDictionary(OSDictionary *d) const;
    void     AddUpdateForAudioControl(com_openaudio_FWBaseProp *p);             // audio-control slice
    void     NoteChangesAvailable() { signalWorkAvailable(); }

    // User-client notification (ref CacheModifiedPropertyDictionaryInUserClients @0x9a2e, SendPropertyChangedMessagesToUserClients @0x9acc,
    // SendAllPropertiesToUserClients @0xa01a, SendArbitraryPropertyToUserClients @0x9d88): every client of this device gets the changes
    void CacheModifiedPropertyDictionaryInUserClients(OSDictionary *d);
    void SendPropertyChangedMessagesToUserClients();
    void SendAllPropertiesToUserClients();
    void SendArbitraryPropertyToUserClients(const char *key, OSObject *value);

    // Test-only access: the number of queued keys
    UInt32 queuedCount() const { return fQueue ? fQueue->getCount() : 0; }

protected:
    virtual bool checkForWork() APPLE_KEXT_OVERRIDE;                            // ref @0x9...
    void CheckModifiedProperties(bool notify);
    void ApplyModifiedProperties(bool notify);
    void CheckAudioControlUpdates();
private:
    IOService    *fDevice;                  // [+0x48]
    OSSet        *fImmediate;               // [+0x50]
    OSDictionary *fQueue;                   // [+0x58]
    OSDictionary *fProps;                   // [+0x60]
    IOWorkLoop   *fLoop;                    // [+0x68]
    volatile UInt32 fBusy;                  // [+0x70]
    OSSet        *fAudioUpdates;            // [+0x78]
};

#endif
