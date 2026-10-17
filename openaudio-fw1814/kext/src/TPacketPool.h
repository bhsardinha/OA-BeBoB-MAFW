// TPacketPool -- AM824 packet backing store for one DCL program, including the packet-index table of
// m_audio_b_FWTPacketIndex. Mirrors m_audio_FWb::TPacketPool (A1 Q17, R8, R10, Q-P1P2 §3) and
// FWTPacketIndex (Ghidra decomp q-mb-dcl-callbacks.c). One allocation holds every packet buffer AND the DCL timestamp
// slots (last page), which makes IODCLProgram::generateBufferMap accept the chain on 10.13.6 (Q-P1P2 §2).
#ifndef OA_TPACKETPOOL_H
#define OA_TPACKETPOOL_H

#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/firewire/IOFireWireBus.h>

class TPacketPool
{
public:
    TPacketPool();
    ~TPacketPool() { Release(); }

    // ref Initialize @0xa3e6. count = total packets, packetBytes = 8 (CIP) + payload (TX) or 12 + payload (RX).
    // audioQ/midiQ = quadlets per data block (cipDataQuadlets / cipPadQuadlets). Returns false and leaves the pool
    // released on failure.
    bool Initialize(IOFireWireBus *bus, UInt32 count, UInt32 packetBytes, UInt32 audioQ, UInt32 midiQ);
    void Release();                                   // ref ForceReleaseMemory @0xa4e8; NULL-safe, idempotent
    bool isInitialized() const { return fInitialized; }
    void SetAudioPool(bool audio) { fAudio = audio; } // ref [+0x39]; set after Initialize (SetPacketSize @0x25e83)

    UInt8  *GetIndexedPacket(UInt32 index) const;     // ref @0xa5bc
    UInt32 *TimestampSlot(UInt32 index) const;        // last page of the allocation (ref SetPacketSize @0x25e56)
    void    ZeroElements(UInt32 group);               // ref @0xa5e6: AM824 silence pre-fill of one group
    void    ZeroAll();

    // --- packet index (FWTPacketIndex) ---
    // ref SetValidPackets @0x10504. offset = number of leading primers skipped this lap; n = packets generated;
    // valid[j] = packet j carries data (sync), not NODATA.
    void    SetValidPackets(UInt32 group, UInt32 offset, UInt32 n, const bool *valid);
    UInt32  GetIndexOfActivePacket(UInt32 group, UInt32 i) const;      // ref @0x1055e
    UInt8  *GetIndexedActivePacket(UInt32 group, UInt32 i) const;      // ref @0xa784
    // ref @0xa7aa: packet + frame holding sample `sampleIndex` (2048-sample groups), pointer to the frame's first quadlet.
    UInt32 *GetPacketFrameContainingSampleIndex(UInt32 sampleIndex, UInt32 samplesPerPacket, UInt32 &framesLeft,
                                                UInt32 **outPacket, UInt32 *outFrame) const;
    // ref InitGroupCIPsForOutput @0xa8b0: writes the 8-byte CIP header of the group's packet 0 and of its n active packets.
    void    InitGroupCIPsForOutput(UInt32 group, UInt32 n, const UInt64 *hdrs);

    UInt32 totalPackets() const    { return fCount; }
    UInt32 packetsPerGroup() const { return fPerGroup; }
    UInt32 packetBytes() const     { return fPacketBytes; }
    UInt32 pages() const           { return fPages; }
    UInt32 activeOffset(UInt32 group) const { return group < 4 ? fActiveOffset[group] : 0; }

private:
    IOBufferMemoryDescriptor *fMem;     // [+0x00]
    UInt8   *fBase;                     // [+0x10]
    UInt32   fCount;                    // [+0x18]
    UInt32   fPerGroup;                 // [+0x1c] = count / 4
    UInt32   fPacketBytes;              // [+0x20]
    UInt32   fPages;                    // [+0x24] = count / perPage + 2
    UInt32   fPerPage;                  // [+0x28] = floor(4096 / packetBytes)
    UInt32   fAudioQ;                   // [+0x2c]
    UInt32   fMidiQ;                    // [+0x30]
    bool     fInitialized;              // [+0x38]
    bool     fAudio;                    // [+0x39]
    // FWTPacketIndex: [+0x18+g*4] active offset, [+0x30+g*8] per-group list of perGroup entries.
    UInt32   fActiveOffset[4];
    UInt32  *fLists;                    // 4 * perGroup UInt32
    size_t   fListBytes;
};

#endif
