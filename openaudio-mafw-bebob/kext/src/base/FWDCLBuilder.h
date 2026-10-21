// FWDCLBuilder -- classic-DCL node arena. Mirrors m_audio_b_FWDCLBuilder (A1 Q1a; Add* primitives @0x6ad8-0x7014).
// Opcode numbers are the IOKit enum from <IOKit/firewire/IOFireWireFamilyCommon.h> (10.13 SDK); the reference's builder
// numbers are the same values (Q1b Correction: label 9, jump 10, set-tag-sync 11).
//
// Deviation from the reference (mechanical, not a 10.13 constraint): the reference grows a TDataBuffer arena
// (TDataBuffer::SetBufferSize 0x14000 TX / 0x10000 RX; Q-DCL follow-up 3 flags that 32 kHz TX overshoots 0x14000).
// The clone computes the exact arena size up front and makes one fixed allocation, so node addresses never move.
#ifndef OA_FWDCLBUILDER_H
#define OA_FWDCLBUILDER_H

#include <IOKit/IOLib.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>
#include <IOKit/firewire/IOFWLocalIsochPort.h>

class FWDCLBuilder
{
public:
    FWDCLBuilder();
    ~FWDCLBuilder() { Free(); }

    bool Init(size_t arenaBytes);                 // single zeroed allocation
    void Free();                                  // NULL-safe, idempotent
    DCLCommand *Head() const { return fHead; }
    UInt32 labelCount() const { return fLabelCount; }
    UInt32 jumpCount() const { return fJumpCount; }
    DCLLabel *label(UInt32 i) const { return i < fLabelCount ? fLabels[i] : 0; }
    DCLJump  *jump(UInt32 i) const { return i < fJumpCount ? fJumps[i] : 0; }
    // ref FWDCLBuilder::ModifyJumpLabel @ Ghidra FWDCLBuilder__ModifyJumpLabel.c: re-target jump[jumpIdx] to label[labelIdx]
    // (pJumpDCLLabel is written in place) and tell the running port (notify kFWDCLModifyNotification).
    void ModifyJumpLabel(IOFWLocalIsochPort *port, UInt32 jumpIdx, UInt32 labelIdx);
    size_t Used() const { return fUsed; }
    size_t Capacity() const { return fCap; }

    // Node primitives. Each appends to the chain; return NULL if the arena is exhausted (callers abort the build).
    DCLLabel    *AddLabel();                                               // @0x6ea0
    DCLJump     *AddJump();                                                // @0x7014
    DCLSetTagSyncBits *AddSetTagSyncBits(UInt16 tag, UInt16 sync);         // @0x6d24
    // N transfer DCLs (opcode = kDCLSendPacketStartOp TX / kDCLReceivePacketStartOp RX) + one UpdateDCLList(N)   @0x6b5a
    bool AddTransferPackets(UInt32 opcode, UInt32 count);
    // N transfer DCLs + PtrTimeStamp + UpdateDCLList(N+1, includes the timestamp node)                           @0x6ad8
    bool AddTimestampedTransferPackets(UInt32 opcode, UInt32 count);
    DCLCallProc *AddCallProc(DCLCallCommandProc *proc, void *procData);

    // Size helpers so callers can compute the arena exactly.
    static size_t BytesForSegment(UInt32 packets, bool timestamped, bool callProc, bool tagSync);
    static size_t BytesForJump();

private:
    void *AllocRaw(size_t bytes);                 // zeroed, 8-aligned, not linked (pointer arrays)
    void *Alloc(size_t bytes, UInt32 opcode);     // AllocRaw + link after fTail
    DCLCommand *fHead;
    DCLCommand *fTail;
    UInt8      *fArena;
    size_t      fCap, fUsed;
    // Every label/jump added, in creation order (ref builder arrays at +0x28 and +0x138).
    enum { kMaxIndexed = 64 };
    DCLLabel   *fLabels[kMaxIndexed];
    DCLJump    *fJumps[kMaxIndexed];
    UInt32      fLabelCount, fJumpCount;
};

#endif
