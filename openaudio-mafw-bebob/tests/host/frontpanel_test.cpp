// Host test of the front-panel law (plan v4 AP-3.7): Level Controller step law (AdjustOSNumberElement @0x21148, R87), A/B toggle, LED values,
// direct-monitoring bits (A1 Q-AP0b §3-§4). Expected values written by hand from those rules.
#include "FWFrontPanelLaw.h"
#include <stdio.h>

static int fails = 0;
static void eq(const char *what, long got, long want) { bool ok = got == want; printf("%s %-62s got %ld want %ld\n", ok ? "OK  " : "FAIL", what, got, want); if (!ok) fails++; }

int main()
{
    // ½-dB units; top region (v >= -12 or v + delta >= -12): one step; below: 12 steps (6 dB) per detent; floor -128, cap 0
    eq("0 dB, up: capped at 0",                          FWRotaryStep(0, 1), 0);
    eq("0 dB, down: -0.5 dB",                            FWRotaryStep(0, -1), -1);
    eq("-6 dB (-12), down: still the fine region -> -13", FWRotaryStep(-12, -1), -13);
    eq("-6.5 dB (-13), down: coarse -> -25",             FWRotaryStep(-13, -1), -25);
    eq("-6.5 dB (-13), up: lands at -12 -> fine -> -12",  FWRotaryStep(-13, 1), -12);
    eq("-10 dB (-20), up: coarse -> -8",                 FWRotaryStep(-20, 1), -8);
    eq("-60 dB (-120), down: floor -128 (-64 dB)",       FWRotaryStep(-120, -1), -128);
    eq("-64 dB (-128), up: coarse -> -116",              FWRotaryStep(-128, 1), -116);
    eq("-255 (minimum), up: coarse -> -243 -> floor -128", FWRotaryStep(-255, 1), -128);
    // monitor law (plan v7 AP-5.4, clone): fixed step per detent inside 0 .. -144 (-72 dB), below = off (-255), cap 0
    eq("monitor 3 dB: -86 (-43 dB), up -> -80",            FWMonitorStep(-86, 1, 6), -80);
    eq("monitor 3 dB: -86, down -> -92",                   FWMonitorStep(-86, -1, 6), -92);
    eq("monitor 3 dB: -2, up: capped at 0",                FWMonitorStep(-2, 1, 6), 0);
    eq("monitor 3 dB: 0, up: stays 0",                     FWMonitorStep(0, 1, 6), 0);
    eq("monitor 3 dB: -144 (bottom), down -> off -255",    FWMonitorStep(-144, -1, 6), -255);
    eq("monitor 3 dB: -140, down: below the span -> off",  FWMonitorStep(-140, -1, 6), -255);
    eq("monitor 3 dB: off, up -> bottom of the span -144", FWMonitorStep(-255, 1, 6), -144);
    eq("monitor 3 dB: off, down: stays off",               FWMonitorStep(-255, -1, 6), -255);
    eq("monitor 3 dB: -129 (old floor), up -> -123",       FWMonitorStep(-129, 1, 6), -123);
    eq("monitor 1.5 dB: -72 (-36 dB), up -> -69",          FWMonitorStep(-72, 1, 3), -69);
    eq("monitor 1.5 dB: -144, down -> off",                FWMonitorStep(-144, -1, 3), -255);
    eq("monitor: two detents at once, 3 dB -> 6 dB",       FWMonitorStep(-72, 2, 6), -60);
    eq("monitor: 24 detents of 3 dB span the whole 72 dB", FWMonitorStep(-144, 24, 6), 0);
    // A/B
    eq("A pressed -> B", FWABToggle(0), 1);
    eq("B pressed -> A", FWABToggle(1), 0);
    eq("other state unchanged", FWABToggle(2), 2);
    eq("LED: latent (panel chose a source) -> off (0)", FWABLedValue(1, 0), 0);
    eq("LED: A -> 1", FWABLedValue(0, 0), 1);
    eq("LED: B -> 2", FWABLedValue(0, 1), 2);
    eq("LED: other -> 3", FWABLedValue(0, 5), 3);
    uint32_t a = 0xff, b = 0xff; FWABDirectMonitoringBits(0, a, b);
    eq("DM A: routing[0] bit 0 cleared", a, 0xfe); eq("DM A: routing[1] bits 0x12 cleared", b, 0xed);
    a = 0; b = 0; FWABDirectMonitoringBits(1, a, b);
    eq("DM B: routing[0] bit 0 set", a, 1); eq("DM B: routing[1] bits 0x12 set", b, 0x12);
    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "PASSED", fails);
    return fails != 0;
}
