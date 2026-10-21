// FWAudiophileMixerMap -- the Audiophile's mixer addressing (module models/FWAudiophile), kernel-free for the host test.
// Reference: com_m_audio_FWAudiophileDevice::MapChannelAndFunctionBlock @0xb0e2 (vt 0xb48) and ::MapRoutingParameters @0xba50 (vt 0xb40);
// tables read from the v1.11.2 slice (A1 Q-AP0c §2-§3, R79; re-read 2026-10-20): aux-send map @0x2f750 (5 x 12 bytes {min, max, fb}),
// _kRouteTable.36633 @0x2f7a0 (15 x 16 bytes). Function-block ids are per type (feature 0x04 != selector 0x04).
#ifndef OA_FWAUDIOPHILEMIXERMAP_H
#define OA_FWAUDIOPHILEMIXERMAP_H

#include "FWMixerMap.h"

struct FWAudiophileAuxRow { uint32_t min, max; uint8_t fb; };
// @0x2f750: aux sources -- analog in, S/PDIF in, stream 1/2, 3/4, 5/6 (ctl-services names, Q-AP0c §2)
static const FWAudiophileAuxRow kFWAudiophileAuxSendMap[5] = { { 1, 2, 0x09 }, { 3, 4, 0x0A }, { 5, 6, 0x06 }, { 7, 8, 0x07 }, { 9, 10, 0x08 } };

// @0x2f7a0: {kind 1 = SW return / 0 = input, source pair, destination mixer, fb, plug}; mixers 1/2/3 = fb 1/2/3 (mixer 1/2, 3/4, 5/6)
static const FWRouting kFWAudiophileRouteTable[15] = {
    { 1, 1, 1, 1, 0 }, { 1, 1, 2, 2, 0 }, { 1, 1, 3, 3, 0 },
    { 1, 2, 1, 1, 1 }, { 1, 2, 2, 2, 1 }, { 1, 2, 3, 3, 1 },
    { 1, 3, 1, 1, 2 }, { 1, 3, 2, 2, 2 }, { 1, 3, 3, 3, 2 },
    { 0, 1, 1, 1, 3 }, { 0, 1, 2, 2, 3 }, { 0, 1, 3, 3, 3 },
    { 0, 2, 1, 1, 4 }, { 0, 2, 2, 2, 4 }, { 0, 2, 3, 3, 4 },
};

// MapChannelAndFunctionBlock @0xb0e2 (decompile). Returns 0, 0xE00002C2 (bad argument) or 0xE00002C7 (unsupported); fb/ch updated as there.
static inline uint32_t FWAudiophileMapChannel(FWFunctionBlockMapping &m)
{
    const uint8_t ch = m.ch;
    switch (m.kind) {
    case 0:                                                                   // aux send: table @0x2f750
        for (int i = 0; i < 5; i++)
            if (kFWAudiophileAuxSendMap[i].min <= ch && ch <= kFWAudiophileAuxSendMap[i].max) {
                m.fb = kFWAudiophileAuxSendMap[i].fb;
                // the reference re-bases the channel only when this row's block differs from the previous row's (true for rows 1-4)
                if (i > 0 && m.fb != kFWAudiophileAuxSendMap[i - 1].fb) m.ch = (uint8_t)(ch + 1 - kFWAudiophileAuxSendMap[i].min);
                return 0;
            }
        return 0xE00002C2u;                                                   // not found: falls into case 9
    case 1: m.fb = 0x0F; return 0;                                            // headphone level
    case 2:                                                                   // output level: 1-2 -> 0x0C, 3-4 -> 0x0D, 5-6 -> 0x0E
        if ((uint8_t)(ch - 1) < 2) { m.fb = 0x0C; return 0; }
        if ((uint8_t)(ch - 3) < 2) { m.fb = 0x0D; m.ch = (uint8_t)(ch - 2); return 0; }
        if ((uint8_t)(ch - 5) < 2) { m.fb = 0x0E; m.ch = (uint8_t)(ch - 4); return 0; }
        return 0;                                                             // ref LAB_0000b174: success, fb/ch unchanged
    case 3:                                                                   // SW-return level: 1-2 -> 0x01, 3-4 -> 0x02, 5-6 -> 0x03
        if ((uint8_t)(ch - 1) < 2) { m.fb = 0x01; return 0; }
        if ((uint8_t)(ch - 3) < 2) { m.fb = 0x02; m.ch = (uint8_t)(ch - 2); return 0; }
        if ((uint8_t)(ch - 5) < 2) { m.fb = 0x03; m.ch = (uint8_t)(ch - 4); return 0; }
        return 0;
    case 4: return 0xE00002C7u;                                               // clock: not through a selector on this model
    case 5: m.fb = ch; return 0;                                              // aux monitor: selector fb = output pair
    case 6:                                                                   // input feature: 1-2 -> 0x04, 3-4 -> 0x05
        if ((uint8_t)(ch - 1) < 2) { m.fb = 0x04; return 0; }
        if ((uint8_t)(ch - 3) < 2) { m.fb = 0x05; m.ch = (uint8_t)(ch - 2); return 0; }
        return 0xE00002C2u;
    case 7: m.fb = 0x0B; return 0;                                            // aux output feature
    case 8: m.fb = 0x04; return 0;                                            // headphone selector
    case 9: return 0xE00002C2u;
    default: return 0;
    }
}

// MapRoutingParameters @0xba50: FetchRoutingEntry over the 15 rows, then source pair = destination mixer = 1 (the pair inside the block)
static inline uint32_t FWAudiophileMapRouting(FWRouting &r)
{
    if (!FWFetchRoutingEntry(r, 15, kFWAudiophileRouteTable)) return 0xE00002C2u;
    r.srcPair = 1; r.dstMixer = 1;
    return 0;
}

#endif
