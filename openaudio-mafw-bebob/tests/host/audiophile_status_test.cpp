// Host test of the Audiophile status-word decode (plan v4 AP-3.4). Lock/FDF cases are the words recorded on the SL rig (capture 93,
// docs/system-map/B5-sl-dtrace-truth/captures/93-sl-audiophile-status-window-reads/status-reads.txt); encoder / button cases are built from the
// bit positions of ReadHSCIData @0xba92 (A1 Q-AP0b §2) because capture 93 has no knob or button events.
// Build + run (from openaudio-mafw-bebob/):
//   c++ -std=c++11 -Wall -I kext/src/models/FWAudiophile tests/host/audiophile_status_test.cpp -o build/apstat && build/apstat
#include "FWAudiophileStatus.h"
#include <stdio.h>

static int fails = 0;
static uint32_t word(uint8_t w0, uint8_t w1, uint8_t w2, uint8_t w3) { return (uint32_t)w3 << 24 | (uint32_t)w2 << 16 | (uint32_t)w1 << 8 | w0; }
static void expect(const char *what, uint32_t s, bool locked, int fdf, int lvl, int hp, bool ab)
{
    FWAudiophileStatus st = FWAudiophileDecodeStatus(s);
    bool ok = st.locked == locked && st.fdf == fdf && st.levelDelta == lvl && st.headphoneDelta == hp && st.abPressed == ab;
    printf("%s %-58s s=%08x locked=%d fdf=%02x lvl=%d hp=%d ab=%d\n", ok ? "OK  " : "FAIL", what, s, st.locked, st.fdf, st.levelDelta, st.headphoneDelta, st.abPressed);
    if (!ok) fails++;
}

int main()
{
    // capture 93: wire bytes w0 w1 w2 w3
    expect("cap93 44.1 kHz internal   00 00 01 01 -> locked, FDF 1", word(0x00, 0x00, 0x01, 0x01), true, 1, 0, 0, false);
    expect("cap93 48 kHz internal     00 00 02 01 -> locked, FDF 2", word(0x00, 0x00, 0x02, 0x01), true, 2, 0, 0, false);
    expect("cap93 88.2 kHz internal   00 00 03 01 -> locked, FDF 3", word(0x00, 0x00, 0x03, 0x01), true, 3, 0, 0, false);
    expect("cap93 96 kHz internal     00 00 04 01 -> locked, FDF 4", word(0x00, 0x00, 0x04, 0x01), true, 4, 0, 0, false);
    expect("cap93 external, no source 00 00 ff 00 -> unlocked, FF",  word(0x00, 0x00, 0xff, 0x00), false, 0xff, 0, 0, false);
    // encoders: w1 low nibble = Level Controller, high nibble = headphone; 1 = +1, 2 = -1, other values = no change
    expect("Level Controller up (w1 = 01)",   word(0x00, 0x01, 0x02, 0x01), true, 2, 1, 0, false);
    expect("Level Controller down (w1 = 02)", word(0x00, 0x02, 0x02, 0x01), true, 2, -1, 0, false);
    expect("headphone up (w1 = 10)",          word(0x00, 0x10, 0x02, 0x01), true, 2, 0, 1, false);
    expect("headphone down + level up (w1 = 21)", word(0x00, 0x21, 0x02, 0x01), true, 2, 1, -1, false);
    expect("nibble 3 = no change (w1 = 33)",  word(0x00, 0x33, 0x02, 0x01), true, 2, 0, 0, false);
    // A/B button: any of w0 bits 4-7; bits 0-3 alone do not count
    expect("A/B pressed (w0 = 10)",           word(0x10, 0x00, 0x02, 0x01), true, 2, 0, 0, true);
    expect("A/B pressed (w0 = f0)",           word(0xf0, 0x00, 0x02, 0x01), true, 2, 0, 0, true);
    expect("w0 = 0f is not a press",          word(0x0f, 0x00, 0x02, 0x01), true, 2, 0, 0, false);
    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "PASSED", fails);
    return fails != 0;
}
