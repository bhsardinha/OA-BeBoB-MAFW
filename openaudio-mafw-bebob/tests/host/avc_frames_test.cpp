// Golden-byte host test of the Rev2 AV/C frames (plan v3 AP-2.3). Expected bytes from A1 Q-AP0 §2.1-§2.2 (asm 0x12e7e-0x1305a, 0x12c60-0x12e1d,
// decomp FWRev2AudioDevice__GetAddressAndPlugForSyncType.c) and the FFADO field order (avc_extended_stream_format.cpp:340-351).
// Build + run:  c++ -std=c++11 -I kext/src/base tests/host/avc_frames_test.cpp -o build/avcft && build/avcft   (from openaudio-mafw-bebob/; build/ is git-ignored)
#include "FWAvcFrames.h"
#include <stdio.h>

static int fails = 0;
static void eq(const char *what, const uint8_t *got, const uint8_t *want, unsigned n)
{
    bool ok = memcmp(got, want, n) == 0;
    printf("%s %s\n", ok ? "OK  " : "FAIL", what);
    if (!ok) { fails++; for (unsigned i = 0; i < n; i++) printf(" %02x/%02x", got[i], want[i]); printf("\n"); }
}
static void eqv(const char *what, uint32_t got, uint32_t want)
{
    printf("%s %s (%#x)\n", got == want ? "OK  " : "FAIL", what, got);
    if (got != want) fails++;
}

int main()
{
    // rate codes: FDF 0..6 -> 02 03 04 0A 05 06 07, unknown -> 03
    const uint8_t rc[8] = { 0x02, 0x03, 0x04, 0x0A, 0x05, 0x06, 0x07, 0x03 };
    uint8_t got[8]; for (int i = 0; i < 8; i++) got[i] = FWAvcRev2RateCode(i);
    eq("rate codes FDF0..7", got, rc, 8);

    // C1 list walk: output plug 0 (direction 0 -> dir' 1), entry 2
    uint8_t l[14]; FWAvcRev2FormatListFrame(l, 0, 0, 2);
    const uint8_t lw[14] = { 0x01, 0xFF, 0x2F, 0xC1, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0, 0, 0 };
    eq("C1 STATUS output plug 0 idx 2 (14 bytes, tail zero)", l, lw, 14);
    FWAvcRev2FormatListFrame(l, 1, 0, 0);
    const uint8_t li[14] = { 0x01, 0xFF, 0x2F, 0xC1, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0, 0, 0 };
    eq("C1 STATUS input plug 0 idx 0", l, li, 14);

    // C0 set from a reply with 2 {channels, format} fields at 48 kHz: reply[15] = 2, reply[16..19] = 04 06 01 0D
    uint8_t reply[0x200] = {0};
    reply[13] = 0x04; reply[15] = 2; reply[16] = 0x04; reply[17] = 0x06; reply[18] = 0x01; reply[19] = 0x0D;
    uint8_t c[0x200];
    uint32_t n = FWAvcRev2FormatSetFrame(c, sizeof(c), 0, 0, FWAvcRev2RateCode(2), reply);
    eqv("C0 length = 2*n + 0x12", n, 2 * 2 + 0x12);
    const uint8_t cw[22] = { 0x00, 0xFF, 0x2F, 0xC0, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x90, 0x40, 0x04, 0x01,
                             0x02, 0x04, 0x06, 0x01, 0x0D, 0x00, 0x00, 0x00 };
    eq("C0 CONTROL output plug 0, 48 kHz, 2 fields, 3 zero bytes at the end", c, cw, 22);
    eqv("C0 refuses a too-small buffer", FWAvcRev2FormatSetFrame(c, 10, 0, 0, 4, reply), 0);

    // SIGNAL SOURCE
    uint8_t s[8]; FWAvcSignalSourceStatusFrame(s, 1);
    const uint8_t sw[8] = { 0x01, 0xFF, 0x1A, 0x0F, 0xFF, 0xFE, 0x60, 0x01 };
    eq("0x1A STATUS music subunit plug 1", s, sw, 8);
    uint8_t a, p;
    const struct { uint32_t type; uint8_t plug, addr, src; } t[] = { {0, 1, 0x60, 0x01}, {0, 2, 0x60, 0x03}, {1, 1, 0xFF, 0x81}, {2, 1, 0xFF, 0x00},
                                                                     {3, 1, 0xFF, 0x83}, {4, 1, 0xFF, 0x82} };
    for (unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
        a = p = 0xEE;
        uint32_t r = FWAvcRev2SyncSource(t[i].type, t[i].plug, &a, &p);
        char what[64]; snprintf(what, sizeof what, "sync type %u plug %u -> %02x %02x", t[i].type, t[i].plug, t[i].addr, t[i].src);
        const uint8_t g[3] = { (uint8_t)r, a, p }, w[3] = { 0, t[i].addr, t[i].src };
        eq(what, g, w, 3);
    }
    eqv("sync type 5 unsupported", FWAvcRev2SyncSource(5, 1, &a, &p), 0xE00002C7u);
    FWAvcRev2SyncSource(4, 1, &a, &p); FWAvcSignalSourceControlFrame(s, a, p, 1);
    const uint8_t cs[8] = { 0x00, 0xFF, 0x1A, 0x0F, 0xFF, 0x82, 0x60, 0x01 };
    eq("0x1A CONTROL S/PDIF (type 4) -> FF 82 (Audiophile, ctl-services normal.rs:483-495)", s, cs, 8);
    FWAvcRev2SyncSource(0, 1, &a, &p); FWAvcSignalSourceControlFrame(s, a, p, 1);
    const uint8_t ci[8] = { 0x00, 0xFF, 0x1A, 0x0F, 0x60, 0x01, 0x60, 0x01 };
    eq("0x1A CONTROL internal (type 0) -> 60 01", s, ci, 8);

    // peaks: low 16 bits of the host quadlet, byte-swapped
    eqv("peak 0x12345678 -> 0x7856", FWRev2PeakFromQuadlet(0x12345678u), 0x7856);
    eqv("peak 0x0000ffff -> 0xffff", FWRev2PeakFromQuadlet(0x0000ffffu), 0xffff);

    // EXTENDED PLUG INFO (clone diagnostic, AP-3.10; FFADO layout)
    uint8_t pi[13];
    FWAvcExtPlugInfoChannelPositionFrame(pi, 1, 0);
    const uint8_t piw[11] = { 0x01, 0xFF, 0x02, 0xC0, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x03, 0x00 };
    eq("plug info: channel position, output plug 0 (11 bytes)", pi, piw, 11);
    FWAvcExtPlugInfoClusterFrame(pi, 0, 0, 2);
    const uint8_t pcw[13] = { 0x01, 0xFF, 0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x07, 0x02, 0xFF, 0xFF };
    eq("plug info: cluster 2 of input plug 0 (13 bytes)", pi, pcw, 13);
    // synthetic response: 2 clusters, [1] 2 channels (pos 1 loc 1, pos 3 loc 2), [2] 2 channels (pos 2 loc 1, pos 4 loc 2)
    const uint8_t pr[21] = { 0x0C, 0xFF, 0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x03, 0x02, 0x02, 0x01, 0x01, 0x03, 0x02, 0x02, 0x02, 0x01, 0x04, 0x02 };
    char sum[96]; uint32_t nc = FWAvcFormatChannelPosition(pr, sizeof pr, sum, sizeof sum);
    eqv("plug info: response has 2 clusters", nc, 2);
    const char *want = "2 clusters: [1] 1/1 3/2 [2] 2/1 4/2";
    eqv("plug info: summary text", strcmp(sum, want) == 0, 1);
    printf("     summary: %s\n", sum);
    eqv("plug info: short response -> 0 clusters", FWAvcFormatChannelPosition(pr, 10, sum, sizeof sum), 0);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "PASSED", fails);
    return fails != 0;
}
