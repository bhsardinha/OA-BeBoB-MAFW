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
check(m.element(MAKeys.outputLevels, 1) == MARange.levelMin, "level clamped to panel minimum")
m.commit()
check(mock.writes.count == 1 && (mock.writes[0]["MAOutputLevels"] as! [NSNumber]).map { $0.intValue } == [-129, 0, -10, -10], "one batched write")
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
m.setABSources(a: .aux, b: .out34)
check((mock.props["MAMomentarySwitchRoutings"] as! [NSNumber]).map { $0.intValue } == [4, 2, 3, 4], "sheet entries (0 = A, 1 = B)")
// Peak math (A2-P9 §7.5): 0 -> -64, 32768 -> 0 dB, 16384 -> -6 dB (ceil).
check(peakToDB(0) == -64 && peakToDB(32768) == 0 && peakToDB(16384) == -6 && peakToDB(1) == -64, "peak dB (below -64 clamps)")
check(PeakSlot.input(1) == 0 && PeakSlot.output(1) == 18 && PeakSlot.swReturn(14) == 51, "peak slots")

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

// Supported rates: CoreAudio's list when the HAL device exists, the kext's six rates otherwise (A2-P8 §10).
final class MockRate: RateBackend {
    var nominalRate: UInt32? = 48000
    var availableRates: [UInt32] = []
    var onRateChanged: (() -> Void)?
    func setNominalRate(_ hz: UInt32) -> Bool { nominalRate = hz; return true }
}
let mr = MockRate()
let rm = DeviceModel(transport: Mock(), rate: mr)
check(rm.supportedRates == DeviceModel.defaultRates, "no HAL list yet: the six default rates")
mr.availableRates = [44100, 48000, 88200, 96000]
check(rm.supportedRates == [44100, 48000, 88200, 96000], "the panel follows CoreAudio's list")
check(DeviceModel(transport: Mock()).supportedRates == DeviceModel.defaultRates, "demo backend: the six default rates")

// Meter slots: ADAT software-return channel n sits at slot 6 + n (after 4 analog + 2 S/PDIF), like the 18 inputs and the outputs.
check(PeakSlot.swReturn(5) == 42 && PeakSlot.swReturnADAT(1) == PeakSlot.swReturn(7) && PeakSlot.swReturnADAT(8) == PeakSlot.swReturn(14) && PeakSlot.swReturnADAT(8) == 51, "ADAT sw-return peak slots 7...14")

print(failures == 0 ? "ALL PASS" : "\(failures) FAILURES")
exit(failures == 0 ? 0 : 1)
