// FW410MixerRegs -- the 410's mixer register window and the words its own setters build (module models/FW410), kernel-free for the host test.
// All writes go through CustomMixerWriteQuads (vt 0xba8) = async block write to 0xFFC7:0x00700000 + offset, the FW1814's mechanism (Q-410-0 §5).
// Offsets and bit layouts from the decompiles of com_m_audio_FW410Device::{SetInputLevel @0x14cf8, SetInputPan @0x14e7a, SetInputRouting
// @0x14d5a, SetSWReturnRouting @0x14076, BatchSetSWReturnRouting @0x14268, SetHeadphoneSource @0x144bc} and the batch-writer offsets of
// InitializeUserSettableProperties @0x14fca (asm). The words are host-order UInt32 / UInt16 shadows written as they are in memory.
#ifndef OA_FW410MIXERREGS_H
#define OA_FW410MIXERREGS_H

#include <stdint.h>

enum {
    kFW410RegSWReturnLevels = 0x00,   // batch writer of the SW-return group (10 levels, 5 quadlets)
    kFW410RegOutputLevels   = 0x14,   // batch writer of the output group (10 levels)
    kFW410RegInputLevels    = 0x28,   // batch writer of the input group (4 levels); SetInputLevel writes 0x28 (ch 1-2) / 0x2c (ch 3-4)
    kFW410RegInputPan       = 0x38,   // SetInputPan: 0x38 (inputs 1-2), 0x3c (inputs 3-4)
    kFW410RegInputRouting   = 0x5c,   // SetInputRouting: 2 quadlets (analog in, S/PDIF in)
    kFW410RegSWRouting      = 0x64,   // BatchSetSWReturnRouting: 1 quadlet
    kFW410RegPhonesSource   = 0x68,   // SetHeadphoneSource: 1 quadlet
};

// A level / pan value as the batch path encodes it: big-endian int16 of v * 0x80, held in a little-endian UInt16 (decompile: (v >> 1) & 0xff
// + (v << 15)).
static inline uint16_t FW410Level16(uint32_t v) { return (uint16_t)(((v >> 1) & 0xff) + (uint16_t)(v << 15)); }
static inline uint16_t FW410Pan16(uint32_t v)   { return FW410Level16(0u - v); }

// SetInputRouting: pair 1 (analog in) -> word 0 (keep 0xeeffbbfb), pair 2 (S/PDIF in) -> word 1 (keep 0xffffe0ff); mask bit 0 out 1/2,
// 1 out 3/4, 2 out 5/6, 3 out 7/8, 4 S/PDIF
static inline void FW410InputRoutingWords(uint32_t pair, uint32_t mask, uint32_t w[2])
{
    static const uint32_t b1[5] = { 0x01000000, 0x10000000, 0x400, 0x4000, 0x4 };
    static const uint32_t b2[5] = { 0x100, 0x200, 0x400, 0x800, 0x1000 };
    uint32_t &x = (pair == 1) ? w[0] : w[1];
    const uint32_t *b = (pair == 1) ? b1 : b2;
    x &= (pair == 1) ? 0xeeffbbfbu : 0xffffe0ffu;
    for (int i = 0; i < 5; i++) if (mask & (1u << i)) x |= b[i];
}

// SetSWReturnRouting: one word for the five SW-return pairs; per pair the kept mask and the five destination bits. Returns false for a pair
// outside 1..5 (reference 0xE00002C2).
static inline bool FW410SWReturnRoutingWord(uint32_t pair, uint32_t mask, uint32_t &w)
{
    static const uint32_t keep[5] = { 0x8cffffff, 0x73fcffff, 0xff83ffff, 0xff7ff0ff, 0xffff0ffe };
    static const uint32_t bits[5][5] = {
        { 0x01000000, 0x02000000, 0x10000000, 0x20000000, 0x40000000 },
        { 0x04000000, 0x08000000, 0x80000000, 0x00010000, 0x00020000 },
        { 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000 },
        { 0x00800000, 0x00000100, 0x00000200, 0x00000400, 0x00000800 },
        { 0x00001000, 0x00002000, 0x00004000, 0x00008000, 0x00000001 },
    };
    if (pair < 1 || pair > 5) return false;
    w &= keep[pair - 1];
    for (int i = 0; i < 5; i++) if (mask & (1u << i)) w |= bits[pair - 1][i];
    return true;
}

// SetHeadphoneSource: word bits 24 (1/2), 25 (3/4), 27 (5/6), 28 (7/8), 29 (S/PDIF), 26 (mask bit 5 = the AUX BUS on the phones, capture 106).
// DEVIATION (user decision 2026-10-26, R138): the reference keeps 0xc4ffffff, so bit 26 is never cleared once set and the phones stay on the
// aux bus until the driver reloads (capture 106; the official panel never sends mask bit 5, so it never showed there). The clone keeps
// 0xc0ffffff: bit 26 follows mask bit 5 like the other five sources.
static inline uint32_t FW410PhonesSourceWord(uint32_t mask, uint32_t w)
{
    static const uint32_t bits[6] = { 0x01000000, 0x02000000, 0x08000000, 0x10000000, 0x20000000, 0x04000000 };
    w &= 0xc0ffffffu;
    for (int i = 0; i < 6; i++) if (mask & (1u << i)) w |= bits[i];
    return w;
}

#endif
