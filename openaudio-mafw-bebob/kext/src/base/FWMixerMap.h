// FWMixerMap -- the generic mixer-addressing records of the AV/C layer (base), kernel-free so a model's mapping is host-testable.
// ref m_audio_b_FWAVCAudioDevice::FunctionBlockMapping {+0 kind, +4 function block, +5 channel, +8 extra} and ::Routing {+0 bus kind
// (0 input, 1 SW return), +4 source pair, +8 destination mixer, +0xc function block, +0xd plug} (16 bytes in the reference's tables).
// Kinds of FunctionBlockMapping (Q-AP0c §2): 0 aux send, 1 headphone, 2 output, 3 SW return, 4 clock, 5 aux monitor, 6 input, 7 aux out,
// 8 headphone selector, 9 (none).
#ifndef OA_FWMIXERMAP_H
#define OA_FWMIXERMAP_H

#include <stdint.h>

struct FWFunctionBlockMapping { uint32_t kind; uint8_t fb; uint8_t ch; uint32_t extra; };
struct FWRouting { uint32_t kind; uint32_t srcPair; uint32_t dstMixer; uint8_t fb; uint8_t plug; };

// ref FWAVCAudioDevice::FetchRoutingEntry (decomp FWAVCAudioDevice__FetchRoutingEntry.c): first row with the same {kind, srcPair, dstMixer};
// copies the row (incl. fb, plug) into `r`; false when none matches
static inline bool FWFetchRoutingEntry(FWRouting &r, uint32_t n, const FWRouting *table)
{
    for (uint32_t i = 0; i < n; i++)
        if (table[i].kind == r.kind && table[i].srcPair == r.srcPair && table[i].dstMixer == r.dstMixer) {
            r.kind = table[i].kind; r.srcPair = table[i].srcPair; r.dstMixer = table[i].dstMixer; r.fb = table[i].fb; r.plug = table[i].plug;
            return true;
        }
    return false;
}

#endif
