// Golden host test of the LLRR sample routines (plan v4 AP-3.2; A1 Q-AP0d, asm 0x28eff-0x28fa6, 0x2c100-0x2c2af, decomp _SamplePeaks6.c).
// Expected values are written by hand from the machine code, not computed with the code under test.
// Build + run (from openaudio-mafw-bebob/):
//   c++ -std=c++11 -Wall -I kext/src/base -I kext/src/models/FW1814 tests/host/samples_test.cpp kext/src/base/FWSamples.cpp \
//       kext/src/models/FW1814/FW1814Peaks.cpp -o build/samplest && build/samplest
#include "FWSamples.h"
#include "FW1814Peaks.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int fails = 0;
static void eq(const char *what, const void *got, const void *want, size_t n)
{
    bool ok = memcmp(got, want, n) == 0;
    printf("%s %s\n", ok ? "OK  " : "FAIL", what);
    if (!ok) { fails++; const unsigned char *g = (const unsigned char *)got, *w = (const unsigned char *)want;
               for (size_t i = 0; i < n; i++) printf(" %02x/%02x", g[i], w[i]); printf("\n"); }
}
// wire bytes [label b2 b1 b0] as the little-endian host reads them from memory
static uint32_t wire(uint8_t label, uint8_t b2, uint8_t b1, uint8_t b0) { uint8_t m[4] = { label, b2, b1, b0 }; uint32_t x; memcpy(&x, m, 4); return x; }

int main()
{
    // input: two frames of 4 audio quadlets + 1 MIDI quadlet; samples chosen so every slot is distinct, incl. negatives
    const uint32_t in[10] = { wire(0x40, 0x00, 0x00, 0x01), wire(0x40, 0x00, 0x00, 0x02), wire(0x40, 0x00, 0x00, 0x03), wire(0x40, 0xFF, 0xFF, 0xFF), wire(0x80, 1, 2, 3),
                              wire(0x40, 0x12, 0x34, 0x56), wire(0x40, 0x80, 0x00, 0x00), wire(0x40, 0x7F, 0xFF, 0xFF), wire(0x40, 0x00, 0x01, 0x00), wire(0x81, 9, 9, 9) };
    int32_t out[8]; memset(out, 0xEE, sizeof out);
    FWConvertLLRR4SkipMIDI1(8, in, out, 0);
    // host = [w1, w3, w0, w2]; w = sign-extended 24-bit of b2 b1 b0; MIDI quadlets skipped
    const int32_t want_in[8] = { 2, -1, 1, 3,   -0x800000, 0x100, 0x123456, 0x7FFFFF };
    eq("ConvertLLRR4SkipMIDI1: host = wire[1,3,0,2], stride 5, MIDI skipped, sign-extended", out, want_in, sizeof want_in);

    // output: one frame host [1, 2, 3, 4, 5, 6] -> wire [h0, h2, h4, h1, h3, h5] packed as 40 00 00 0v, slot 6 (MIDI) untouched
    const int32_t h[12] = { 1, 2, 3, 4, 5, 6,   0x900000, -0x900000, -1, 0, 0x7FFFFF, -0x800000 };
    uint32_t w[14]; for (int i = 0; i < 14; i++) w[i] = 0xDEADBEEF;
    FWRearrangeLLRR6(h, w, 2);
    const uint32_t want_out[14] = { wire(0x40, 0, 0, 1), wire(0x40, 0, 0, 3), wire(0x40, 0, 0, 5), wire(0x40, 0, 0, 2), wire(0x40, 0, 0, 4), wire(0x40, 0, 0, 6), 0xDEADBEEF,
                                    // frame 2: clamp to [-0x800000, 0x7FFFFF]; order h0 h2 h4 h1 h3 h5
                                    wire(0x40, 0x7F, 0xFF, 0xFF), wire(0x40, 0xFF, 0xFF, 0xFF), wire(0x40, 0x7F, 0xFF, 0xFF), wire(0x40, 0x80, 0, 0), wire(0x40, 0, 0, 0), wire(0x40, 0x80, 0, 0), 0xDEADBEEF };
    eq("RearrangeLLRR6: wire = host[0,2,4,1,3,5], clamp, label 0x40, stride 7, MIDI slot untouched", w, want_out, sizeof want_out);
    uint32_t z = 0x12345678; FWRearrangeLLRR6(h, &z, 0);
    eq("RearrangeLLRR6: 0 frames writes nothing (clone guard)", &z, (const uint32_t[]){ 0x12345678 }, 4);

    // peaks: 16 frames x 6 channels; only frames 0 and 8 are sampled; value = |(int16)(s >> 8)|, running max over dst
    int32_t s[96]; memset(s, 0, sizeof s);
    for (int c = 0; c < 6; c++) { s[c] = (c + 1) << 8; s[48 + c] = (c == 2) ? (int32_t)0xFF900000 : (c << 12); s[6 + c] = 0x7FFF00; }  // frame 1 must be ignored
    s[5] = (int32_t)0xFF800000;                         // frame 0 ch 5: >>8 = 0xFFFF8000 -> (int16)-32768 -> abs trick keeps it negative -> never wins
    int32_t pk[6] = { 0, 0, 0, 0, 0, 100 };
    FWSamplePeaks6(s, pk, 16);
    // by hand: frame 0 gives c+1 (ch 5: -32768, ignored), frame 8 gives c<<4 (ch 2: |int16(0xFF9000)| = 0x7000); ch 5 keeps its 100
    const int32_t want_pk[6] = { 1, 0x10, 0x7000, 0x30, 0x40, 100 };
    eq("SamplePeaks6: every 8th frame, |int16(s>>8)| max-merge, -32768 never registers", pk, want_pk, sizeof want_pk);
    int32_t pk7[6] = { 0 }; FWSamplePeaks6(s, pk7, 7);
    eq("SamplePeaks6: fewer than 8 frames samples nothing", pk7, (const int32_t[6]){ 0 }, sizeof pk7);

    // the FW1814 S/PDIF-6 proc shares the max-merge and the identity map: same result
    int32_t pk14[6] = { 0, 0, 0, 0, 0, 100 };
    FW1814PeakProcForChannels(6)(s, pk14, 16);
    eq("FW1814 SPDIF6 peak proc unchanged by the move to the shared max-merge", pk14, want_pk, sizeof want_pk);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "PASSED", fails);
    return fails != 0;
}
