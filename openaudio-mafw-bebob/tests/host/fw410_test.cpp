// Host test of the FireWire 410 module (plan FW410 v2 M-9): sample routines (LLRR10WithMIDI, SamplePeaks10, the 192 kHz channel maps),
// register words (input routing, SW-return routing, phones source, level/pan encoding), mixer addressing and the status decode.
// Oracles: the reference decompiles transcribed literally below (ref_* functions, A1 decomp FW410Device__*.c) and frames written by hand from
// the asm (Q-410-0 §2, R136) -- not the code under test.
// Build + run (from openaudio-mafw-bebob/): see tests/host/run.sh
#include "FWSamples.h"
#include "FW410MixerRegs.h"
#include "FW410MixerMap.h"
#include "FW410Status.h"
#include "FW410Tables.h"
#include <stdio.h>
#include <string.h>

static int fails = 0;
static void ok(bool c, const char *what) { printf("%s %s\n", c ? "OK  " : "FAIL", what); if (!c) fails++; }

// ---- literal transcriptions of the decompiles ----------------------------------------------------------------------------------------
static void ref_SetInputRouting(uint32_t p1, uint32_t p2, uint32_t &w1134, uint32_t &w1138)   // FW410Device__SetInputRouting.c
{
    uint32_t u1, u2;
    if (p1 == 1) {
        u1 = (w1134 & 0xeeffbbfb) + 0x1000000; if ((p2 & 1) == 0) u1 = w1134 & 0xeeffbbfb;
        u2 = u1 | 0x10000000; if ((p2 & 2) == 0) u2 = u1;
        u1 = u2 | 0x400; if ((p2 & 4) == 0) u1 = u2;
        u2 = u1 | 0x4000; if ((p2 & 8) == 0) u2 = u1;
        w1134 = u2; if ((p2 & 0x10) != 0) w1134 = u2 | 4;
    } else {
        u1 = (w1138 & 0xffffe0ff) + 0x100; if ((p2 & 1) == 0) u1 = w1138 & 0xffffe0ff;
        u2 = u1 | 0x200; if ((p2 & 2) == 0) u2 = u1;
        u1 = u2 | 0x400; if ((p2 & 4) == 0) u1 = u2;
        u2 = u1 | 0x800; if ((p2 & 8) == 0) u2 = u1;
        w1138 = u2; if ((p2 & 0x10) != 0) w1138 = u2 | 0x1000;
    }
}
static void ref_SetSWReturnRouting(uint32_t p1, uint32_t p2, uint32_t &w)                     // FW410Device__SetSWReturnRouting.c
{
    uint32_t u1, u2;
    switch (p1) {
    case 1: u1 = (w & 0x8cffffff) + 0x1000000; if (!(p2 & 1)) u1 = w & 0x8cffffff; u2 = u1 | 0x2000000; if (!(p2 & 2)) u2 = u1;
            u1 = u2 | 0x10000000; if (!(p2 & 4)) u1 = u2; u2 = u1 | 0x20000000; if (!(p2 & 8)) u2 = u1; w = u2; if (!(p2 & 0x10)) return; u2 |= 0x40000000; break;
    case 2: u1 = (w & 0x73fcffff) + 0x4000000; if (!(p2 & 1)) u1 = w & 0x73fcffff; u2 = u1 | 0x8000000; if (!(p2 & 2)) u2 = u1;
            u1 = u2 | 0x80000000; if (!(p2 & 4)) u1 = u2; u2 = u1 | 0x10000; if (!(p2 & 8)) u2 = u1; w = u2; if (!(p2 & 0x10)) return; u2 |= 0x20000; break;
    case 3: u1 = (w & 0xff83ffff) + 0x40000; if (!(p2 & 1)) u1 = w & 0xff83ffff; u2 = u1 | 0x80000; if (!(p2 & 2)) u2 = u1;
            u1 = u2 | 0x100000; if (!(p2 & 4)) u1 = u2; u2 = u1 | 0x200000; if (!(p2 & 8)) u2 = u1; w = u2; if (!(p2 & 0x10)) return; u2 |= 0x400000; break;
    case 4: u1 = (w & 0xff7ff0ff) + 0x800000; if (!(p2 & 1)) u1 = w & 0xff7ff0ff; u2 = u1 | 0x100; if (!(p2 & 2)) u2 = u1;
            u1 = u2 | 0x200; if (!(p2 & 4)) u1 = u2; u2 = u1 | 0x400; if (!(p2 & 8)) u2 = u1; w = u2; if (!(p2 & 0x10)) return; u2 |= 0x800; break;
    case 5: u1 = (w & 0xffff0ffe) + 0x1000; if (!(p2 & 1)) u1 = w & 0xffff0ffe; u2 = u1 | 0x2000; if (!(p2 & 2)) u2 = u1;
            u1 = u2 | 0x4000; if (!(p2 & 4)) u1 = u2; u2 = u1 | 0x8000; if (!(p2 & 8)) u2 = u1; w = u2; if (!(p2 & 0x10)) return; u2 |= 1; break;
    default: return;
    }
    w = u2;
}
static uint32_t ref_SetHeadphoneSource(uint32_t p1, uint32_t w)                               // FW410Device__SetHeadphoneSource.c
{
    uint32_t u1 = (w & 0xc4ffffff) + 0x1000000, u2;
    if ((p1 & 1) == 0) u1 = w & 0xc4ffffff;
    u2 = u1 | 0x2000000;  if ((p1 & 2) == 0) u2 = u1;
    u1 = u2 | 0x8000000;  if ((p1 & 4) == 0) u1 = u2;
    u2 = u1 | 0x10000000; if ((p1 & 8) == 0) u2 = u1;
    u1 = u2 | 0x20000000; if ((p1 & 0x10) == 0) u1 = u2;
    u2 = u1 | 0x4000000;  if ((p1 & 0x20) == 0) u2 = u1;
    return u2;
}
static uint16_t ref_Level(uint32_t p2) { return (uint16_t)(((uint16_t)(p2 >> 1) & 0xff) + (int16_t)(p2 << 0xf)); }       // SetInputLevel
static uint16_t ref_Pan(uint32_t p2)   { return (uint16_t)(((uint16_t)((0u - p2) >> 1) & 0xff) + (int16_t)p2 * -0x8000); } // SetInputPan

static uint32_t W(SInt32 v) { return FWPackAM824(v); }

int main()
{
    // ---- register words: every mask 0..63, from two different starting words -----------------------------------------------------
    bool allIn = true, allSW = true, allHp = true;
    const uint32_t starts[2] = { 0, 0xffffffff };
    for (int s = 0; s < 2; s++) for (uint32_t pair = 1; pair <= 5; pair++) for (uint32_t m = 0; m < 32; m++) {
        uint32_t a = starts[s], b = starts[s];
        ref_SetSWReturnRouting(pair, m, a); FW410SWReturnRoutingWord(pair, m, b); if (a != b) allSW = false;
        if (pair <= 2) { uint32_t r[2] = { starts[s], starts[s] }, x = starts[s], y = starts[s]; FW410InputRoutingWords(pair, m, r); ref_SetInputRouting(pair, m, x, y);
                         if (r[0] != x || r[1] != y) allIn = false; }
    }
    // the reference word, except bit 26 = mask bit 5 (deviation R138: the reference never clears it)
    for (int s = 0; s < 2; s++) for (uint32_t m = 0; m < 64; m++)
        if (FW410PhonesSourceWord(m, starts[s]) != ((ref_SetHeadphoneSource(m, starts[s]) & ~0x04000000u) | ((m & 0x20) ? 0x04000000u : 0))) allHp = false;
    ok(allIn, "SetInputRouting words equal the decompile (pairs 1-2, masks 0-31, two start words)");
    ok(allSW, "SetSWReturnRouting word equals the decompile (pairs 1-5, masks 0-31, two start words)");
    ok(allHp, "SetHeadphoneSource word equals the decompile except bit 26 = mask bit 5 (masks 0-63, two start words; R138)");
    uint32_t w = 0; ok(!FW410SWReturnRoutingWord(6, 1, w) && w == 0, "SW-return routing pair 6 rejected, word unchanged");
    ok(FW410PhonesSourceWord(0, 0x04000000) == 0 && ref_SetHeadphoneSource(0, 0x04000000) == 0x04000000, "phones aux bit 26 cleared by a mask without bit 5 (the reference keeps it: deviation R138)");
    bool lv = true; for (uint32_t v = 0; v < 0x200; v++) if (FW410Level16(0u - v) != ref_Level(0u - v) || FW410Pan16(v) != ref_Pan(v) || FW410Pan16(0u - v) != ref_Pan(0u - v)) lv = false;
    ok(lv, "level / pan 16-bit encodings equal the decompile (0..-511, 0..511)");
    ok(FW410Level16(0u - 2) == 0x00ff, "level -2 (x 0x80 = -256 = 0xFF00 big-endian) -> little-endian 0x00FF");

    // ---- mixer map ----
    struct { uint32_t kind; uint8_t ch; uint32_t extra, ret; uint8_t fb, och; } mt[] = {
        {0, 1, 0, 0, 7, 1}, {0, 2, 0, 0, 7, 2}, {0, 3, 0, 0, 8, 1}, {0, 4, 0, 0, 8, 2}, {0, 5, 0, 0, 5, 1}, {0, 12, 0, 0, 5, 8}, {0, 13, 0, 0, 6, 1},
        {0, 14, 0, 0, 6, 2}, {0, 15, 0, 0xE00002C2, 0, 0}, {1, 1, 0, 0, 0xf, 1}, {2, 1, 0, 0, 0xa, 1}, {2, 4, 0, 0, 0xb, 2}, {2, 9, 0, 0, 0xe, 1},
        {2, 10, 0, 0, 0xe, 2}, {2, 11, 0, 0xE00002C2, 0, 0}, {3, 8, 0, 0, 1, 8}, {3, 9, 0, 0, 2, 1}, {3, 11, 0, 0xE00002C2, 0, 0}, {4, 7, 0, 0, 1, 0},
        {4, 7, 1, 0, 1, 2}, {4, 7, 4, 0, 1, 1}, {4, 7, 5, 0, 1, 3}, {4, 7, 2, 0, 1, 7}, {5, 2, 0, 0, 3, 2}, {6, 2, 0, 0, 3, 2}, {6, 3, 0, 0, 4, 1},
        {6, 5, 0, 0xE00002C2, 0, 0}, {7, 1, 0, 0, 9, 1}, {8, 1, 0, 0xE00002C2, 0, 0}, {10, 1, 0, 0, 0, 1} };
    bool mapok = true;
    for (unsigned i = 0; i < sizeof mt / sizeof mt[0]; i++) {
        FWFunctionBlockMapping m = { mt[i].kind, 0, mt[i].ch, mt[i].extra };
        uint32_t r = FW410MapChannel(m);
        if (r != mt[i].ret || (r == 0 && (m.fb != mt[i].fb || m.ch != mt[i].och))) { mapok = false; printf("  map kind %u ch %u extra %u -> %x fb %x ch %u\n", mt[i].kind, mt[i].ch, mt[i].extra, r, m.fb, m.ch); }
    }
    ok(mapok, "MapChannelAndFunctionBlock per kind equals the decompile (30 cases)");
    FWRouting r1 = { 1, 5, 2, 0, 0 }; FW410MapRouting(r1); FWRouting r2 = { 1, 3, 2, 0, 0 }; FW410MapRouting(r2);
    FWRouting r3 = { 0, 1, 1, 0, 0 }; FW410MapRouting(r3); FWRouting r4 = { 0, 2, 1, 0, 0 }; FW410MapRouting(r4);
    ok(r1.plug == 1 && r1.srcPair == 1 && r2.plug == 0 && r2.srcPair == 3 && r3.plug == 1 && r3.srcPair == 1 && r4.plug == 2 && r4.srcPair == 1,
       "MapRoutingParameters fix-ups (SW rtn pair 5 -> plug 1; input pair 1 -> plug 1, pair 2 -> plug 2, pair 1)");

    // ---- status ----
    FW410Status st = FW410DecodeStatus(0x01030100u);
    ok(st.locked && st.fdf == 3 && st.encoder == 1, "status 0x01030100: locked, FDF 3, encoder +1");
    st = FW410DecodeStatus(0x00000401u);                                       // capture 103/105 idle word at 96 kHz, internal
    ok(!st.locked && st.fdf == 0 && st.encoder == 4, "status 0x00000401 (rig idle): unlocked, FDF 0, encoder byte 4 (no step)");

    // ---- samples ----
    SInt32 host[2 * 10]; for (int i = 0; i < 20; i++) host[i] = (i + 1) * 0x10101;
    UInt32 wire[2 * 11]; memset(wire, 0xee, sizeof wire);
    FWRearrangeLLRR10WithMIDI(host, wire, 2);
    const int p[10] = { 8, 0, 2, 4, 6, 9, 1, 3, 5, 7 };
    bool r10 = true; for (int f = 0; f < 2; f++) { for (int k = 0; k < 10; k++) if (wire[f * 11 + k] != W(host[f * 10 + p[k]])) r10 = false; if (wire[f * 11 + 10] != 0xeeeeeeee) r10 = false; }
    ok(r10, "LLRR10WithMIDI: wire = S/PDIF L, L1-L4, S/PDIF R, R1-R4; MIDI quadlet untouched; stride 10/11");
    UInt32 none = 0x12345678; FWRearrangeLLRR10WithMIDI(host, &none, 0); ok(none == 0x12345678, "LLRR10WithMIDI: 0 frames writes nothing");

    SInt32 h8[2 * 8]; for (int i = 0; i < 16; i++) h8[i] = (i + 1) * 0x20202;
    UInt32 w9[2 * 9]; memset(w9, 0xee, sizeof w9);
    FWRemapSampleFramesForOutput(h8, w9, 8, 8, 2, kFW410OutputMap192, 0, true);
    bool rm = true; for (int f = 0; f < 2; f++) { for (int h = 0; h < 8; h++) if (w9[f * 9 + kFW410OutputMap192[h]] != W(h8[f * 8 + h])) rm = false; if (w9[f * 9 + 8] != 0xeeeeeeee) rm = false; }
    ok(rm, "192 kHz output map: host h -> wire {0,4,1,5,2,6,3,7}[h] (L1-L4, R1-R4), stride 8/9, MIDI untouched");
    const UInt32 copy[3] = { 0, 8, 0xFFFFFFFF }; UInt32 wc[9]; memset(wc, 0, sizeof wc);
    FWRemapSampleFramesForOutput(h8, wc, 8, 8, 1, kFW410OutputMap192, copy, true);
    ok(wc[8] == wc[0] && wc[0] == W(h8[0]), "output copy list (0 -> 8) copies inside the frame");
    UInt32 in3[2 * 3] = { W(0x111111), W(0x222222), 0x80000000, W(0x333333), W(0x444444), 0x80000000 }; SInt32 hin[4] = { 0, 0, 0, 0 };
    FWConvertSampleFramesFromInput(in3, hin, 2, 2, 2, kFW410InputMap192, true);
    ok(hin[0] == 0x111111 && hin[1] == 0x222222 && hin[2] == 0x333333 && hin[3] == 0x444444, "192 kHz input map {0,1}: 2 channels + MIDI per frame");

    // the reference reads only the first frame of every 8-frame group (decomp: puVar5 += 0x50 per iteration)
    SInt32 pk[10]; memset(pk, 0, sizeof pk); SInt32 src[16 * 10]; memset(src, 0, sizeof src); src[3] = 0x123400; src[80 + 9] = -0x7f0000; src[10 + 5] = 0x7f0000;
    FWSamplePeaks10(src, pk, 16);
    ok(pk[3] == 0x1234 && pk[9] == 0x7f00 && pk[5] == 0 && pk[0] == 0, "SamplePeaks10: |sample >> 8| max of frame 0 of each 8-frame group (frame 1 ignored)");

    // ---- tables ----
    FWDeviceChannels lo = kFW410Channels, hi = kFW410Channels192;
    ok(lo.in == 4 && lo.out == 10 && !lo.channelMap[0] && hi.in == 2 && hi.out == 8 && hi.channelMap[0] == kFW410InputMap192 && hi.channelMap[1] == kFW410OutputMap192 && !hi.channelMap[2],
       "channel rows: 4/10 without maps, 2/8 with the input + output maps (capture 103)");
    return fails ? 1 : 0;
}
