// Golden-byte host test of the Audiophile mixer addressing (plan v4 AP-3.5): MapChannelAndFunctionBlock per kind, MapRoutingParameters per row,
// and the frames the generic setters then send. Expected values from A1 Q-AP0c §1-§3 (decompiles + slice tables @0x2f750 / @0x2f7a0;
// cross-checked there with ctl-services normal.rs:609-622), written by hand.
// Build + run (from openaudio-mafw-bebob/):
//   c++ -std=c++11 -Wall -I kext/src/base -I kext/src/models/FWAudiophile tests/host/audiophile_mixer_test.cpp -o build/apmix && build/apmix
#include "FWAudiophileMixerMap.h"
#include "FWAvcFrames.h"
#include <stdio.h>

static int fails = 0;
static void ok(bool c, const char *what) { printf("%s %s\n", c ? "OK  " : "FAIL", what); if (!c) fails++; }
static bool same(const uint8_t *a, const uint8_t *b, unsigned n) { return memcmp(a, b, n) == 0; }
static void map(uint32_t kind, uint8_t ch, uint32_t wantRet, uint8_t wantFb, uint8_t wantCh, const char *what)
{
    FWFunctionBlockMapping m = { kind, 0, ch, 0 };
    uint32_t r = FWAudiophileMapChannel(m);
    char b[160]; snprintf(b, sizeof b, "kind %u ch %u -> ret %x fb %02x ch %u   (%s)", kind, ch, r, m.fb, m.ch, what);
    ok(r == wantRet && (r != 0 || (m.fb == wantFb && m.ch == wantCh)), b);
}
static void feature(uint32_t kind, uint8_t ch, uint8_t sel, int32_t v, const uint8_t want[12], const char *what)
{
    FWFunctionBlockMapping m = { kind, 0, ch, 0 }; FWAudiophileMapChannel(m);
    uint8_t f[12]; FWAvcFeatureFrame(f, m.fb, sel, m.ch, (uint16_t)(v << 7));
    ok(same(f, want, 12), what);
}

int main()
{
    // kind 0 aux send: 1-2 -> 09, 3-4 -> 0A, 5-6 -> 06, 7-8 -> 07, 9-10 -> 08, channel re-based to 1/2
    const uint8_t aux[10][2] = { {9,1},{9,2},{0xA,1},{0xA,2},{6,1},{6,2},{7,1},{7,2},{8,1},{8,2} };
    for (int c = 1; c <= 10; c++) map(0, c, 0, aux[c-1][0], aux[c-1][1], "aux send");
    map(0, 11, 0xE00002C2, 0, 0, "aux send out of table");
    map(1, 1, 0, 0x0F, 1, "headphone"); map(1, 2, 0, 0x0F, 2, "headphone");
    const uint8_t outm[6][2] = { {0xC,1},{0xC,2},{0xD,1},{0xD,2},{0xE,1},{0xE,2} };
    for (int c = 1; c <= 6; c++) map(2, c, 0, outm[c-1][0], outm[c-1][1], "output");
    map(2, 7, 0, 0, 7, "output out of range: success, unchanged (ref LAB_0000b174)");
    const uint8_t swm[6][2] = { {1,1},{1,2},{2,1},{2,2},{3,1},{3,2} };
    for (int c = 1; c <= 6; c++) map(3, c, 0, swm[c-1][0], swm[c-1][1], "SW return (kext-only blocks, unverified on hardware)");
    map(4, 1, 0xE00002C7, 0, 0, "clock: unsupported");
    map(5, 1, 0, 1, 1, "aux monitor pair 1"); map(5, 3, 0, 3, 3, "aux monitor pair 3 (S/PDIF)");
    const uint8_t inm[4][2] = { {4,1},{4,2},{5,1},{5,2} };
    for (int c = 1; c <= 4; c++) map(6, c, 0, inm[c-1][0], inm[c-1][1], "input");
    map(6, 5, 0xE00002C2, 0, 0, "input out of range");
    map(7, 2, 0, 0x0B, 2, "aux output"); map(8, 1, 0, 0x04, 1, "headphone selector"); map(9, 1, 0xE00002C2, 0, 0, "kind 9");

    // frames: aux send ch 5 at -20 (= -10 dB): fb 06 ch 1 sel 2 value (-20 << 7) = 0xF600
    const uint8_t f1[12] = { 0x00, 0x08, 0xB8, 0x81, 0x06, 0x10, 0x02, 0x01, 0x02, 0x02, 0xF6, 0x00 };
    feature(0, 5, 2, -20, f1, "frame: aux send ch 5 = -10 dB -> 00 08 B8 81 06 10 02 01 02 02 F6 00");
    // input pan ch 3 hard right (+255): generic SetInputPan sends -v: (-255 << 7) = 0x8080 on fb 05 ch 1 sel 3
    const uint8_t f2[12] = { 0x00, 0x08, 0xB8, 0x81, 0x05, 0x10, 0x02, 0x01, 0x03, 0x02, 0x80, 0x80 };
    feature(6, 3, 3, -255, f2, "frame: S/PDIF-in L pan +255 -> 00 08 B8 81 05 10 02 01 03 02 80 80");
    // output 4 at 0 dB: fb 0D ch 2
    const uint8_t f3[12] = { 0x00, 0x08, 0xB8, 0x81, 0x0D, 0x10, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00 };
    feature(2, 4, 2, 0, f3, "frame: output 4 = 0 dB -> 00 08 B8 81 0D 10 02 02 02 02 00 00");
    // aux monitor pair 2 = aux: selector fb 02 value 1
    uint8_t s[12]; FWFunctionBlockMapping m5 = { 5, 0, 2, 0 }; FWAudiophileMapChannel(m5); FWAvcSelectorFrame(s, m5.fb, 1);
    const uint8_t f4[12] = { 0x00, 0x08, 0xB8, 0x80, 0x02, 0x10, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00 };
    ok(same(s, f4, 12), "frame: out 3/4 monitors aux -> 00 08 B8 80 02 10 02 01 01 00 00 00");
    // headphone selector: source 3 (aux) -> selector fb 04 input 3 (SetHeadphoneSelector frame: header + 02 which 01 00 00 00)
    uint8_t h[12] = { 0 }; FWFunctionBlockMapping m8 = { 8, 0, 0, 0 }; FWAudiophileMapChannel(m8);
    h[6] = 0x02; h[7] = 3; h[8] = 0x01; FWAvcFunctionBlockHeader(h, 0x80, m8.fb);
    const uint8_t f5[12] = { 0x00, 0x08, 0xB8, 0x80, 0x04, 0x10, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00 };
    ok(same(h, f5, 12), "frame: phones source aux -> 00 08 B8 80 04 10 02 03 01 00 00 00");

    // routing table: every row, then src/dst forced to 1
    for (uint32_t kind = 0; kind <= 1; kind++)
        for (uint32_t pair = 1; pair <= (kind ? 3u : 2u); pair++)
            for (uint32_t mix = 1; mix <= 3; mix++) {
                FWRouting r = { kind, pair, mix, 0, 0 };
                uint32_t ret = FWAudiophileMapRouting(r);
                uint8_t wantPlug = kind ? (uint8_t)(pair - 1) : (uint8_t)(pair + 2);    // SW 1/2,3/4,5/6 -> plug 0,1,2; inputs analog,S/PDIF -> 3,4
                char b[120]; snprintf(b, sizeof b, "route %s pair %u -> mixer %u: fb %u plug %u", kind ? "SW" : "in", pair, mix, r.fb, r.plug);
                ok(ret == 0 && r.fb == mix && r.plug == wantPlug && r.srcPair == 1 && r.dstMixer == 1, b);
            }
    FWRouting bad = { 1, 4, 1, 0, 0 }; ok(FWAudiophileMapRouting(bad) == 0xE00002C2, "route SW pair 4: not in the table -> E00002C2");
    // mixer connect pair for SW 3/4 -> mixer 5/6 (fb 3, plug 1), connect: singles (1,1) then (2,2)
    uint8_t c1[16], c2[16]; FWAvcMixerConnectFrame(c1, true, 3, 1, 1, 1); FWAvcMixerConnectFrame(c2, true, 3, 1, 2, 2);
    const uint8_t w1[16] = { 0x00, 0x08, 0xB8, 0x82, 0x03, 0x10, 0x04, 0x01, 0x01, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t w2[16] = { 0x00, 0x08, 0xB8, 0x82, 0x03, 0x10, 0x04, 0x01, 0x02, 0x02, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00 };
    ok(same(c1, w1, 16) && same(c2, w2, 16), "frames: SW 3/4 -> mixer 5/6 connect = two singles (1,1), (2,2) on fb 3 plug 1");
    uint8_t d1[16]; FWAvcMixerConnectFrame(d1, false, 1, 4, 1, 1);
    ok(d1[12] == 0x80 && d1[4] == 1 && d1[7] == 4, "frame: S/PDIF in -> mixer 1/2 disconnect: value 80, fb 1, plug 4");

    // LED vendor command (AVCControlSetLEDStatus @0xb7d0: AVCVendorCommand(2, 0, state))
    uint8_t v[16]; const uint8_t st = 2; uint32_t n = FWAvcVendorFrame(v, 2, 0, 1, &st);
    const uint8_t wl[16] = { 0x00, 0xFF, 0x00, 0x00, 0x0D, 0x6C, 0x02, 0x00, 0x01, 0x02, 0, 0, 0, 0, 0, 0 };
    ok(n == 16 && same(v, wl, 16), "frame: LED state 2 -> 00 FF 00 00 0D 6C 02 00 01 02 + 6 zero bytes, length 16");
    uint8_t big[8] = { 0 }; ok(FWAvcVendorFrame(v, 2, 0, 8, big) == 0, "vendor command with 8 data bytes refused (ref: n >= 8 -> E00002C2)");

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "PASSED", fails);
    return fails != 0;
}
