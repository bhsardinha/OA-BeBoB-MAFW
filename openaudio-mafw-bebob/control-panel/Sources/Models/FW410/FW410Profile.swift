// FireWire 410 profile (module Models/FW410): the official panel's CX410Device counts and CFW410* views, drawn in our panel's layout (user,
// 2026-10-26: each interface has its own panel; the official screens are the checklist of controls). Sources: A2 PN410-control-key-map §1-§5
// (counts, keys, measured control map), capture 103 (screens, 263 decoded writes), docs/fw410-manual-reference.md (usage, defaults), kext
// models/FW410 (channel tables, meter page [inputs | SW returns | outputs]).
import AppKit

final class FW410Profile: DeviceProfile {
    static let shared = FW410Profile()

    let productName = "FireWire 410"
    let windowTitle = "M-Audio FireWire 410"
    let aboutPhoto = "fw410-front"                         // Resources/fw410-front.png: cut-out of docs/fw410 front panel.webp (user-supplied, 2026-10-27), 118 px high like the others
    let aboutTitle = "FireWire 410"
    let aboutCaption = "M-Audio FireWire 410"
    let counts = TranslatorCounts(outputs: 14, mixer: 14, inputs: 4, headphone: 2)        // CX410Device (decomp, PN410 §1)
    /// kSupportedInputRatesArray @0x2fa10 (kext FW410Tables.h): 44.1 ... 192 kHz, no 32 kHz
    let defaultRates: [UInt32] = [44100, 48000, 88200, 96000, 176400, 192000]
    /// FirmwareCatalog FW 410 = FW410.bcd build 20070504102656; the user's unit runs it (capture 103 about tab: 2007-05-04 10:26:56)
    let minimumFirmware = "20070504102656"
    /// MASWReturnRoutingV2 / MAInputRoutingV2 bits 0-4 = out 1/2, 3/4, 5/6, 7/8, S/PDIF; several at once (capture 103, manual p.19)
    let routeLabels = ["1/2", "3/4", "5/6", "7/8", "spd"]
    /// MAHeadphoneSourceV2 bits 0-4 = output buses 1/2, 3/4, 5/6, 7/8, S/PDIF ("any or all", manual p.21; capture 103). Bit 5: the sixth source
    /// the v1.11.2 kext accepts (SetHeadphoneSource @0x144bc, word bit 26) and the official panel never offers = the AUX BUS (capture 106), so
    /// the phones can carry the aux mix together with output buses (plan FW410 v2 §6: all hidden features). The reference never clears
    /// word bit 26 once set; the clone does (deviation R138, user) -- see the kext's FW410MixerRegs.h.
    let phonesSources = [PhonesSource(mask: 1, button: "1/2", name: "Out 1/2", short: "out 1/2"),
                         PhonesSource(mask: 2, button: "3/4", name: "Out 3/4", short: "out 3/4"),
                         PhonesSource(mask: 4, button: "5/6", name: "Out 5/6", short: "out 5/6"),
                         PhonesSource(mask: 8, button: "7/8", name: "Out 7/8", short: "out 7/8"),
                         PhonesSource(mask: 16, button: "spd", name: "S/PDIF", short: "spdif"),
                         PhonesSource(mask: 32, button: "aux", name: "Aux bus", short: "aux")]
    var phonesMultiSelect: Bool { return true }
    var peakSWReturnsFirst: Bool { return true }
    var levelControllerTargets: [MARotaryMask.Group] { return [.swReturn, .output, .input, .headphone, .aux] }
    /// MAClockSource: internal 0x40000004 / external S/PDIF 0x40000005 (bit 2 = coaxial input, the S/PDIF card) (capture 103; kext asm 0x14eda)
    let hardware = HardwarePlan(sync: [SyncOption(title: "Internal", sync: .internalUnmuted, enabled: { _ in true }),
                                       SyncOption(title: "External - S/PDIF", sync: .externalDigital, enabled: { _ in true })],
                                digitalPorts: false, abSwitchTitle: "", abSheetName: "", abSwitch: false)

    func hardwareCard(_ ctx: PanelContext) -> HardwareCard? { return FW410SpdifCard(context: ctx) }

    /// 176.4/192 kHz: 2 in / 8 out (kext sDeviceChannels192; capture 105: all 8 analog outputs stream), no S/PDIF stream channels
    private func quad(_ l: ChannelLayout) -> Bool { return l.rateClass == .quad }

    // Mixer: SW returns 1/2, 3/4, 5/6, 7/8, S/PDIF; inputs analog 1/2, S/PDIF (manual p.19, capture 103 mixer.png)
    func mixer(_ l: ChannelLayout) -> MixerPlan {
        let p = peaks, q = quad(l)
        func o(_ id: Int) -> Int { return p.output(id) }
        let rets = ["Return 1/2", "Return 3/4", "Return 5/6", "Return 7/8", "Return S/PDIF"].enumerated().map {
            StripPlan(title: $0.element, pair: $0.offset + 1, enabled: !(q && $0.offset == 4)) }
        return MixerPlan(pageNote: nil, returns: rets, returnBus: nil,
                         inputs: [StripPlan(title: "Analog 1/2", pair: 1, enabled: true), StripPlan(title: "S/PDIF", pair: 2, enabled: !q)],
                         inputsNote: nil,
                         bus: [MeterGroupPlan(title: "Output Bus", labels: ["1/2", "3/4", "5/6", "7/8", "S/PDIF"],
                                              slots: (1...5).map { [o(2 * $0 - 1), o(2 * $0)] }, enabled: [true, true, true, true, !q]),
                               MeterGroupPlan(title: "Aux Bus", labels: ["L / R"], slots: [[p.auxOut(1), p.auxOut(2)]], enabled: [true])])
    }

    // Outputs: 1/2 ... 7/8 and S/PDIF (fader, bal, ctrl, solo, link/mute, main/aux = MAMonitorAuxOnOutput[1..5]); aux master with pan; phones
    // with ctrl, main/aux (MAMonitorAuxOnOutput[6], capture 103 "out-hp-main") and the multi-select sources (manual p.20-21).
    func output(_ l: ChannelLayout) -> OutputPlan {
        let p = peaks
        func out(_ title: String, _ pair: Int) -> OutputStripPlan {
            let (a, b) = DeviceModel.channels(ofPair: pair)
            return OutputStripPlan(title: title, levels: MAKeys.outputLevels, mute: MAKeys.outputMute, link: MAKeys.outputLink, solo: MAKeys.outputSolo,
                                   pairInGroup: pair, monitorPair: pair, rotary: (.output, MARotaryMask.outputPairBit(pair)),
                                   meterSlots: (p.output(a), p.output(b)), phones: nil, features: [.balance, .ctrl, .solo, .linkMute, .mainAux, .faders], hideLastMon: false, pan: nil)
        }
        let aux = OutputStripPlan(title: "Aux Out", levels: MAKeys.auxOutputLevels, mute: MAKeys.auxOutputMute, link: MAKeys.auxOutputLink, solo: nil,
                                  pairInGroup: 1, monitorPair: nil, rotary: (.aux, MARotaryMask.auxOutput), meterSlots: (p.auxOut(1), p.auxOut(2)),
                                  phones: nil, features: [.balance, .ctrl, .linkMute, .pan, .faders], hideLastMon: false, pan: MAKeys.auxOutputPan2)
        let phones = OutputStripPlan(title: "Phones", levels: MAKeys.headphoneLevels, mute: MAKeys.headphoneMute, link: MAKeys.headphoneLink, solo: nil,
                                     pairInGroup: 1, monitorPair: 6, rotary: (.headphone, MARotaryMask.headphone), meterSlots: (p.phones(1), p.phones(2)),
                                     phones: 1, features: [.balance, .ctrl, .linkMute, .mainAux, .mon, .faders], hideLastMon: false, pan: nil)
        let sw = p.swReturn
        let q = quad(l)
        return OutputPlan(mainTitle: "Main Outputs", main: [out("Out 1/2", 1), out("Out 3/4", 2), out("Out 5/6", 3), out("Out 7/8", 4), out("S/PDIF Out", 5)],
                          digitalBus: nil, otherTitle: "Aux & Phones", other: [aux, phones],
                          bus: [MeterGroupPlan(title: "Software Return Bus", labels: ["1/2", "3/4", "5/6", "7/8", "S/PDIF"],
                                               slots: (1...5).map { [sw(2 * $0 - 1), sw(2 * $0)] }, enabled: [true, true, true, true, !q]),
                                MeterGroupPlan(title: "Input Bus", labels: ["Analog 1/2", "S/PDIF"], slots: [[p.input(1), p.input(2)], [p.input(3), p.input(4)]], enabled: [true, !q])])
    }

    func meters(_ l: ChannelLayout, narrow: Bool) -> [(String, [MeterItem])] {
        let p = peaks
        func it(_ label: String, _ slots: [Int], _ group: String) -> MeterItem { return MeterItem(label: label, slots: slots, group: group, enabled: true) }
        func spdif(_ a: Int, _ b: Int) -> [MeterItem] { return [it("L", [a], "S/PDIF"), it("R", [b], "S/PDIF")] }
        let input = [it("1", [p.input(1)], "Analog"), it("2", [p.input(2)], "Analog")] + spdif(p.input(3), p.input(4))
        let sw = (1...8).map { it("\($0)", [p.swReturn($0)], "") } + spdif(p.swReturn(9), p.swReturn(10))
        let out = (1...8).map { it("\($0)", [p.output($0)], "Analog") } + spdif(p.output(9), p.output(10))
        return [("Input", input), ("Software Return", sw), ("Output", out)]
    }
}
