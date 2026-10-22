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

print(failures == 0 ? "ALL PASS" : "\(failures) FAILURES")
exit(failures == 0 ? 0 : 1)
