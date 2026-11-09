// OpenAudio Glue app - host tests of the pure logic (make -C panel test). Same check() style as the main panel's Tests/LayoutTests.swift.
import Foundation

var failures = 0
func check(_ cond: Bool, _ msg: String) { if !cond { failures += 1; print("FAIL: \(msg)") } }

// EQ maths against dsp/GlueEQ.cpp (EQBandMagnitudeDb at 48 kHz, values printed by the C++ on 2026-10-28)
let vectors: [(Int, Double, Double, Double, Double)] = [
    (0, 100, 6, 40, 5.839022323),
    (0, 100, -5, 100, -2.500000000),
    (0, 30, 12, 1000, 0.000013046),
    (1, 400, 6, 400, 6.000000000),
    (1, 400, -12, 800, -1.906284339),
    (1, 2000, 3, 5000, 0.742480369),
    (2, 2500, -4, 2500, -4.000000000),
    (2, 12000, 9, 8000, 2.624517104),
    (3, 8000, 6, 20000, 5.996286241),
    (3, 1500, -6, 500, -0.078008780),
    (1, 5, 30, 60, 12.000000000),
    (3, 16000, 12, 23000, 11.997310405),
]
for v in vectors {
    let got = GlueEQMath.bandMagnitudeDb(band: v.0, freq: v.1, gain: v.2, rate: 48000, at: v.3)
    check(abs(got - v.4) < 1e-6, "eq: band \(v.0) \(v.1) Hz \(v.2) dB at \(v.3) Hz = \(got), C++ says \(v.4)")
}

// Settings plist: round trip, and the driver's merge rules
var s = GlueSettings()
s.bypass = true
s.stages = [4, 2, 6, 3]
s.channels[0].on = false
s.channels[1].threshold = -22
s.channels[1].source = 1
s.channels[2].bands[1] = GlueBand(freq: 350, gain: -4)
let back = GlueSettings(plist: s.plist(channels: 16))
check(back.bypass && back.stages == [4, 2, 6, 3], "plist: global settings round trip")
check(!back.channels[0].on && back.channels[1].threshold == -22 && back.channels[1].source == 1, "plist: channel settings round trip")
check(back.channels[2].bands[1] == GlueBand(freq: 350, gain: -4), "plist: EQ band round trip")
check(back.channels[20] == GlueChannel(), "plist: channels beyond the sent count keep their defaults")
let bad = GlueSettings(plist: ["stages": [9, -1, 7, 2], "channels": [["thr": 5, "src": 42]]])
check(bad.stages == [0, 0, 0, 2], "plist: out-of-range options fall back to Off")
check(bad.channels[0].threshold == 0 && bad.channels[0].source == 0, "plist: a positive threshold and an unknown source are refused")

// Dual routing: B after A, raw copies after every Glue input
check(GlueRouting.dualInput(unitIndex: 0, channel: 0, channelsA: 16) == 1, "dual: A1 is aggregate input 1")
check(GlueRouting.dualInput(unitIndex: 1, channel: 0, channelsA: 16) == 17, "dual: B1 is aggregate input 17")
check(GlueRouting.rawCopyInput(unitIndex: 1, channel: 15, channelsA: 16, channelsB: 16) == 64, "dual: B16's raw copy is input 64")

// Name templates: 16 entries each; every stereo pair starts on an odd channel and its partner is the matching R side
for (name, t) in [("band16", GlueTemplates.band16), ("dualA", GlueTemplates.dualA), ("dualB", GlueTemplates.dualB)] {
    check(t.count == 16, "templates: \(name) has 16 entries")
    for (i, e) in t.enumerated() where e.link {
        check(i % 2 == 0, "templates: \(name) \(e.name) links from an odd channel")
        check(i + 1 < t.count, "templates: \(name) \(e.name) has a partner")
    }
    for (i, e) in t.enumerated() where e.name.hasSuffix(" L") {
        check(i % 2 == 0 && t[i].link && t[i + 1].name == String(e.name.dropLast(2)) + " R", "templates: \(name) \(e.name) is a linked L/R pair")
    }
}
check(Set(GlueTemplates.dualA.prefix(8).map { $0.name } + GlueTemplates.dualB.prefix(8).map { $0.name }).isSuperset(of: ["Kick In", "Snare Top", "OH L", "OH R", "Bass DI", "Lead Vocal"]),
      "templates: the key sources are on the 16 analog inputs in Dual")

// Stereo link: the follower takes the leader's settings
var ls = GlueSettings()
ls.channels[6].threshold = -20; ls.channels[6].bands[2].gain = 4; ls.channels[6].link = true
ls.propagateLink(from: 6)
check(ls.isFollower(7) && ls.channels[7].threshold == -20 && ls.channels[7].bands[2].gain == 4 && !ls.channels[7].link, "link: the follower mirrors its leader")
var applied = GlueSettings()
let names = GlueTemplates.apply(GlueTemplates.band16, to: &applied, channels: 16, settings: true)
check(names[0] == "Kick" && applied.channels[6].link && applied.isFollower(7) && applied.channels[13].source == 2, "templates: apply sets names, sources and links")
var plain = GlueSettings()
_ = GlueTemplates.apply(GlueTemplates.dualB, to: &plain, channels: 16, settings: false)
check(plain == GlueSettings(), "templates: names only leaves the settings alone")
check(GlueSettings(plist: ls.plist(channels: 16)).channels[6].link, "plist: the link round-trips")

var dv = GlueSettings(); dv.drive = 7.5
check(GlueSettings(plist: dv.plist(channels: 4)).drive == 7.5, "plist: the Drive round-trips")
check(GlueSettings(plist: ["drive": 40]).drive == 18, "plist: the Drive is limited to +18 dB")

if failures > 0 { print("\(failures) FAILURE(S)"); exit(1) }
print("ALL PASS")
