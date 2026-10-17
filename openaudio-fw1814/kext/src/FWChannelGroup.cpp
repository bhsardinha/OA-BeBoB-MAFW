#include "FWChannelGroup.h"
#include "Trace.h"

// ref BaseChannelGroup::Solo @ decomp m_audio_FWb__BaseChannelGroup__Solo.c: any channel of THIS group has solo != 0 (the caller passes the
// parent, see AdjustedLevels).
bool FWChannelGroup::Solo() const
{
    FWChannelGroup *self = const_cast<FWChannelGroup *>(this);
    const UInt32 n = Count();
    for (UInt32 i = 0; i < n; i++)
        if (self->ChannelAtIndex(i)->solo != 0) return true;
    return false;
}

// ref AdjustedLevelForChannel (decomp m_audio_FWb__BaseChannelGroup__AdjustedLevelForChannel.c)
SInt32 FWChannelGroup::AdjustedLevelForChannel(const FWChannel &ch, bool anySolo, bool muteAll, bool dim) const
{
    SInt32 r;
    if (muteAll || (anySolo && ch.solo == 0)) return fMin;
    if (ch.mute == 0) {
        SInt32 v = ch.balance + ch.level;
        r = fMin;
        if (v >= fMin) { r = v; if (fMax < v) r = fMax; }
    } else {
        r = fMin;
    }
    if (dim) { SInt32 d = r - 0x14; r = (d < fMin) ? fMin : d; }
    return r;
}

SInt32 *FWChannelGroup::AdjustedLevels() const
{
    FWChannelGroup *self = const_cast<FWChannelGroup *>(this);
    if (!self->fParent) self->fParent = self;                    // ref: [+8] := this when NULL
    const UInt32 n = Count();
    SInt32 *out = static_cast<SInt32 *>(IOMalloc((size_t)n * 4));
    if (!out) return 0;
    const bool solo = fParent->Solo();
    for (UInt32 i = 0; i < n; i++)
        out[i] = AdjustedLevelForChannel(*self->ChannelAtIndex(i), solo, fMuteAll, fDim);
    return out;
}

SInt16 *FWChannelGroup::AdjustedAndMultipliedLevels() const
{
    FWChannelGroup *self = const_cast<FWChannelGroup *>(this);
    if (!self->fParent) self->fParent = self;
    const UInt32 n = Count();
    SInt16 *out = static_cast<SInt16 *>(IOMalloc((size_t)n * 2));
    if (!out) return 0;
    const bool solo = fParent->Solo();
    for (UInt32 i = 0; i < n; i++)
        out[i] = (SInt16)((SInt16)AdjustedLevelForChannel(*self->ChannelAtIndex(i), solo, fMuteAll, fDim) * (SInt16)fMultiplier);
    return out;
}

// ref BaseChannelGroup::SendLevelsToDevice @ decomp m_audio_FWb__BaseChannelGroup__SendLevelsToDevice.c. With a batch writer the
// adjusted levels are multiplied (x0x80), byte-swapped to big-endian int16 and written Count()/2 quadlets at the group's register offset;
// without one each channel goes through the per-channel setter (index is 1-based).
void FWChannelGroup::SendLevelsToDevice(const SInt32 *)
{
    const UInt32 n = Count();
    if (!fBatch.isSet()) {
        SInt32 *lv = AdjustedLevels();
        if (!lv) return;
        for (UInt32 i = 0; i < n; i++) fPerChannel(i + 1, (UInt32)lv[i]);
        IOFree(lv, (size_t)n * 4);
        return;
    }
    SInt16 *lv = AdjustedAndMultipliedLevels();
    if (!lv) return;
    for (UInt32 i = 0; i < n; i++) lv[i] = (SInt16)(((UInt16)lv[i] << 8) | ((UInt16)lv[i] >> 8));
    fBatch(fRegisterOffset, n >> 1, lv);
    IOFree(lv, (size_t)n * 2);
}

void FWChannelGroup::SetAllLevels()  { for (UInt32 i = 0, n = Count(); i < n; i++) ChannelAtIndex(i)->level = 0; }
void FWChannelGroup::SetAllBalance() { for (UInt32 i = 0, n = Count(); i < n; i++) ChannelAtIndex(i)->balance = 0; }
void FWChannelGroup::SetAllMute()    { for (UInt32 i = 0, n = Count(); i < n; i++) ChannelAtIndex(i)->mute = 0; }
void FWChannelGroup::SetAllSolo()    { for (UInt32 i = 0, n = Count(); i < n; i++) ChannelAtIndex(i)->solo = 0; }
void FWChannelGroup::SetAllLinked()  { for (UInt32 i = 0, n = Count(); i < n; i++) ChannelAtIndex(i)->link = 1; }
void FWChannelGroup::ResetAll() { SetAllLevels(); SetAllBalance(); SetAllMute(); SetAllSolo(); SetAllLinked(); }

// ---- FWGroupChannelGroup ---------------------------------------------------------------------------------------------------------------

// ref GroupChannelGroup::Init: enabled, no parent, each member's parent := this
void FWGroupChannelGroup::Init(FWChannelGroup *a, FWChannelGroup *b, FWChannelGroup *c)
{
    fEnabled = true; fParent = 0; fDim = false; fMuteAll = false;
    fG[0] = a; fG[1] = b; fG[2] = c;
    for (int i = 0; i < 3; i++) if (fG[i]) fG[i]->SetParent(this);
}

UInt32 FWGroupChannelGroup::Count() const
{
    UInt32 n = 0;
    for (int i = 0; i < 3; i++) if (fG[i]) n += fG[i]->Count();
    return n;
}

FWChannel *FWGroupChannelGroup::ChannelAtIndex(UInt32 i)
{
    for (int k = 0; k < 3; k++) {
        if (!fG[k]) continue;
        const UInt32 c = fG[k]->Count();
        if (i < c) return fG[k]->ChannelAtIndex(i);
        i -= c;
    }
    OA_LOG("FWGroupChannelGroup: no channel at index");           // ref panics ("no channel at index", TChannelGroup.h:444); never panic here
    for (int k = 0; k < 3; k++) if (fG[k]) return fG[k]->ChannelAtIndex(0);
    return 0;
}

void FWGroupChannelGroup::SendLevelsToDevice(const SInt32 *)
{
    for (int i = 0; i < 3; i++) if (fG[i]) fG[i]->SendLevelsToDevice(0);
}
