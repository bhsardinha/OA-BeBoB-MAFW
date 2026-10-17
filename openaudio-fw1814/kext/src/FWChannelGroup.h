// FWChannelGroup -- mixer channel groups. Mirrors m_audio_FWb::{BaseChannelGroup, ConcreteChannelGroup<N>, GroupChannelGroup}
// (A1 Q-ME §2; decomp m_audio_FWb__BaseChannelGroup__*.c, ...ConcreteChannelGroup6u__*.c, ...GroupChannelGroup__*.c).
// The classes are plain C++ (not OSObject); they are embedded by value in the mixer object, which is zero-initialised by OSObject.
#ifndef OA_FWCHANNELGROUP_H
#define OA_FWCHANNELGROUP_H

#include <libkern/OSTypes.h>
#include <libkern/c++/OSMetaClass.h>
#include <IOKit/IOLib.h>
#include "FWFunctor.h"

// One channel slot, 0x20 bytes (ref ConcreteChannelGroup<N> ctor: three zeroed qwords per slot; field offsets from AdjustedLevelForChannel
// and the SetAll* helpers). The 64-bit cells are written through 32-bit stores by FWChannelGroupProperty, so only their low half is used.
struct FWChannel {
    SInt32 level;       // +0x00 (0.5 dB steps, 0 = unity, -255 = minimum)
    SInt32 balance;     // +0x04
    UInt64 mute;        // +0x08
    UInt64 solo;        // +0x10
    UInt64 link;        // +0x18
};

class FWChannelGroup
{
public:
    FWChannelGroup() : fParent(0), fDim(false), fMuteAll(false), fEnabled(true), fMin((SInt32)0xffffff01), fMax(0),
                       fRegisterOffset(0), fMultiplier(0) {}
    virtual ~FWChannelGroup() {}

    virtual UInt32     Count() const = 0;                       // vt+0x10
    virtual FWChannel *ChannelAtIndex(UInt32 i) = 0;            // vt+0x18
    virtual void       SendLevelsToDevice(const SInt32 *before);   // vt+0x28 (ref BaseChannelGroup; `before` is unused in the reference)

    // ref AdjustedLevels @ BaseChannelGroup: heap array of Count() adjusted levels (caller IOFree()s it with AdjustedBytes())
    SInt32 *AdjustedLevels() const;
    UInt32  AdjustedBytes() const { return Count() * 4; }
    // ref AdjustedAndMultipliedLevels: int16 array, adjusted * int16(multiplier)
    SInt16 *AdjustedAndMultipliedLevels() const;
    SInt32  AdjustedLevelForChannel(const FWChannel &ch, bool anySolo, bool muteAll, bool dim) const;
    bool    Solo() const;                                       // any channel of the PARENT group has solo set

    // ref SetAll*: the reference stores literals (decomp: level 0, balance 0, mute 0, solo 0, link 1) whatever the bool argument is
    void SetAllLevels();
    void SetAllBalance();
    void SetAllMute();
    void SetAllSolo();
    void SetAllLinked();
    void ResetAll();                                            // the five calls of FWSettingsLevels::ResetToFactorySettings

    // ref SetupBatchChangeMethod @0xe184: register offset, multiplier 0x80, the writer
    void SetupBatch(const FWCallBatch &writer, UInt32 registerOffset) { fBatch = writer; fRegisterOffset = registerOffset; fMultiplier = 0x80; }
    void SetPerChannel(const FWCall2 &f) { fPerChannel = f; }

    void SetLimits(SInt32 minLevel, SInt32 maxLevel) { fMin = minLevel; fMax = maxLevel; }   // ref [+0x14] / [+0x18]
    void SetParent(FWChannelGroup *p) { fParent = p; }
    void SetDim(bool d) { fDim = d; }                           // ref [+0x10]
    void SetMuteAll(bool m) { fMuteAll = m; }                   // ref [+0x11]
    void SetEnabled(bool e) { fEnabled = e; }                   // ref [+0x12]
    bool Enabled() const { return fEnabled; }
    SInt32 minLevel() const { return fMin; }
    SInt32 maxLevel() const { return fMax; }

protected:
    FWChannelGroup *fParent;        // [+0x08]
    bool    fDim;                   // [+0x10]
    bool    fMuteAll;               // [+0x11]
    bool    fEnabled;               // [+0x12]
    SInt32  fMin;                   // [+0x14]
    SInt32  fMax;                   // [+0x18]
    FWCall2 fPerChannel;            // [+0x20/0x28]
    FWCallBatch fBatch;             // [+0x30/0x38]
    UInt32  fRegisterOffset;        // [+0x40]
    UInt32  fMultiplier;            // [+0x44]
};

template <UInt32 N>
class FWConcreteChannelGroup : public FWChannelGroup
{
public:
    FWConcreteChannelGroup() { for (UInt32 i = 0; i < N; i++) { fCh[i].level = 0; fCh[i].balance = 0; fCh[i].mute = 0; fCh[i].solo = 0; fCh[i].link = 0; } }
    virtual UInt32 Count() const APPLE_KEXT_OVERRIDE { return N; }
    virtual FWChannel *ChannelAtIndex(UInt32 i) APPLE_KEXT_OVERRIDE { return &fCh[i < N ? i : 0]; }   // ref ChannelAtIndex @0xac9e clamp
private:
    FWChannel fCh[N];
};

// ref GroupChannelGroup: a union of up to three groups. Init makes each member's parent the union (so Solo() spans the union).
class FWGroupChannelGroup : public FWChannelGroup
{
public:
    FWGroupChannelGroup() { fG[0] = fG[1] = fG[2] = 0; }
    void Init(FWChannelGroup *a, FWChannelGroup *b, FWChannelGroup *c);
    virtual UInt32 Count() const APPLE_KEXT_OVERRIDE;
    virtual FWChannel *ChannelAtIndex(UInt32 i) APPLE_KEXT_OVERRIDE;
    virtual void SendLevelsToDevice(const SInt32 *before) APPLE_KEXT_OVERRIDE;
private:
    FWChannelGroup *fG[3];
};

#endif
