#include "TPacketPool.h"
#include "Trace.h"
#include <kern/task.h>
#include <IOKit/IOLib.h>

TPacketPool::TPacketPool() : fMem(0), fBase(0), fCount(0), fPerGroup(0), fPacketBytes(0), fPages(0), fPerPage(0),
                             fAudioQ(0), fMidiQ(0), fInitialized(false), fAudio(false), fLists(0), fListBytes(0)
{
    fActiveOffset[0] = fActiveOffset[1] = fActiveOffset[2] = fActiveOffset[3] = 0;
}

bool TPacketPool::Initialize(IOFireWireBus *bus, UInt32 count, UInt32 packetBytes, UInt32 audioQ, UInt32 midiQ)
{
    Release();
    if (!bus || count < 4 || packetBytes == 0 || packetBytes > 4096 || audioQ == 0) return false;   // ref fails on audioQ == 0

    fPacketBytes = packetBytes;
    fCount       = count;
    fPerGroup    = count / 4;
    fAudioQ      = audioQ;
    fMidiQ       = midiQ;
    fPerPage     = 4096 / packetBytes;
    fPages       = count / fPerPage + 2;          // integer division + 2 (R8: ref divl @0xa461)

    // packet-index lists first (ref: FWTPacketIndex::withElementCount before the memory allocation)
    fListBytes = (size_t)count * sizeof(UInt32);
    fLists = static_cast<UInt32 *>(IOMalloc(fListBytes));
    if (!fLists) { Release(); return false; }
    bzero(fLists, fListBytes);

    // P2 constraint 1: mask comes from the bus object, not a constant; page-aligned; options 0 (KMA_LOMEM path).
    UInt64 mask = bus->getFireWirePhysicalBufferMask() & ~(UInt64)0xFFF;
    fMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, 0, (mach_vm_size_t)fPages << 12, mask);
    if (!fMem) { OA_LOG("TPacketPool: inTaskWithPhysicalMask(%u pages, mask %llx) returned NULL", fPages, mask); Release(); return false; }
    if (fMem->prepare() != kIOReturnSuccess) { OA_LOG("TPacketPool: prepare failed"); Release(); return false; }
    fBase = static_cast<UInt8 *>(fMem->getBytesNoCopy());
    if (!fBase) { Release(); return false; }
    bzero(fBase, (size_t)fPages << 12);           // not relied upon from the allocator
    fInitialized = true;
    OA_LOG("TPacketPool: %u packets x %u B, %u/page, %u pages (%u KiB), base %p", fCount, fPacketBytes, fPerPage, fPages,
           fPages * 4, fBase);
    return true;
}

void TPacketPool::Release()
{
    if (fMem) {
        fMem->complete();
        fMem->release();
        fMem = 0;
    }
    if (fLists) {
        IOFree(fLists, fListBytes);
        fLists = 0;
        fListBytes = 0;
    }
    fBase = 0;
    fInitialized = false;
    fActiveOffset[0] = fActiveOffset[1] = fActiveOffset[2] = fActiveOffset[3] = 0;
}

UInt8 *TPacketPool::GetIndexedPacket(UInt32 index) const
{
    if (!fInitialized || index >= fCount) return 0;
    return fBase + (size_t)(index / fPerPage) * 4096 + (size_t)(index % fPerPage) * fPacketBytes;
}

UInt32 *TPacketPool::TimestampSlot(UInt32 index) const
{
    if (!fInitialized || index >= 1024) return 0;                 // last page holds 1024 slots; we use 4 (TX) / 64 (RX)
    return reinterpret_cast<UInt32 *>(fBase + ((size_t)fPages << 12) - 0x1000) + index;
}

// ref ZeroElements (Q17): audio path = per packet, skip the 8-byte CIP header, then per data block write cipDataQ quadlets
// of 0x000000CF and cipPadQ quadlets of 0x00000080 (host-endian stores; wire bytes CF 00 00 00 / 80 00 00 00).
// Raw path (non-audio pool) = bzero(packet). Also run from every TX callback (R10); the converters later overwrite
// handled slots with 0x40 / 0x80.
void TPacketPool::ZeroElements(UInt32 group)
{
    if (!fInitialized || group >= 4) return;
    const UInt32 blockQ = fAudioQ + fMidiQ;
    for (UInt32 i = 0; i < fPerGroup; i++) {
        UInt8 *p = GetIndexedPacket(group * fPerGroup + i);
        if (!p) continue;
        if (!fAudio) { bzero(p, fPacketBytes); continue; }
        UInt32 *q = reinterpret_cast<UInt32 *>(p) + 2;
        const UInt32 blocks = ((fPacketBytes >> 2) - 2) / blockQ;
        for (UInt32 b = 0; b < blocks; b++) {
            for (UInt32 a = 0; a < fAudioQ; a++) *q++ = 0x000000CF;
            for (UInt32 m = 0; m < fMidiQ; m++)  *q++ = 0x00000080;
        }
    }
}

void TPacketPool::ZeroAll()
{
    for (UInt32 g = 0; g < 4; g++) ZeroElements(g);
}

// ref FWTPacketIndex::SetValidPackets @0x10504, mirrored including its asymmetry: an invalid packet zeroes list[j];
// a valid packet writes list[k++] = (j == 0 ? 0 : offset + j). (So the first valid packet maps to the group's packet 0,
// which the DCL ring sends as the LAST packet of the previous bank: see Q-MB-dcl-callbacks.md "pool offset by one".)
void TPacketPool::SetValidPackets(UInt32 group, UInt32 offset, UInt32 n, const bool *valid)
{
    if (!fLists || n > fPerGroup || group >= 4) return;
    fActiveOffset[group] = offset;
    UInt32 *list = fLists + (size_t)group * fPerGroup;
    UInt32 k = 0;
    for (UInt32 j = 0; j < n; j++) {
        if (!valid[j]) {
            list[j] = 0;
        } else {
            list[k] = (j == 0) ? 0 : offset + j;
            k++;
        }
    }
}

UInt32 TPacketPool::GetIndexOfActivePacket(UInt32 group, UInt32 i) const
{
    if (!fLists || i > fPerGroup || group >= 4) return 0;
    return fPerGroup * group + fLists[(size_t)group * fPerGroup + i];
}

UInt8 *TPacketPool::GetIndexedActivePacket(UInt32 group, UInt32 i) const
{
    return GetIndexedPacket(GetIndexOfActivePacket(group, i));
}

UInt32 *TPacketPool::GetPacketFrameContainingSampleIndex(UInt32 sampleIndex, UInt32 spp, UInt32 &framesLeft,
                                                         UInt32 **outPacket, UInt32 *outFrame) const
{
    UInt32 frame = sampleIndex % spp;
    UInt32 idx = GetIndexOfActivePacket(sampleIndex >> 11, sampleIndex / spp - (0x800 / spp) * (sampleIndex >> 11));
    UInt32 *pkt = reinterpret_cast<UInt32 *>(GetIndexedPacket(idx));
    if (outPacket) *outPacket = pkt;
    if (outFrame) *outFrame = frame;
    framesLeft = spp - frame;
    return pkt ? pkt + ((fMidiQ + fAudioQ) * frame + 2) : 0;
}

void TPacketPool::InitGroupCIPsForOutput(UInt32 group, UInt32 n, const UInt64 *hdrs)
{
    if (!fInitialized) return;
    UInt32 off = fActiveOffset[group < 4 ? group : 0];
    if (off + n > fPerGroup) return;
    if (n == 0) return;                          // ref would copy an uninitialised hdrs[0]; nothing meaningful to write
    UInt32 base = fPerGroup * group;
    UInt64 *p = reinterpret_cast<UInt64 *>(GetIndexedPacket(base));
    if (p) *p = hdrs[0];
    UInt32 idx = off + base;
    for (UInt32 i = 0; i < n; i++, idx++) {
        UInt64 *q = reinterpret_cast<UInt64 *>(GetIndexedPacket(idx));
        if (q) *q = hdrs[i];
    }
}
