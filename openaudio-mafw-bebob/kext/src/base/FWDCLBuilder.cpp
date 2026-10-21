#include "FWDCLBuilder.h"

static inline size_t align8(size_t n) { return (n + 7) & ~(size_t)7; }

FWDCLBuilder::FWDCLBuilder() : fHead(0), fTail(0), fArena(0), fCap(0), fUsed(0), fLabelCount(0), fJumpCount(0) {}

bool FWDCLBuilder::Init(size_t arenaBytes)
{
    Free();
    fArena = static_cast<UInt8 *>(IOMalloc(arenaBytes));
    if (!fArena) return false;
    bzero(fArena, arenaBytes);
    fCap = arenaBytes;
    return true;
}

void FWDCLBuilder::Free()
{
    if (fArena) {
        IOFree(fArena, fCap);
        fArena = 0;
    }
    fHead = fTail = 0;
    fCap = fUsed = 0;
    fLabelCount = fJumpCount = 0;
}

void *FWDCLBuilder::AllocRaw(size_t bytes)
{
    bytes = align8(bytes);
    if (!fArena || fUsed + bytes > fCap) return 0;
    void *p = fArena + fUsed;
    fUsed += bytes;
    return p;
}

void *FWDCLBuilder::Alloc(size_t bytes, UInt32 opcode)
{
    DCLCommand *n = static_cast<DCLCommand *>(AllocRaw(bytes));
    if (!n) return 0;
    n->opcode = opcode;                            // pNextDCLCommand / compilerData stay zero
    if (fTail) fTail->pNextDCLCommand = n; else fHead = n;
    fTail = n;
    return n;
}

DCLLabel *FWDCLBuilder::AddLabel()
{
    DCLLabel *n = static_cast<DCLLabel *>(Alloc(sizeof(DCLLabel), kDCLLabelOp));
    if (n && fLabelCount < kMaxIndexed) fLabels[fLabelCount++] = n;
    return n;
}

DCLJump *FWDCLBuilder::AddJump()
{
    DCLJump *n = static_cast<DCLJump *>(Alloc(sizeof(DCLJump), kDCLJumpOp));
    if (n && fJumpCount < kMaxIndexed) fJumps[fJumpCount++] = n;
    return n;
}

void FWDCLBuilder::ModifyJumpLabel(IOFWLocalIsochPort *port, UInt32 jumpIdx, UInt32 labelIdx)
{
    DCLJump *j = jump(jumpIdx);
    DCLLabel *l = label(labelIdx);
    if (!j || !l) return;                       // ref falls back to element 0 on a bad index; we refuse instead
    j->pJumpDCLLabel = l;
    if (port) {
        DCLCommand *changed = reinterpret_cast<DCLCommand *>(j);
        port->notify(kFWDCLModifyNotification, &changed, 1);
    }
}

DCLSetTagSyncBits *FWDCLBuilder::AddSetTagSyncBits(UInt16 tag, UInt16 sync)
{
    DCLSetTagSyncBits *n = static_cast<DCLSetTagSyncBits *>(Alloc(sizeof(DCLSetTagSyncBits), kDCLSetTagSyncBitsOp));
    if (n) { n->tagBits = tag; n->syncBits = sync; }
    return n;
}

DCLCallProc *FWDCLBuilder::AddCallProc(DCLCallCommandProc *proc, void *procData)
{
    DCLCallProc *n = static_cast<DCLCallProc *>(Alloc(sizeof(DCLCallProc), kDCLCallProcOp));
    if (n) { n->proc = proc; n->procData = procData; }
    return n;
}

// The UpdateDCLList node lists the nodes whose contents the driver will rewrite while the program runs
// (packet sizes via SetValidPackets, buffers); ref AppendDCLCommandList @0x6a37 sizes the pointer array as
// (sum of preceding counts) * 8 and fills it with every node of the call.
bool FWDCLBuilder::AddTransferPackets(UInt32 opcode, UInt32 count)
{
    DCLCommand **list = static_cast<DCLCommand **>(AllocRaw((size_t)count * sizeof(DCLCommand *)));
    if (!list) return false;
    for (UInt32 i = 0; i < count; i++) {
        DCLTransferPacket *t = static_cast<DCLTransferPacket *>(Alloc(sizeof(DCLTransferPacket), opcode));
        if (!t) return false;
        list[i] = reinterpret_cast<DCLCommand *>(t);
    }
    DCLUpdateDCLList *u = static_cast<DCLUpdateDCLList *>(Alloc(sizeof(DCLUpdateDCLList), kDCLUpdateDCLListOp));
    if (!u) return false;
    u->dclCommandList = list;
    u->numDCLCommands = count;
    return true;
}

bool FWDCLBuilder::AddTimestampedTransferPackets(UInt32 opcode, UInt32 count)
{
    DCLCommand **list = static_cast<DCLCommand **>(AllocRaw((size_t)(count + 1) * sizeof(DCLCommand *)));
    if (!list) return false;
    for (UInt32 i = 0; i < count; i++) {
        DCLTransferPacket *t = static_cast<DCLTransferPacket *>(Alloc(sizeof(DCLTransferPacket), opcode));
        if (!t) return false;
        list[i] = reinterpret_cast<DCLCommand *>(t);
    }
    DCLPtrTimeStamp *ts = static_cast<DCLPtrTimeStamp *>(Alloc(sizeof(DCLPtrTimeStamp), kDCLPtrTimeStampOp));
    if (!ts) return false;
    list[count] = reinterpret_cast<DCLCommand *>(ts);                  // ref post-fixup @0x6b36: list covers N+1 nodes
    DCLUpdateDCLList *u = static_cast<DCLUpdateDCLList *>(Alloc(sizeof(DCLUpdateDCLList), kDCLUpdateDCLListOp));
    if (!u) return false;
    u->dclCommandList = list;
    u->numDCLCommands = count + 1;
    return true;
}

size_t FWDCLBuilder::BytesForSegment(UInt32 packets, bool timestamped, bool callProc, bool tagSync)
{
    size_t n = align8(sizeof(DCLLabel));
    if (tagSync)   n += align8(sizeof(DCLSetTagSyncBits));
    n += align8((size_t)(packets + (timestamped ? 1 : 0)) * sizeof(DCLCommand *));    // UpdateDCLList pointer array
    n += (size_t)packets * align8(sizeof(DCLTransferPacket));
    if (timestamped) n += align8(sizeof(DCLPtrTimeStamp));
    n += align8(sizeof(DCLUpdateDCLList));
    if (callProc)  n += align8(sizeof(DCLCallProc));
    return n;
}

size_t FWDCLBuilder::BytesForJump() { return align8(sizeof(DCLJump)); }
