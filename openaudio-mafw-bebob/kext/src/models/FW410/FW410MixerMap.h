// FW410MixerMap -- the 410's mixer addressing (module models/FW410), kernel-free for the host test.
// Reference: com_m_audio_FW410Device::MapChannelAndFunctionBlock @0x14600 (vt 0xb48, decompile) with the aux-send table @0x2fa50 (4 x 12 bytes
// {min, max, fb}), and ::MapRoutingParameters @0x1428a (vt 0xb40: the base, then the 410's fix-ups). On the 410 only the groups WITHOUT a
// register batch writer use these AV/C paths: aux sends (kind 0), headphone level (1), aux monitor (5), aux out (7); SW return / output / input
// levels go to registers, pan and routing to the 410's register writers (FW410MixerRegs.h). Q-410-0 §5.
#ifndef OA_FW410MIXERMAP_H
#define OA_FW410MIXERMAP_H

#include "FWMixerMap.h"

struct FW410AuxRow { uint32_t min, max; uint8_t fb; };
// @0x2fa50: aux sources in MAAuxSendLevels order -- analog in 1/2, S/PDIF in L/R, SW returns 1-8, S/PDIF SW return (capture 103)
static const FW410AuxRow kFW410AuxSendMap[4] = { { 1, 2, 0x07 }, { 3, 4, 0x08 }, { 5, 12, 0x05 }, { 13, 14, 0x06 } };

// Returns 0, 0xE00002C2 (bad argument); fb/ch updated as in the reference.
static inline uint32_t FW410MapChannel(FWFunctionBlockMapping &m)
{
    const uint8_t ch = m.ch;
    switch (m.kind) {
    case 0:                                                                   // aux send: table @0x2fa50
        for (int i = 0; i < 4; i++)
            if (kFW410AuxSendMap[i].min <= ch && ch <= kFW410AuxSendMap[i].max) {
                m.fb = kFW410AuxSendMap[i].fb;
                if (i > 0 && m.fb != kFW410AuxSendMap[i - 1].fb) m.ch = (uint8_t)(ch + 1 - kFW410AuxSendMap[i].min);
                return 0;
            }
        return 0xE00002C2u;                                                   // not found: falls into case 8/9
    case 1: m.fb = 0x0F; return 0;                                            // headphone level
    case 2:                                                                   // output level: pairs 1-5 -> fb 0x0A..0x0E
        for (uint8_t p = 0; p < 5; p++)
            if ((uint8_t)(ch - 1 - 2 * p) < 2) { m.fb = (uint8_t)(0x0A + p); m.ch = (uint8_t)(ch - 2 * p); return 0; }
        return 0xE00002C2u;
    case 3:                                                                   // SW-return level: 1-8 -> fb 1, 9-10 -> fb 2 (ch - 8)
        if ((uint8_t)(ch - 1) < 8) { m.fb = 0x01; return 0; }
        if ((uint8_t)(ch - 9) < 2) { m.fb = 0x02; m.ch = (uint8_t)(ch - 8); return 0; }
        return 0xE00002C2u;
    case 4:                                                                   // clock: selector fb 1, channel by extra & 7
        m.fb = 0x01;
        switch (m.extra & 7) { case 0: m.ch = 0; break; case 1: m.ch = 2; break; case 4: m.ch = 1; break; case 5: m.ch = 3; break; default: break; }
        return 0;
    case 5: m.fb = (uint8_t)(ch + 1); return 0;                               // aux monitor: selector fb = output pair + 1
    case 6:                                                                   // input feature: 1-2 -> 3, 3-4 -> 4 (ch - 2)
        if ((uint8_t)(ch - 1) < 2) { m.fb = 0x03; return 0; }
        if ((uint8_t)(ch - 3) < 2) { m.fb = 0x04; m.ch = (uint8_t)(ch - 2); return 0; }
        return 0xE00002C2u;
    case 7: m.fb = 0x09; return 0;                                            // aux output feature
    case 8: case 9: return 0xE00002C2u;
    default: return 0;
    }
}

// MapRoutingParameters @0x1428a: the base (fb 1, plug 0), then bus kind 1 (SW return) pair 5 -> plug 1, pair 1; bus kind 0 (input) -> plug
// 1 for pair 1, 2 for pair 2, pair 1. Unused while the 410's register routing writers are installed (kept for the AV/C routing path).
static inline uint32_t FW410MapRouting(FWRouting &r)
{
    r.fb = 1; r.plug = 0;
    if (r.kind == 1) { if (r.srcPair != 5) return 0; r.plug = 1; r.srcPair = 1; }
    else if (r.kind == 0) { r.plug = (uint8_t)((r.srcPair != 1) + 1); if (r.srcPair != 1) r.srcPair = 1; }
    return 0;
}

#endif
