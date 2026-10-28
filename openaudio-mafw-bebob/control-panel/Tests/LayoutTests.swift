// Host unit test for ChannelLayout, MaKeys, DeviceModel and the peak math (run on the dev Mac: see Makefile target `test`). Not part of the app.
import Foundation

var failures = 0
func check(_ c: Bool, _ m: String) { if !c { failures += 1; print("FAIL: \(m)") } }

func clock(adatIn: Bool, adatOut: Bool, smux: Bool = false) -> MAClockSource {
    var c = MAClockSource.factoryDefault
    c.adatIn = adatIn; c.adatOut = adatOut; c.smux = smux
    return c
}

// 48 kHz, ADAT/ADAT: 8 input strips, nothing greyed, S/MUX user-set and enabled.
var l = ChannelLayout(rate: 48000, clock: clock(adatIn: true, adatOut: true))
check(l.inputStrips.count == 8 && l.inputStrips.filter { !$0.enabled }.isEmpty, "48k adat strips")
check(l.smuxEnabled && !l.smuxShownChecked, "48k smux user-set")
check(l.returnBus == .adat && !l.adatBusHalfGreyed, "48k adat bus")

// 96 kHz, ADAT: adat 5/6 and 7/8 greyed, S/MUX automatic (checked, locked) even if the stored bit is 0.
l = ChannelLayout(rate: 96000, clock: clock(adatIn: true, adatOut: true))
check(l.inputStrips.filter { !$0.enabled }.map { $0.title } == ["adat 5/6 in", "adat 7/8 in"], "96k greyed strips")
check(l.smuxShownChecked && !l.smuxEnabled && l.adatBusHalfGreyed, "96k smux automatic")
check(l.inputBusMeters.filter { !$0.enabled }.map { $0.title } == ["adt 5/6", "adt 7/8"], "96k input bus meters")

// 96 kHz, S/PDIF in and out: nothing greyed, S/MUX not applicable.
l = ChannelLayout(rate: 96000, clock: clock(adatIn: false, adatOut: false, smux: true))
check(l.inputStrips.count == 5 && l.inputStrips.last!.kind == .spdif, "96k spdif strips")
check(!l.smuxEnabled && l.smuxShownChecked, "spdif out: smux greyed, state kept")
check(l.returnBus == .spdif, "spdif bus")

// 44.1 kHz, ADAT in / S/PDIF out (mixed): ADAT inputs, S/PDIF return bus, S/MUX greyed.
l = ChannelLayout(rate: 44100, clock: clock(adatIn: true, adatOut: false))
check(l.inputStrips.count == 8 && l.returnBus == .spdif && !l.smuxEnabled, "mixed mode")
check(!l.spdifPortEnabled, "spdif port greyed with adat in")

// 192 kHz: 2 in analog, no digital anywhere.
l = ChannelLayout(rate: 192000, clock: clock(adatIn: true, adatOut: true, smux: true))
check(l.inputStrips.count == 1 && l.inputStrips[0].kind == .analog, "192k one strip")
check(!l.digitalAvailable && !l.portCardEnabled && !l.externalDigitalSyncEnabled && !l.spdifPortEnabled, "192k digital off")
check(l.returnBus == .none && !l.hasDigitalOutputStrip && !l.smuxEnabled && l.inputBusMeters.count == 1, "192k buses")

// MAClockSource codec: default, bit fields, gate.
var c = MAClockSource.factoryDefault
check(c.raw == 0x4000_0003 && c.sync == .internalMuted, "factory default")
c.sync = .wordClock; c.adatIn = true; c.smux = true
check(c.raw == 0x4000_0002 | 0x08 | 0x20, "bits")
c.raw |= 0x8000_0000
check(c.wireValue == (0x4000_0000 | 0x0000_002A), "wire value clears bit 31, keeps gate")

// Index helpers.
check(MAIndex.mixerChannel(18)! == ("Input", 18) && MAIndex.mixerChannel(19)! == ("SWReturn", 1) && MAIndex.mixerChannel(33) == nil, "mixer ids")
check(MAIndex.outputChannel(15)! == ("Headphone", 1) && MAIndex.outputChannel(19)! == ("AuxOutput", 1), "output ids")
check(MARotaryMask.inputPairBit(1) == 0x1_0000 && MARotaryMask.outputPairBit(1) == 0x1_0000_0000 && MARotaryMask.swReturnPairBit(1) == 1, "rotary bits")


// DeviceModel with a mock transport.
final class Mock: PropertyTransport {
    var props: [String: Any] = [
        "MAOutputLevels": [NSNumber(value: 0), NSNumber(value: 0), NSNumber(value: 0), NSNumber(value: 0)],
        "MAOutputMute": [NSNumber(value: 0), NSNumber(value: 0), NSNumber(value: 0), NSNumber(value: 0)],
        "MAClockSource": NSNumber(value: 0x4000_0003 as UInt32),
        "MASampleRate": NSNumber(value: 48000),
        "MAMomentarySwitchRoutings": [NSNumber(value: 1), NSNumber(value: 2), NSNumber(value: 3), NSNumber(value: 4)],
    ]
    var writes: [[String: Any]] = []
    func copyProperties() -> [String: Any] { return props }
    func setProperties(_ p: [String: Any]) -> kern_return_t { writes.append(p); for (k, v) in p { props[k] = v }; return 0 }
}
let mock = Mock()
let m = DeviceModel(transport: mock)
check(m.ints(MAKeys.outputLevels) == [0, 0, 0, 0] && m.sampleRate == 48000, "model read")
m.setLevel(MAKeys.outputLevels, channel: 3, value: -10, linked: true)
check(m.ints(MAKeys.outputLevels) == [0, 0, -10, -10] && mock.writes.isEmpty, "linked level queued, not yet written")
m.setLevel(MAKeys.outputLevels, channel: 1, value: -500, linked: false)
check(m.element(MAKeys.outputLevels, 1) == MARange.levelMin, "output level clamped to the console floor (outputs left the monitor law, R127)")
m.commit()
check(mock.writes.count == 1 && (mock.writes[0]["MAOutputLevels"] as! [NSNumber]).map { $0.intValue } == [MARange.levelMin, 0, -10, -10], "one batched write")
// Balance: R35 leaves the right fader and lowers the left one by 35 steps.
m.setLevel(MAKeys.outputLevels, channel: 3, value: 0, linked: true)
m.setBalance(MAKeys.outputLevels, pair: 2, balance: 35)
check(m.element(MAKeys.outputLevels, 3) == -35 && m.element(MAKeys.outputLevels, 4) == 0 && m.balance(MAKeys.outputLevels, pair: 2) == 35, "balance R35")
m.setBalance(MAKeys.outputLevels, pair: 2, balance: -20)
check(m.element(MAKeys.outputLevels, 3) == 0 && m.element(MAKeys.outputLevels, 4) == -20, "balance L20")
// Clock read-modify-write keeps the gate and other bits.
m.updateClock { $0.adatIn = true }
check(UInt32((mock.props["MAClockSource"] as! NSNumber).uint64Value) == 0x4000_000B, "clock rmw")
// Pair flags write both elements.
m.setPairFlag(MAKeys.outputMute, pair: 1, on: true)
check(m.ints(MAKeys.outputMute) == [1, 1, 0, 0], "pair mute")
// A/B sheet keeps entry 0.
m.setABSources(a: 4, b: 2)
check((mock.props["MAMomentarySwitchRoutings"] as! [NSNumber]).map { $0.intValue } == [4, 2, 3, 4], "sheet entries (0 = A, 1 = B)")
// Peak math (A2-P9 §7.5): 0 -> -64, 32768 -> 0 dB, 16384 -> -6 dB (ceil).
check(peakToDB(0) == -64 && peakToDB(32768) == 0 && peakToDB(16384) == -6 && peakToDB(1) == -64, "peak dB (below -64 clamps)")
let pk = PeakMap(counts: TranslatorCounts(outputs: 20, mixer: 32, inputs: 18, headphone: 4))     // FW1814 (A2-P13 §1)
check(pk.input(1) == 0 && pk.output(1) == 18 && pk.swReturn(14) == 51 && pk.phones(1) == pk.output(15) && pk.auxOut(2) == pk.output(20), "FW1814 peak slots")
let apk = PeakMap(counts: TranslatorCounts(outputs: 10, mixer: 10, inputs: 4, headphone: 2))    // Audiophile (PNAP §1, §6)
check(apk.input(4) == 3 && apk.output(1) == 4 && apk.output(10) == 13 && apk.swReturn(1) == 14 && apk.swReturn(6) == 19 && apk.phones(1) == 10 && apk.auxOut(1) == 12, "Audiophile peak slots: inputs 0-3, outputs 4-13, SW returns 14-19")
let f4 = PeakMap(counts: TranslatorCounts(outputs: 14, mixer: 14, inputs: 4, headphone: 2), swReturnsFirst: true)    // FW410 (kext ReadHSCIData @0x15198)
check(f4.input(4) == 3 && f4.swReturn(1) == 4 && f4.swReturn(10) == 13 && f4.output(1) == 14 && f4.output(10) == 23 && f4.phones(1) == 24 && f4.auxOut(2) == 27,
      "FW410 peak slots: inputs 0-3, SW returns 4-13, outputs 14-23, phones 24-25, aux 26-27")
check(TranslatorCounts(outputs: 14, mixer: 14, inputs: 4, headphone: 2).auxSendElement(swReturn: 1) == 5, "aux-send element of FW410 SW return 1 = 5 (MAAuxSendLevels: inputs first, capture 103)")
check(TranslatorCounts(outputs: 10, mixer: 10, inputs: 4, headphone: 2).auxSendElement(swReturn: 1) == 5 && TranslatorCounts(outputs: 20, mixer: 32, inputs: 18, headphone: 4).auxSendElement(swReturn: 1) == 19, "aux-send element of SW return 1: Audiophile 5, FW1814 19")

// FirmwareInfo: the live block values of Q-P3 §3 (SL, runtime mode).
var blk = [UInt8](repeating: 0, count: 0x50)
for (i, c) in "bridgeCo".utf8.enumerated() { blk[i] = c }
blk[0x10...0x17] = [0x04, 0x6c, 0x0d, 0x00, 0xfb, 0xb9, 0x74, 0x00]
blk[0x18...0x1B] = [0, 0, 0, 0x83]; blk[0x1C...0x1F] = [0, 0, 0, 1]
for (i, c) in "20070713".utf8.enumerated() { blk[0x20 + i] = c }
for (i, c) in "080440".utf8.enumerated() { blk[0x28 + i] = c }
for (i, c) in "20040330".utf8.enumerated() { blk[0x40 + i] = c }
for (i, c) in "025909".utf8.enumerated() { blk[0x48 + i] = c }
let fw = FirmwareInfo.parse(Data(blk))!
check(fw.hardwareModel == 131 && fw.hardwareRevision == 1 && fw.guid == "0xD6C040074B9FB", "firmware info ids")
check(fw.firmwareDate == "20070713" && fw.bootloaderTime == "025909", "firmware info dates")
check(FirmwareInfo.display(date: fw.firmwareDate, time: fw.firmwareTime).hasPrefix("2007-07-13 08:04:40 "), "firmware date format")
blk[0x18...0x1B] = [0x83, 0, 0, 0]                               // little-endian fallback
check(FirmwareInfo.parse(Data(blk))!.hardwareModel == 131, "firmware info little-endian fallback")

// Headphone safety: at panel open every phones level above -86 is lowered to -86 and written at once; levels below it stay.
let hpMock = Mock()
hpMock.props["MAHeadphoneLevels"] = [NSNumber(value: 0), NSNumber(value: -20), NSNumber(value: -120), NSNumber(value: -86)]
let hpModel = DeviceModel(transport: hpMock)
hpModel.limitPhonesLevelsAtStartup()
check(hpMock.writes.count == 1 && (hpMock.writes[0]["MAHeadphoneLevels"] as! [NSNumber]).map { $0.intValue } == [-86, -86, -120, -86], "phones limited at start-up, lower levels kept")
let hpMock2 = Mock()
hpMock2.props["MAHeadphoneLevels"] = [NSNumber(value: -100), NSNumber(value: -90), NSNumber(value: -129), NSNumber(value: -86)]
DeviceModel(transport: hpMock2).limitPhonesLevelsAtStartup()
check(hpMock2.writes.isEmpty, "no write when every phones level is already at or below the ceiling")
// Monitor fader law (plan v7 AP-5.4): dB-linear over 72 dB, centre -36 dB, bottom = off; console law unchanged
check(FaderLaw.monitor.fraction(forValue: 0) == 0 && FaderLaw.monitor.fraction(forValue: -72) == 0.5 && FaderLaw.monitor.fraction(forValue: -144) == 1,
      "monitor law: 0 dB top, -36 dB centre, -72 dB bottom")
check(FaderLaw.monitor.value(forFraction: 0.5) == -72 && FaderLaw.monitor.value(forFraction: 0.25) == -36 && FaderLaw.monitor.value(forFraction: 0) == 0,
      "monitor law: fraction -> level")
check(FaderLaw.monitor.value(forFraction: 1) == -255 && FaderLaw.monitor.fraction(forValue: -255) == 1, "monitor law: the bottom is off (-255)")
check(FaderLaw.monitor.labels == [0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72], "monitor law: labels every 6 dB")
check(FaderLaw.console.fraction(forValue: -20) == 0.55 && FaderLaw.console.value(forFraction: 0.55) == -20 && FaderLaw.console.labels == [0, 2, 5, 10, 20, 30, 50],
      "console law unchanged (-10 dB at 0.55)")
// Monitor faders reach off (-255): the console floor -129 no longer clamps them; mixer levels keep -129
let flMock = Mock()
flMock.props["MAHeadphoneLevels"] = [NSNumber(value: -86), NSNumber(value: -86)]
flMock.props["MAInputLevels"] = [NSNumber(value: 0), NSNumber(value: 0)]
let flModel = DeviceModel(transport: flMock)
flModel.setLevel(MAKeys.headphoneLevels, channel: 1, value: -255, linked: true)
flModel.setLevel(MAKeys.inputLevels, channel: 1, value: -255, linked: true)
check(flModel.ints(MAKeys.headphoneLevels) == [-255, -255] && flModel.ints(MAKeys.inputLevels) == [-129, -129], "monitor fader reaches off; mixer floor stays -129")
flModel.setLevel(MAKeys.headphoneLevels, channel: 1, value: -140, linked: true)
check(flModel.ints(MAKeys.headphoneLevels) == [-140, -140], "monitor fader below -64.5 dB (-140 = -70 dB)")
// R127 (user, 2026-10-21): only the phones keep the monitor law; outputs and aux out are back on the console law and floor
check(DeviceModel.law(MAKeys.headphoneLevels) == .monitor && DeviceModel.law(MAKeys.outputLevels) == .console
      && DeviceModel.law(MAKeys.auxOutputLevels) == .console && DeviceModel.law(MAKeys.inputLevels) == .console, "monitor law: phones only")
check(DeviceModel.floor(MAKeys.auxOutputLevels) == MARange.levelMin && DeviceModel.floor(MAKeys.headphoneLevels) == FaderLaw.off,
      "floors: aux out -129, phones off")
// Editions (About tab): no OAEdition key = Classic with the 0.2.x caption; Modern names its OS range and has no 10.13.6 text
check(PanelEdition.current == .classic && PanelEdition.classic.caption == "macOS High Sierra (10.13.6) Driver" && PanelEdition.classic.creditsNote == nil,
      "edition: default Classic, 0.2.x caption")
check(PanelEdition(rawValue: "modern") == .modern && !PanelEdition.modern.caption.contains("10.13") && PanelEdition.modern.caption.contains("macOS 11 to 15"),
      "edition: Modern caption")
// Status menu / big meter logic (plan v8 AP-5.5 / AP-5.6)
check(StatusIconState.of(nil) == .offline, "status icon: no unit = offline")
let stMock = Mock()
stMock.props["MAGlobalMute"] = NSNumber(value: 1); stMock.props["MARateLocked"] = NSNumber(value: 0)
check(StatusIconState.of(DeviceModel(transport: stMock)) == StatusIconState(online: true, muted: true, noSync: true), "status icon: muted and not locked")
stMock.props["MAGlobalMute"] = NSNumber(value: 0); stMock.props["MARateLocked"] = NSNumber(value: 1)
check(StatusIconState.of(DeviceModel(transport: stMock)) == StatusIconState(online: true, muted: false, noSync: false), "status icon: online")
let pfModel = DeviceModel(transport: Mock())                               // 4 output levels, 4 output mutes
let pf = pfModel.fittingPreset(["MAOutputLevels": [-10, -10, -10, -10].map { NSNumber(value: $0) },
                                "MAOutputMute": [0, 0, 0, 0, 0, 0].map { NSNumber(value: $0) }, "NotAKey": NSNumber(value: 1)])
check(pf.count == 1 && pf["MAOutputLevels"] != nil, "preset fit: same-length arrays only, unknown keys dropped")
check(BigMeterScale.db(raw: 32768) == 0 && abs(BigMeterScale.db(raw: 3277) + 20) < 0.01 && BigMeterScale.db(raw: 0) == -Double.infinity, "big meter: raw -> dBFS")
check(BigMeterScale.fraction(0) == 1 && BigMeterScale.fraction(-30) == 0.5 && BigMeterScale.fraction(-60) == 0 && BigMeterScale.fraction(-80) == 0, "big meter: dB-linear -60...0")
check(BigMeterScale.litSegments(0) == 80 && BigMeterScale.litSegments(-60) == 0 && BigMeterScale.litSegments(-59.9) == 1 && BigMeterScale.litSegments(-30) == 40
      && BigMeterScale.litSegments(-29.9) == 41, "big meter: 80 segments of 0.75 dB")
check(BigMeterScale.zone(segment: 0) == 0 && BigMeterScale.zone(segment: 55) == 0 && BigMeterScale.zone(segment: 56) == 1 && BigMeterScale.zone(segment: 71) == 1
      && BigMeterScale.zone(segment: 72) == 2 && BigMeterScale.zone(segment: 79) == 2, "big meter: zones green to -18, amber to -6, red above")
check(BigMeterScale.marks.count == 21 && BigMeterScale.labels == [-60, -54, -48, -42, -36, -30, -24, -18, -12, -6, 0], "big meter: marks every 3 dB, labels every 6")
// Reset: the factory phones level is 0 dB, so the ceiling is written right after the reset, in a second write (Audiophile: 2 phones channels).
let hpMock3 = Mock()
hpMock3.props["MAHeadphoneLevels"] = [NSNumber(value: -100), NSNumber(value: -100)]
DeviceModel(transport: hpMock3).resetToFactory()
check(hpMock3.writes.count == 2 && hpMock3.writes[0]["MAMetaResetToFactorySettings"] != nil
      && (hpMock3.writes[1]["MAHeadphoneLevels"] as! [NSNumber]).map { $0.intValue } == [-86, -86], "reset is followed by the phones ceiling")

// Supported rates: CoreAudio's list when the HAL device exists, the kext's six rates otherwise (A2-P8 §10).
final class MockRate: RateBackend {
    var nominalRate: UInt32? = 48000
    var availableRates: [UInt32] = []
    var onRateChanged: (() -> Void)?
    func setNominalRate(_ hz: UInt32) -> Bool { nominalRate = hz; return true }
}
let mr = MockRate()
let fw1814Rates: [UInt32] = [44100, 48000, 88200, 96000, 176400, 192000]
let rm = DeviceModel(transport: Mock(), rate: mr, fallbackRates: fw1814Rates)
check(rm.supportedRates == fw1814Rates, "no HAL list yet: the six default rates")
mr.availableRates = [44100, 48000, 88200, 96000]
check(rm.supportedRates == [44100, 48000, 88200, 96000], "the panel follows CoreAudio's list")
check(DeviceModel(transport: Mock(), fallbackRates: fw1814Rates).supportedRates == fw1814Rates, "no CoreAudio backend: the model's rates")

// Meter slots: ADAT software-return channel n sits at slot 6 + n (after 4 analog + 2 S/PDIF), like the 18 inputs and the outputs.
check(pk.swReturn(5) == 42 && pk.swReturn(6 + 1) == pk.swReturn(7) && pk.swReturn(6 + 8) == 51, "ADAT sw-return peak slots 7...14")

// Rate follow (RATE_FOLLOW_PLAN_v1). The simulated interface behaves as the FW1814 did in capture 98: the source's rate is seen only within the
// interface's speed range (44.1/48 or 88.2/96), nothing for 6 s after a rate change; CoreAudio applies a rate 1 s after it is asked for.
final class FollowSim {
    let f = RateFollower()
    var rate: UInt32, source: UInt32, busy = false, enabled = true, external = true, keyPublished = true
    var t: TimeInterval = 0, changedAt: TimeInterval = -100, pending: (UInt32, TimeInterval)?
    var sets: [UInt32] = []
    init(rate: UInt32, source: UInt32) { self.rate = rate; self.source = source }
    func run(_ seconds: TimeInterval) {
        let end = t + seconds
        while t < end {
            if let (hz, at) = pending, t >= at { rate = hz; changedAt = t; pending = nil }
            let fast = { (r: UInt32) in r >= 88200 }
            let seen: UInt32 = (source != 0 && fast(source) == fast(rate) && t - changedAt >= 6) ? source : 0
            let i = RateFollowInput(enabled: enabled, external: external, detected: keyPublished ? seen : nil, current: rate,
                                    available: [44100, 48000, 88200, 96000], busy: busy)
            if let hz = f.tick(i, now: t) { sets.append(hz); pending = (hz, t + 1) }
            t += 0.25
        }
    }
}
var fs = FollowSim(rate: 48000, source: 44100); fs.run(15)
check(fs.rate == 44100 && fs.sets == [44100] && fs.f.status == .following, "follow: 48 -> 44.1 source in the same range")
fs = FollowSim(rate: 96000, source: 88200); fs.run(15)
check(fs.rate == 88200 && fs.sets == [88200], "follow: 96 -> 88.2 source in the same range")
fs = FollowSim(rate: 48000, source: 96000); fs.run(30)
check(fs.rate == 96000 && fs.sets == [96000] && fs.f.status == .following, "follow: 48 -> 96 source by one try of the other range")
fs = FollowSim(rate: 96000, source: 44100); fs.run(30)
check(fs.rate == 44100 && fs.sets == [48000, 44100], "follow: 96 -> 44.1 source: try 48, then the exact rate")
fs = FollowSim(rate: 48000, source: 0); fs.run(120)
check(fs.rate == 48000 && fs.sets == [96000, 48000] && fs.f.status == .noSource, "follow: no source: one try, back, no retry")
fs = FollowSim(rate: 48000, source: 44100); fs.busy = true; fs.run(10)
check(fs.sets.isEmpty && fs.f.status == .waiting(44100) && fs.f.note == "Source at 44.1 kHz - follows when playback stops", "follow: busy waits")
fs.busy = false; fs.run(15)
check(fs.rate == 44100 && fs.sets == [44100], "follow: follows once playback stops")
fs = FollowSim(rate: 96000, source: 96000); fs.run(5)
check(fs.sets.isEmpty && fs.f.status == .following, "follow: already at the source's rate")
fs.source = 88200; fs.run(0.75); fs.source = 48000; fs.run(30)    // the POD steps 96 -> 88.2 -> 48: the 0.8 s at 88.2 is ignored
check(fs.rate == 48000 && fs.sets == [48000] && !fs.sets.contains(88200), "follow: a source passing through 88.2 is not followed there")
fs = FollowSim(rate: 48000, source: 44100); fs.enabled = false; fs.run(20)
check(fs.sets.isEmpty && fs.f.status == .off, "follow: setting off")
fs = FollowSim(rate: 48000, source: 44100); fs.external = false; fs.run(20)
check(fs.sets.isEmpty && fs.f.status == .off, "follow: internal clock")
fs = FollowSim(rate: 48000, source: 44100); fs.keyPublished = false; fs.run(20)
check(fs.sets.isEmpty && fs.f.status == .off, "follow: driver without MAExternalSyncRate")
fs = FollowSim(rate: 48000, source: 0); fs.run(120); fs.external = false; fs.run(1); fs.external = true; fs.source = 96000; fs.run(30)
check(fs.rate == 96000 && fs.sets == [96000, 48000, 96000], "follow: re-selecting external sync re-arms the try")
check(RateFollower.otherRange(44100, []) == 96000 && RateFollower.otherRange(88200, [44100, 88200]) == 44100, "follow: other speed range")

// Advanced window (PANEL_REDESIGN_PLAN_v3): gate, scenes, templates, rises.
check(AdvancedGate.available(modelAllows: true, rate: 48000) && AdvancedGate.available(modelAllows: true, rate: 44100), "advanced: 44.1/48 open")
check(!AdvancedGate.available(modelAllows: true, rate: 96000) && !AdvancedGate.available(modelAllows: true, rate: 192000), "advanced: other rates locked")
check(!AdvancedGate.available(modelAllows: false, rate: 48000), "advanced: other models never")
check(AdvancedMode.first == .daw && AdvancedMode.dual == [.daw, .console], "advanced: opens in DAW; Dual = DAW + Console")
func nums(_ a: [Int]) -> [NSNumber] { return a.map { NSNumber(value: Int32($0)) } }
let hw: [String: Any] = [
    "MAInputRoutingV2": nums([3, 1, 0, 0, 2, 0, 0, 0, 0]), "MASWReturnRoutingV2": nums([2, 1]), "MASWReturnLevels": nums([-20, -20, 0, 0]),
    "MASWReturnMute": nums([0, 1, 0, 0]), "MASWReturnSolo": nums([0, 0, 1, 0]), "MAInputSolo": nums(Array(repeating: 0, count: 18)),
    "MAOutputLevels": nums([-40, -40, -12, -12]), "MAOutputMute": nums([0, 0, 1, 1]), "MAOutputSolo": nums([0, 0, 0, 0]),
    "MAHeadphoneLevels": nums([-86, -86, -86, -86]), "MAHeadphoneMute": nums([0, 0, 0, 0]), "MAMonitorAuxOnOutput": nums([1, 0, 0]),
    "MAHeadphoneSourceV2": NSNumber(value: 4), "MAHeadphoneSecondSource": NSNumber(value: 4),
    "MAGlobalMute": NSNumber(value: 1), "MAGlobalDim": NSNumber(value: 0), "MASampleRate": NSNumber(value: 48000)]
let cap = AdvancedScene.capture(hw)
check(cap["MAGlobalMute"] == nil && cap["MAGlobalDim"] == nil && cap["MASampleRate"] == nil && cap["MAOutputLevels"] != nil, "advanced: scene keeps mixer keys only")
let pt = AdvancedScene.passthrough(from: hw)
check(AdvancedScene.ints(pt, MAKeys.inputRoutingV2) == Array(repeating: 0, count: 9), "advanced: passthrough monitors no input")
check(AdvancedScene.ints(pt, MAKeys.swReturnRoutingV2) == [1, 2], "advanced: passthrough returns to their own pair")
check(AdvancedScene.ints(pt, MAKeys.swReturnLevels) == [0, 0, 0, 0] && AdvancedScene.ints(pt, MAKeys.swReturnMute) == [0, 0, 0, 0], "advanced: returns at unity")
check(AdvancedScene.ints(pt, MAKeys.monitorAuxOnOutput) == [0, 0, 0], "advanced: outputs on their main mix")
check((pt["MAHeadphoneSourceV2"] as? NSNumber)?.intValue == 1 && (pt["MAHeadphoneSecondSource"] as? NSNumber)?.intValue == 2, "advanced: phones on out 1/2 and 3/4")
check(AdvancedScene.ints(pt, MAKeys.outputLevels) == [-40, -40, -12, -12] && AdvancedScene.ints(pt, MAKeys.outputMute) == [0, 0, 1, 1], "advanced: passthrough keeps output levels")
check(AdvancedScene.rises(from: hw, to: AdvancedScene.template(.daw, current: hw)).isEmpty, "advanced: DAW never raises an output")
let un = AdvancedScene.template(.console, current: hw)
check(AdvancedScene.ints(un, MAKeys.outputLevels) == [0, 0, 0, 0] && AdvancedScene.ints(un, MAKeys.outputMute) == [0, 0, 0, 0], "advanced: console at unity")
let r = AdvancedScene.rises(from: hw, to: un)
check(r.map { $0.channel } == [1, 2, 3, 4] && r[0].from == -40 && r[2].from == MALevel.minimum && !r.contains { $0.to != 0 || $0.key != "MAOutputLevels" }, "advanced: console rises listed (muted = minimum)")
check(AdvancedScene.template(.monitor, current: hw).count == cap.count, "advanced: monitor starts from the mixer as it is")
let sw = AdvancedSwitch.plan(from: .daw, to: .console, current: hw, stored: nil)
check(sw.leaving.count == cap.count && sw.rises.count == 4, "advanced: switch plan")
let back = AdvancedSwitch.plan(from: .console, to: .daw, current: un, stored: cap)
check(AdvancedScene.ints(back.applying, MAKeys.outputLevels) == [-40, -40, -12, -12] && back.rises.isEmpty, "advanced: a stored scene comes back, lower levels need no question")

var hw2 = hw; hw2["MARotaryControlV2"] = NSNumber(value: MARotaryMask.factoryDefault)
check((AdvancedScene.passthrough(from: hw2)["MARotaryControlV2"] as? NSNumber)?.uint64Value == MARotaryMask.outputPairBit(1), "advanced: DAW knob on Line 1/2")
// FW1814 channels of the Advanced meter views (Core Audio order, peak slots as FW1814Profile.meters)
let ch = fw1814AdvancedChannels(ChannelLayout(rate: 48000, clock: clock(adatIn: true, adatOut: true)), peaks: pk)
check(ch.inputs.count == 16 && ch.inputs.map { $0.ca } == Array(1...16), "advanced: ADAT 16 in, Core Audio 1-16")
check(ch.inputs[8].slot == pk.input(11) && ch.inputs[15].slot == pk.input(18) && ch.inputs[0].jack == "Mic/Line 1", "advanced: ADAT in slots 11-18")
check(ch.outputs.count == 12 && ch.outputs[4].ca == 5 && ch.outputs[4].slot == pk.output(7) && ch.outputs[11].slot == pk.output(14), "advanced: ADAT out = Core Audio 5-12, slots 7-14")
check(ch.phones.count == 2 && ch.phones[1].slots.0 == pk.phones(3), "advanced: two phones")
let chs = fw1814AdvancedChannels(ChannelLayout(rate: 44100, clock: clock(adatIn: false, adatOut: false)), peaks: pk)
check(chs.inputs.count == 10 && chs.inputs[9].slot == pk.input(10) && chs.outputs.count == 6 && chs.outputs[5].slot == pk.output(6), "advanced: S/PDIF 10 in / 6 out")

check((AdvancedScene.unity(from: hw2)["MARotaryControlV2"] as? NSNumber)?.uint64Value == 0, "advanced: Console knob on nothing")
check(AdvancedScene.offUnity(AdvancedScene.unity(from: hw)).isEmpty, "advanced: unity scene is calibrated")
check(AdvancedScene.offUnity(hw) == ["Line 1", "Line 2", "Line 3", "Line 4", "DAW 1", "DAW 2"], "advanced: off-unity list (levels and mutes, not solos)")

// Monitor mode logic (AdvancedMonitor.swift)
func mscene() -> [String: Any] {
    return ["MAInputLevels": nums(Array(repeating: -129, count: 18)), "MAInputPan2": nums((0..<18).map { $0 % 2 == 0 ? -255 : 255 }),
            "MAInputRoutingV2": nums(Array(repeating: 0, count: 9)), "MASWReturnLevels": nums([0, 0, 0, 0]), "MASWReturnRoutingV2": nums([1, 2]),
            "MAAuxSendLevels": nums(Array(repeating: -255, count: 22))]
}
let mixesM = MonitorMix.defaults                                       // Monitors stereo on 1/2, Cue A mono on 3/4, Cue B mono on Aux
var mm = MonitorMixer(mscene())
check(mm.member(.daw(1), .mix12) && mm.member(.daw(2), .mix34) && !mm.member(.input(1), .mix34), "monitor: passthrough membership")
mm.setMember(.input(1), .mix34, true, mixes: mixesM)
check(mm.member(.input(1), .mix34) && mm.level(.input(1)) == MonitorMixer.joinLevel && mm.pan(1) == 0, "monitor: In 1 joins Cue A at -12 dB, centred")
check(!mm.member(.input(2), .mix34) && mm.routed(.input(2), .mix34), "monitor: the partner rides in silent (pair routing)")
mm.setMember(.input(2), .mix34, true, mixes: mixesM)
check(mm.member(.input(2), .mix34) && mm.pan(2) == 0, "monitor: In 2 joins, centred (mono mix)")
mm.setSend(.input(1), .mix34, -6)
mm.setMember(.input(1), .mix12, true, mixes: mixesM)
check(mm.member(.input(1), .mix12) && mm.sharedLevel(.input(1)) && mm.send(.input(1), .mix12) == -6, "monitor: one level for Mix 1/2 and 3/4")
check(mm.member(.input(2), .mix12) && mm.notes.contains { $0.hasPrefix("In 2 joins too") }, "monitor: partner in the other mix joins too, with a note")
mm.setPan(1, -255)
check(mm.panConflict(.input(1), mono: .mix34, mixes: mixesM) == .mix12, "monitor: stereo Monitors pans In 1: conflict shown in mono Cue A")
mm.centre(.mix34, mixes: mixesM)
check(mm.pan(1) == -255, "monitor: centring a mono mix leaves sources a stereo mix pans")
var mm2 = MonitorMixer(mscene())
mm2.setMember(.input(3), .mix34, true, mixes: mixesM); mm2.setMember(.input(4), .mix34, true, mixes: mixesM)
mm2.setMember(.input(3), .mix34, false, mixes: mixesM)
check(!mm2.member(.input(3), .mix34) && mm2.member(.input(4), .mix34) && mm2.level(.input(3)) == MonitorMixer.off, "monitor: In 3 leaves, partner stays (level off)")
mm2.setMember(.input(4), .mix34, false, mixes: mixesM)
check(!mm2.routed(.input(4), .mix34), "monitor: last of the pair leaves: pair unrouted")
var mm3 = MonitorMixer(mscene())
mm3.setMember(.input(5), .aux, true, mixes: mixesM); mm3.setMember(.daw(2), .aux, true, mixes: mixesM)
check(mm3.member(.input(5), .aux) && !mm3.member(.input(6), .aux) && mm3.member(.daw(2), .aux), "monitor: aux sends per channel")
check(mm3.changed == ["MAAuxSendLevels"] && AdvancedScene.ints(mm3.s, MAKeys.auxSendLevels)[20] == MonitorMixer.joinLevel, "monitor: DAW 3/4 aux sends are elements 21-22")
check(mm3.writes.keys.sorted() == ["MAAuxSendLevels"], "monitor: writes only what changed")
// stereo source (an input pair as one strip)
var mm4 = MonitorMixer(mscene())
let st7 = MonitorSource.stereo(7)
check(st7.partner == nil && st7.isStereoInput && MonitorSource.input(7).partner == 8, "monitor: a stereo source has no partner")
mm4.setMember(st7, .mix12, true, mixes: mixesM)                        // Monitors is stereo
check(mm4.member(st7, .mix12) && mm4.level(.input(8)) == MonitorMixer.joinLevel && mm4.pan(7) == -255 && mm4.pan(8) == 255, "monitor: stereo source joins a stereo mix hard L / R, both levels")
mm4.setMember(st7, .mix34, true, mixes: mixesM)                        // Cue A is mono, but Monitors pans it: pans stay
check(mm4.member(st7, .mix34) && mm4.pan(7) == -255, "monitor: stereo source in a mono mix keeps the stereo mix's pans (shared)")
var mm5 = MonitorMixer(mscene())
let out3 = MixLine(bus: .mix34, side: .left), out4 = MixLine(bus: .mix34, side: .right)
mm5.setLineMember(MonitorSource.stereo(9), out3, true, mixes: mixesM)
check(mm5.pan(9) == -255 && mm5.pan(10) == -255 && mm5.member(MonitorSource.stereo(9), out3) && !mm5.member(MonitorSource.stereo(9), out4), "lines: a linked pair on Out 3 only: both channels hard left")
mm5.spread(.mix34, stereoPairs: [9])
check(mm5.pan(9) == -255 && mm5.pan(10) == 255, "monitor: the bus turns stereo: the pair spreads")
mm5.setMember(MonitorSource.stereo(9), .mix34, false, mixes: mixesM)
check(!mm5.routed(.input(10), .mix34), "monitor: stereo source leaves: pair unrouted")

// mono lines (user, 2026-10-28): a MONO bus is two independent outputs; the panel pans each source to the lines it is IN on
let linesMixes = [MonitorMix(bus: .mix12, name: "Monitors", mono: false), MonitorMix(bus: .mix34, name: "Wedge 1", mono: true, rightName: "Wedge 2"),
                  MonitorMix(bus: .aux, name: "Aux", mono: false)]
check(linesMixes[1].lines == [out3, out4] && linesMixes[0].lines.count == 1 && out3.output == "Out 3" && out4.output == "Out 4", "lines: a mono bus is Out 3 and Out 4")
var ml = MonitorMixer(mscene())
ml.setLineMember(.input(1), out3, true, mixes: linesMixes)
check(ml.member(.input(1), out3) && !ml.member(.input(1), out4) && ml.pan(1) == -255, "lines: In 1 IN on Out 3 only: hard left")
check(!ml.member(.input(2), out3) && !ml.member(.input(2), out4), "lines: only the clicked channel (its partner, used nowhere, stays out)")
ml.setLineMember(.input(2), out4, true, mixes: linesMixes)
check(ml.member(.input(2), out4) && !ml.member(.input(2), out3) && ml.pan(2) == 255 && ml.member(.input(1), out3) && !ml.member(.input(1), out4), "lines: In 2 on Out 4 only, In 1 untouched")
ml.setLineMember(.input(1), out4, true, mixes: linesMixes)
check(ml.member(.input(1), out3) && ml.member(.input(1), out4) && ml.pan(1) == 0, "lines: In 1 on both lines: centre")
ml.setLineMember(.input(1), out3, false, mixes: linesMixes)
check(!ml.member(.input(1), out3) && ml.member(.input(1), out4) && ml.pan(1) == 255 && ml.member(.input(2), out4), "lines: In 1 off Out 3: Out 4 only; In 2 stays")
check(!ml.forcedPartner(.input(1), .mix34), "lines: nothing forced while no input is in the other bus")
// the hardware case: In 2 is in Monitors (Mix 1/2) at a level; In 1 joining Mix 3/4 brings it (pair routing, shared level)
var mf = MonitorMixer(mscene())
mf.setMember(.input(2), .mix12, true, mixes: linesMixes)
mf.setLineMember(.input(1), out3, true, mixes: linesMixes)
check(mf.member(.input(2), .mix34) && mf.forcedPartner(.input(2), .mix34) && mf.notes.contains { $0.hasPrefix("In 2 joins too") }, "lines: a partner in the other bus comes along, marked as forced")
// mono DAW returns: no pan in the hardware, odd lands left, even right; pair routing and one level like the inputs
let d1 = MonitorSource.dawMono(1), d2 = MonitorSource.dawMono(2), d3 = MonitorSource.dawMono(3)
check(d1.fixedSide == .left && d2.fixedSide == .right && d1.partnerSource == d2 && d1.label == "DAW 1", "returns: fixed sides, partner")
var mret = MonitorMixer(mscene())                                            // mscene: DAW 1/2 -> Mix 1/2, DAW 3/4 -> Mix 3/4, levels 0
check(mret.member(d3, out3) && !mret.member(d3, out4) && mret.member(MonitorSource.dawMono(4), out4), "returns: DAW 3 on Out 3 only, DAW 4 on Out 4")
check(!mret.reachable(d3, out4) && mret.reachable(d3, out3) && mret.reachable(d3, MixLine(bus: .mix34, side: nil)), "returns: DAW 3 cannot reach Out 4")
mret.setLineMember(d3, out4, true, mixes: linesMixes)
check(mret.changed.isEmpty, "returns: IN on an unreachable line writes nothing")
mret.setLineMember(d3, out3, false, mixes: linesMixes)
check(!mret.member(d3, out3) && mret.member(MonitorSource.dawMono(4), out4) && mret.level(d3) == MonitorMixer.off, "returns: DAW 3 off Out 3, DAW 4 stays (level)")
var mr2 = MonitorMixer(mscene())
mr2.setLineMember(d1, out3, true, mixes: linesMixes)                       // DAW 2 is in Mix 1/2 at a level: it comes along
check(mr2.member(d1, out3) && mr2.member(d2, .mix34) && mr2.forcedPartner(d2, .mix34), "returns: a partner in the other bus comes along, marked")

// IN off then on gives the level back (user: "it always comes back as -12 dB"); a new source joins at 0 dB (user)
var mp = MonitorMixer(mscene())
mp.setLineMember(.input(5), out3, true, mixes: linesMixes); mp.setLineMember(.input(6), out4, true, mixes: linesMixes)
check(mp.level(.input(5)) == 0, "park: a silent input joins at 0 dB")
mp.setSend(.input(5), .mix34, -14)
mp.setLineMember(.input(5), out3, false, mixes: linesMixes)            // In 6 keeps the pair routed: In 5 is silenced
check(mp.level(.input(5)) == MonitorMixer.off && mp.parked["lvl.in5"] == -14, "park: OFF keeps the level aside")
mp.setLineMember(.input(5), out3, true, mixes: linesMixes)
check(mp.level(.input(5)) == -14 && mp.parked["lvl.in5"] == nil, "park: IN gives -14 dB back")
mp.setMember(.input(7), .aux, true, mixes: linesMixes); mp.setSend(.input(7), .aux, -20)
mp.setMember(.input(7), .aux, false, mixes: linesMixes); mp.setMember(.input(7), .aux, true, mixes: linesMixes)
check(mp.send(.input(7), .aux) == -20, "park: the Aux send comes back too")

// Aux mute and solo (user, 2026-10-28): M / S in the Aux act on the Aux sends only, keep the sources IN, and give the sends back
let send = { (m: MonitorMixer, el: Int) -> Int in AdvancedScene.ints(m.s, MAKeys.auxSendLevels)[el - 1] }
var ma = MonitorMixer(mscene())
ma.setMember(.input(3), .mix12, true, mixes: linesMixes); ma.setMember(.input(3), .aux, true, mixes: linesMixes); ma.setSend(.input(3), .aux, -10)
let lvl3 = ma.level(.input(3))
ma.setAuxMute(.input(3), true)
check(ma.auxMuted(.input(3)) && ma.member(.input(3), .aux) && ma.send(.input(3), .aux) == -10 && send(ma, 3) == MonitorMixer.off,
      "aux mute: send at the floor, the fader keeps -10, still IN")
check(ma.level(.input(3)) == lvl3 && ma.member(.input(3), .mix12) && !ma.changed.contains("MAInputMute"), "aux mute: Mix 1/2 and the channel mute untouched")
ma.setSend(.input(3), .aux, -6)
check(send(ma, 3) == MonitorMixer.off && ma.send(.input(3), .aux) == -6, "aux mute: the fader moves the kept send, the send stays cut")
ma.setAuxMute(.input(3), false)
check(!ma.auxMuted(.input(3)) && send(ma, 3) == -6 && ma.parked["auxkeep.in3"] == nil, "aux mute: M again gives -6 back")
ma.setAuxMute(.daw(1), true)
check(!ma.member(.daw(1), .aux) && ma.auxMuted(.daw(1)), "aux mute: a source not in the Aux stays out")
ma.setAuxMute(.daw(1), false)
ma.setAuxMute(.input(3), true); ma.setMember(.input(3), .aux, false, mixes: linesMixes)
check(!ma.member(.input(3), .aux) && !ma.auxMuted(.input(3)) && ma.parked["auxkeep.in3"] == nil && ma.parked["aux.in3"] == -6, "aux mute: OFF ends the mute, IN gives the send back")
ma.setMember(.input(3), .aux, true, mixes: linesMixes)
check(ma.send(.input(3), .aux) == -6 && send(ma, 3) == -6 && !ma.auxMuted(.input(3)), "aux mute: back IN unmuted at -6")
var raised = MonitorMixer(mscene()); raised.setSend(.input(5), .aux, -2)
var mb = MonitorMixer(raised.s, parked: ["auxmute.in5": 1, "auxkeep.in5": -4])
mb.applyAux()
check(!mb.auxMuted(.input(5)) && send(mb, 5) == -2 && mb.send(.input(5), .aux) == -2, "aux mute: a send raised elsewhere ends the mute")
// solo: In 3 at -6, In 4 at -8, DAW 2 at -3 in the Aux; S on In 4 cuts the others, keeps their sends; S off gives them back
var ms = MonitorMixer(ma.s)
ms.setMember(.input(4), .aux, true, mixes: linesMixes); ms.setSend(.input(4), .aux, -8)
ms.setMember(.dawMono(2), .aux, true, mixes: linesMixes); ms.setSend(.dawMono(2), .aux, -3)
ms.setAuxSolo(.input(4), true)
check(ms.auxSoloActive && ms.auxSoloed(.input(4)) && send(ms, 4) == -8 && send(ms, 3) == MonitorMixer.off && send(ms, 20) == MonitorMixer.off,
      "aux solo: only the soloed source is heard in the Aux")
check(ms.member(.input(3), .aux) && ms.send(.input(3), .aux) == -6 && ms.auxSilenced(.input(3)) && !ms.changed.contains("MAInputSolo") && ms.level(.input(3)) == lvl3,
      "aux solo: the others stay IN with their faders; Mix 1/2 and MAInputSolo untouched")
ms.setAuxSolo(.dawMono(2), true)
check(send(ms, 20) == -3 && send(ms, 3) == MonitorMixer.off, "aux solo: a second solo is heard too")
ms.setAuxMute(.input(4), true)
check(send(ms, 4) == MonitorMixer.off && ms.send(.input(4), .aux) == -8, "aux solo: M still wins on a soloed source")
ms.setAuxMute(.input(4), false); ms.setAuxSolo(.input(4), false); ms.setAuxSolo(.dawMono(2), false)
check(!ms.auxSoloActive && send(ms, 3) == -6 && send(ms, 4) == -8 && send(ms, 20) == -3 && ms.parked.keys.filter({ $0.hasPrefix("auxkeep.") }).isEmpty,
      "aux solo: the last S off gives every send back")
ms.setAuxSolo(.input(4), true); ms.setMember(.input(4), .aux, false, mixes: linesMixes)
check(!ms.auxSoloActive && send(ms, 3) == -6, "aux solo: OFF on the soloed source ends the solo")
check(MonitorSource.input(3).auxSide == .left && MonitorSource.input(4).auxSide == .right && MonitorSource.dawMono(2).auxSide == .right && MonitorSource.stereo(3).auxSide == nil,
      "aux: a stereo pair with no pan, odd channels left, even right")
check(MonitorMix.defaults[2].name == "Aux" && !MonitorMix.defaults[2].mono && MonitorMix.defaults[2].lines.count == 1, "aux: default name Aux, one stereo line")

// link (MAInputLink): the partner takes the first channel's level, send and pan; in a stereo mix the pair spreads
var mk = MonitorMixer(mscene())
mk.setMember(.input(3), .mix12, true, mixes: linesMixes); mk.setSend(.input(3), .mix12, -8)
mk.link(3, mixes: linesMixes)
check(mk.level(.input(4)) == -8 && mk.pan(3) == -255 && mk.pan(4) == 255, "link: partner takes the level; stereo mix spreads the pair")

// the front knob chip
check(KnobTarget.describe(MARotaryMask.outputPairBit(1)) == "Line 1/2" && KnobTarget.describe(0) == "nothing", "knob: describe")
check(KnobTarget.describe(MARotaryMask.outputPairBit(1) | MARotaryMask.outputPairBit(2)) == "Line 1/2 + Line 3/4", "knob: two pairs")
let l34 = KnobTarget.items[1], in12 = KnobTarget.items[2]
check(KnobTarget.choose(l34, current: MARotaryMask.outputPairBit(1)) == MARotaryMask.outputPairBit(1) | MARotaryMask.outputPairBit(2), "knob: same group adds a pair")
check(KnobTarget.choose(in12, current: MARotaryMask.outputPairBit(1)) == MARotaryMask.inputPairBit(1), "knob: another group replaces (one group at a time)")
check(KnobTarget.choose(l34, current: MARotaryMask.outputPairBit(1) | MARotaryMask.outputPairBit(2)) == MARotaryMask.outputPairBit(1), "knob: a chosen pair toggles off")

// custom modes (M6)
let cmode = CustomMode(id: "custom-1", name: "Drums tracking", base: .monitor)
check(CustomMode(plist: cmode.plist) == cmode && CustomMode(plist: ["ID": "x", "Name": "y", "Base": "nope"]) == nil, "custom mode round trip")
check(CustomMode.clean("   ", fallback: "Monitor 2") == "Monitor 2" && CustomMode.clean("A very long mode name indeed", fallback: "") == "A very long mode n", "custom mode name: trimmed, 18 chars")

// Hybrid loops
let lp = HybridLoop(name: "Vocal comp", device: "1176", sends: ["out.l3"], returns: ["in.a5"])
check(HybridLoop(plist: lp.plist) == lp && HybridLoop(plist: ["Name": "x", "Sends": ["out.l3"], "Returns": []]) == nil, "hybrid: loop round trip, mismatched loop refused")
check(HybridLoop.freeSends(ch.outputs, loops: [lp]).map { $0.id }.prefix(2) == ["out.l4", "out.adat1"], "hybrid: free sends skip Line 1/2 and used outputs")
check(HybridLoop.lineChannels(["out.l3", "out.l4", "out.adat2"]) == [3, 4], "hybrid: Line sends go through the mixer, ADAT direct")
check(HybridLoop.difference(send: -12, ret: -14) == -2 && HybridLoop.difference(send: -64, ret: -10) == nil, "hybrid: return vs send")

// Dual (two FW1814s as one aggregate)
let ua = DualUnitState(guid: "0xA", rate: 48000, sync: .internalMuted, locked: true, bus: 1)
var ub = DualUnitState(guid: "0xB", rate: 48000, sync: .wordClock, locked: true, bus: 2)
let goodAgg = DualAggregateState(exists: true, subUIDs: ["uidA", "uidB"], masterUID: "uidA", drift: ["uidB": false], inputs: 32, outputs: 24)
var items = DualSetup.check(ua, ub, uidA: "uidA", uidB: "uidB", aggregate: goodAgg)
check(DualSetup.ready(items) && items.count == 5 && items.last?.status == .ok, "dual: ready with word clock, aggregate, two buses")
ub = DualUnitState(guid: "0xB", rate: 48000, sync: .internalMuted, locked: true, bus: 1)
items = DualSetup.check(ua, ub, uidA: "uidA", uidB: "uidB", aggregate: .missing)
check(!DualSetup.ready(items) && items.contains { $0.fix == .wordClockB } && items.contains { $0.fix == .createAggregate } && items.last?.status == .info, "dual: B internal and no aggregate -> two fixes; one bus -> info")
items = DualSetup.check(ua, DualUnitState(guid: "0xB", rate: 48000, sync: .wordClock, locked: true, bus: nil), uidA: "uidA", uidB: "uidB",
                        aggregate: DualAggregateState(exists: true, subUIDs: ["uidB", "uidA"], masterUID: "uidB", drift: [:], inputs: 32, outputs: 24))
check(items.contains { $0.fix == .recreateAggregate } && items.count == 4, "dual: wrong order / master -> recreate; unknown bus -> no line")
check(!DualSetup.available(ua, DualUnitState(guid: "0xB", rate: 44100, sync: .wordClock, locked: true, bus: nil)) && !DualSetup.available(DualUnitState(guid: "0xA", rate: 96000, sync: .internalMuted, locked: true, bus: nil), DualUnitState(guid: "0xB", rate: 96000, sync: .wordClock, locked: true, bus: nil)), "dual: same gated rate only")
let desc = DualSetup.description(uidA: "uidA", uidB: "uidB")
check(desc["master"] as? String == "uidA" && (desc["subdevices"] as? [[String: Any]])?.map { $0["uid"] as? String ?? "" } == ["uidA", "uidB"] && desc["private"] as? Int == 0, "dual: aggregate description")
check(DualSetup.offset(unitB: true, countA: 16) == 16 && DualSetup.offset(unitB: false, countA: 16) == 0, "dual: B after A")

// A write the driver refuses is kept (keys + code) until cleared: the Advanced windows show it instead of "applied"
let refusedCode = kern_return_t(bitPattern: 0xe00002c2)
final class Refusing: PropertyTransport {
    var refuse = true
    func copyProperties() -> [String: Any] {
        let zeros: [NSNumber] = [0, 0, 0, 0].map { NSNumber(value: $0) }
        return ["MAOutputMute": zeros]
    }
    func setProperties(_ p: [String: Any]) -> kern_return_t {
        if refuse { return refusedCode }
        return KERN_SUCCESS
    }
}
let refusing = Refusing()
let wm = DeviceModel(transport: refusing)
wm.setElement(MAKeys.outputMute, 1, 1); wm.commit()
let wf = wm.writeFailure
check(wf != nil && wf!.keys == ["MAOutputMute"] && wf!.code == refusedCode, "refused write recorded")
refusing.refuse = false
wm.setElement(MAKeys.outputMute, 2, 1); wm.commit()
check(wm.writeFailure != nil, "a later success does not hide the refused write")
wm.clearWriteFailure()
check(wm.writeFailure == nil, "cleared on the next scene apply")

print(failures == 0 ? "ALL PASS" : "\(failures) FAILURES")
exit(failures == 0 ? 0 : 1)
