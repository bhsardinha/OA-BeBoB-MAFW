// FWFrontPanelLaw -- the arithmetic of the front-panel settings (Level Controller step law, A/B switch state, LEDs, direct-monitoring bits),
// shared by every model with these controls (base) and kernel-free for the host test. The settings objects (FWSettings.cpp) call these.
// Reference: AdjustOSNumberElement @0x21148 (asm 0x21148-0x211fe, R87), FWSettingsABSwitch::MomentarySwitchPressed / UpdateLEDs @0xe96a /
// SendMixerSettingsForCurrentMomentaryMode (A1 Q-AP0b §3-§4, Q-ME §3).
#ifndef OA_FWFRONTPANELLAW_H
#define OA_FWFRONTPANELLAW_H

#include <stdint.h>

// One knob event on one element (½-dB units): near the top (v >= -12, or v + delta >= -12) one ½-dB step in the turn direction; further down
// 12 steps (6 dB) per detent. Result floored at -128 (-64 dB) and capped at 0 (the reference's lower clamp -0x100 can never trigger).
static inline int32_t FWRotaryStep(int32_t v, int32_t delta)
{
    int32_t step;
    if (v < -0xc && v + delta < -0xc) step = delta * 0xc;
    else step = (delta > 0) ? 1 : -1;
    int32_t r = -0x80;
    if (v + step > -0x81) r = v + step;
    if (r < -0x100) r = -0x100; else if (r > 0) r = 0;
    return r;
}

// Monitor law (clone, user decision 2026-10-20, plan v7 AP-5.4; NOT the reference): output / phones / aux-out levels move by a fixed `step`
// per detent (½-dB units: 6 = 3 dB, 3 = 1.5 dB) inside a 72 dB span (0 .. -144); one detent below the span, or any value under it, is
// off (-255, the level domain's minimum); from off one detent up lands on the bottom of the span. Capped at 0.
static const int32_t kFWMonitorFloor = -144;                                    // -72 dB
static const int32_t kFWLevelOff = -255;
static inline int32_t FWMonitorStep(int32_t v, int32_t delta, int32_t step)
{
    if (delta == 0) return v;
    if (v < kFWMonitorFloor) {
        if (delta < 0) return kFWLevelOff;
        const int32_t r = kFWMonitorFloor + (delta - 1) * step;
        return r > 0 ? 0 : r;
    }
    const int32_t r = v + delta * step;
    if (r < kFWMonitorFloor) return kFWLevelOff;
    return r > 0 ? 0 : r;
}

// MomentarySwitchPressed: A (0) <-> B (1); any other state stays
static inline uint32_t FWABToggle(uint32_t mode) { return mode == 1 ? 0 : mode == 0 ? 1 : mode; }

// UpdateLEDs: latent (headphone source set by the panel) -> 0 (off); A -> 1; B -> 2; other -> 3
static inline uint32_t FWABLedValue(uint32_t latent, uint32_t mode) { return latent ? 0 : mode == 1 ? 2 : mode == 0 ? 1 : 3; }

// Direct-monitoring mode: state A clears bit 0 of MAInputRoutingV2[0] and bits 0x12 of [1]; state B sets them
static inline void FWABDirectMonitoringBits(uint32_t mode, uint32_t &a, uint32_t &b)
{
    if (mode == 0) { a &= 0xfffffffe; b &= 0xffffffed; }
    else           { a |= 1;          b |= 0x12; }
}

#endif
