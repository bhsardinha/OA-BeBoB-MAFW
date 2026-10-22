// FW1814 profile (module Models/FW1814): the official panel's CX1814Device counts and CFW1814DeviceSettings views, as the 0.1.0 panel drew them.
// Every title, feature set and meter slot here was moved unchanged from the 0.1.0 tab code (AP-5.1: the 1814 snapshots must stay identical).
// Sources: A2-P13 (translator, slots, routing, headphone masks), P12 (screens), PN2 (control map), P14 (clone panel rules).
import AppKit

final class FW1814Profile: DeviceProfile {
    static let shared = FW1814Profile()

    let productName = "FireWire 1814"
    let windowTitle = "M-Audio FireWire 1814"
    let aboutPhoto = "fw1814-front"
    let aboutTitle = "FireWire 1814"
    let aboutCaption = "M-Audio FireWire 1814"                           // the About banner adds the edition (PanelEdition.caption)
    let counts = TranslatorCounts(outputs: 20, mixer: 32, inputs: 18, headphone: 4)        // CX1814Device vtable, A2-P13 §1
    /// The six rates the kext offers (kext models/FW1814/FW1814Tables.h kFW1814Rates, ref @0x2f890)
    let defaultRates: [UInt32] = [44100, 48000, 88200, 96000, 176400, 192000]
    /// _extracted/firmware/fw1814.bcd header build date (offset 0x08) = 20070713080440; the bench FW1814 runs exactly this build (MAFirmwareInfo)
    let minimumFirmware = "20070713080440"
    let routeLabels = ["1/2", "3/4"]                                                          // cookie 0x5f1: bit 0 out 1/2, bit 1 out 3/4
    /// translate(v) = v - (v == 4), mask = 1 << (translate - 1): panel 1/2/4 -> masks 1 / 2 / 4 (CX1814Device @0x72e8e, A2-P13 §5)
    let phonesSources = [PhonesSource(mask: 1, button: "1/2", name: "Out 1/2", short: "out 1/2"),
                         PhonesSource(mask: 2, button: "3/4", name: "Out 3/4", short: "out 3/4"),
                         PhonesSource(mask: 4, button: "aux", name: "Aux", short: "aux")]
    let hardware = HardwarePlan(sync: [SyncOption(title: "Internal - digital inputs muted", sync: .internalMuted, enabled: { _ in true }),
                                       SyncOption(title: "Internal - digital inputs unmuted", sync: .internalUnmuted, enabled: { _ in true }),
                                       SyncOption(title: "External - digital", sync: .externalDigital, enabled: { $0.externalDigitalSyncEnabled }),
                                       SyncOption(title: "Word clock", sync: .wordClock, enabled: { _ in true })],
                                digitalPorts: true, abSwitchTitle: "Headphone out 1 A/B", abSheetName: "Headphone Out 1")

    func hardwareCard(_ ctx: PanelContext) -> HardwareCard? { return FW1814PortsCard(context: ctx) }

    // MARK: mixer tab

    func mixer(_ l: ChannelLayout) -> MixerPlan {
        let p = peaks
        // 176.4 / 192 kHz: the DSP mixer is dead (user, hardware, 2026-10-17): no inputs, returns, aux sends, routing or input meters to show.
        if l.rateClass == .quad {
            return MixerPlan(pageNote: "At 176.4 and 192 kHz the interface's internal mixer is off: audio goes straight through Core Audio, so there are no inputs, software returns, aux sends or routing here, and no input metering. Output levels and the phones are on the Output tab.",
                             returns: [], returnBus: nil, inputs: [], inputsNote: nil, bus: [])
        }
        var bus: BusPlan?
        switch l.returnBus {
        // Digital return bus meters: S/PDIF = SW-return slots 5, 6; ADAT = slots 7...14 (was 5... = 2 strips early, found on hardware 2026-10-17).
        case .adat: bus = BusPlan(title: "ADAT Bus", numbers: nil, slots: (1...8).map { p.swReturn(6 + $0) }, greyedFrom: l.adatBusHalfGreyed ? 4 : 8, mainAuxPair: nil)
        case .spdif: bus = BusPlan(title: "S/PDIF Bus", numbers: ["L", "R"], slots: [p.swReturn(5), p.swReturn(6)], greyedFrom: 2, mainAuxPair: nil)
        case .none: bus = nil
        }
        return MixerPlan(pageNote: nil,
                         returns: [StripPlan(title: "Return 1/2", pair: 1, enabled: true), StripPlan(title: "Return 3/4", pair: 2, enabled: true)],
                         returnBus: bus,
                         inputs: l.inputStrips.map { StripPlan(title: fw1814InputTitle($0.title), pair: $0.pair, enabled: $0.enabled) },
                         inputsNote: l.digitalAvailable ? nil : "Inputs 3-8 and the digital ports are not available at 176.4 and 192 kHz: the interface runs\u{00A0}2\u{00A0}in\u{00A0}/\u{00A0}4\u{00A0}out.",
                         bus: [MeterGroupPlan(title: "Output Bus", labels: ["1/2", "3/4"], slots: [[p.output(1), p.output(2)], [p.output(3), p.output(4)]], enabled: [true, true]),
                               MeterGroupPlan(title: "Aux Bus", labels: ["L / R"], slots: [[p.auxOut(1), p.auxOut(2)]], enabled: [true])])
    }

    // MARK: output tab

    func output(_ l: ChannelLayout) -> OutputPlan {
        let p = peaks
        // 176.4 / 192 kHz: the DSP mixer is dead (user, hardware, 2026-10-17; the manual says so too): no main/aux choice, no aux out, no bus meters;
        // the hardware output peaks freeze with the mixer, so the main strips show the software peaks of what Core Audio sends (SW-return slots 1-4)
        // and the phones have no meter.
        let quad = l.rateClass == .quad
        let mainF: StripFeatures = quad ? [.balance, .ctrl, .solo, .linkMute, .faders] : [.balance, .ctrl, .solo, .linkMute, .mainAux, .faders]
        func out(_ title: String, _ pair: Int) -> OutputStripPlan {
            let (a, b) = DeviceModel.channels(ofPair: pair)
            return OutputStripPlan(title: title, levels: MAKeys.outputLevels, mute: MAKeys.outputMute, link: MAKeys.outputLink, solo: MAKeys.outputSolo,
                                   pairInGroup: pair, monitorPair: pair, rotary: (.output, MARotaryMask.outputPairBit(pair)),
                                   meterSlots: quad ? (p.swReturn(a), p.swReturn(b)) : (p.output(a), p.output(b)), phones: nil, features: mainF, hideLastMon: false, pan: nil)
        }
        let hpF: StripFeatures = [.balance, .linkMute, .mon, .faders]
        func hp(_ n: Int) -> OutputStripPlan {
            let (a, b) = DeviceModel.channels(ofPair: n)
            return OutputStripPlan(title: "Phones \(n)", levels: MAKeys.headphoneLevels, mute: MAKeys.headphoneMute, link: MAKeys.headphoneLink, solo: nil,
                                   pairInGroup: n, monitorPair: nil, rotary: nil, meterSlots: quad ? (-1, -1) : (p.phones(a), p.phones(b)),
                                   phones: n, features: hpF, hideLastMon: quad, pan: nil)
        }
        let aux = OutputStripPlan(title: "Aux Out", levels: MAKeys.auxOutputLevels, mute: MAKeys.auxOutputMute, link: MAKeys.auxOutputLink, solo: nil,
                                  pairInGroup: 1, monitorPair: nil, rotary: (.aux, MARotaryMask.auxOutput), meterSlots: (p.auxOut(1), p.auxOut(2)),
                                  phones: nil, features: [.balance, .ctrl, .linkMute, .faders], hideLastMon: false, pan: nil)   // official 1814 aux strip: bal only (P12 screens)
        var digital: BusPlan?
        switch l.returnBus {
        // S/PDIF out = output channels 5, 6; ADAT = channels 7...14 (order consistent with the kext peak maps, A2-P13 §1).
        case .adat: digital = BusPlan(title: "ADAT Out", numbers: nil, slots: (7...14).map { p.output($0) }, greyedFrom: l.adatBusHalfGreyed ? 4 : 8, mainAuxPair: nil)
        case .spdif: digital = BusPlan(title: "S/PDIF Out", numbers: ["L", "R"], slots: [p.output(5), p.output(6)], greyedFrom: 2, mainAuxPair: ChannelLayout.spdifOutputPair)
        case .none: digital = nil
        }
        var bus: [MeterGroupPlan]?
        if !quad {
            // SW return bus 1/2, 3/4; input bus: analog pairs 1...4, then the S/PDIF pair (5) or ADAT pairs (6...9), as in the strip list.
            let labels = l.inputBusMeters
            bus = [MeterGroupPlan(title: "Software Return Bus", labels: ["1/2", "3/4"], slots: [[p.swReturn(1), p.swReturn(2)], [p.swReturn(3), p.swReturn(4)]], enabled: [true, true]),
                   MeterGroupPlan(title: "Input Bus", labels: labels.map { fw1814BusLabel($0.title) },
                                  slots: l.inputStrips.map { s -> [Int] in let (a, b) = DeviceModel.channels(ofPair: s.pair); return [p.input(a), p.input(b)] },
                                  enabled: labels.map { $0.enabled })]
        }
        return OutputPlan(mainTitle: "Main Outputs", main: [out("Out 1/2", 1), out("Out 3/4", 2)], digitalBus: digital,
                          otherTitle: quad ? "Phones" : "Aux & Phones", other: quad ? [hp(1), hp(2)] : [aux, hp(1), hp(2)], bus: bus)
    }

    // MARK: meter windows

    func meters(_ l: ChannelLayout, narrow: Bool) -> [(String, [MeterItem])] {
        let p = peaks
        // ADAT 5-8 are greyed at 88.2/96 kHz, as on the strips and bus meters (input follows optical in, the others optical out).
        let inHalf = l.inputBusMeters.contains { !$0.enabled }, outHalf = l.adatBusHalfGreyed
        func item(_ label: String, _ slots: [Int], _ group: String, panel: Int) -> MeterItem {
            let grey = group == "ADAT" && (Int(label) ?? 0) > 4 && (panel == 0 ? inHalf : outHalf)
            return MeterItem(label: label, slots: slots, group: group, enabled: !grey)
        }
        func numbered(_ n: Int, _ group: String, panel: Int, _ slot: (Int) -> Int) -> [MeterItem] { return (1...n).map { item("\($0)", [slot($0)], group, panel: panel) } }
        // 176.4 / 192 kHz: no DSP mixer: the only live meters are the software peaks of the four outputs Core Audio sends (SW-return slots 1...4).
        if l.rateClass == .quad { return [("Output", numbered(4, "Analog", panel: 0, p.swReturn))] }
        var input = numbered(8, "Analog", panel: 0, p.input)
        var sw = numbered(4, "", panel: 1, p.swReturn)
        var out = numbered(4, "Analog", panel: 2, p.output)
        if l.digitalAvailable {
            if l.adatIn { input += (1...8).map { item("\($0)", [p.input(10 + $0)], "ADAT", panel: 0) } }
            // S/PDIF as two meters (L, R) in both views (user, 2026-10-20; 0.1.0 merged them into one "L+R" bar in the wide view)
            else { input += [item("L", [p.input(9)], "S/PDIF", panel: 0), item("R", [p.input(10)], "S/PDIF", panel: 0)] }
            if l.adatOut {
                sw += (1...8).map { item("\($0)", [p.swReturn(6 + $0)], "ADAT", panel: 1) }
                out += (1...8).map { item("\($0)", [p.output(6 + $0)], "ADAT", panel: 2) }
            } else {
                sw += [item("L", [p.swReturn(5)], "S/PDIF", panel: 1), item("R", [p.swReturn(6)], "S/PDIF", panel: 1)]
                out += [item("L", [p.output(5)], "S/PDIF", panel: 2), item("R", [p.output(6)], "S/PDIF", panel: 2)]
            }
        } else {
            input = numbered(2, "Analog", panel: 0, p.input)       // 192 kHz: 2 in / 4 out analog
        }
        return [("Input", input), ("Software Return", sw), ("Output", out)]
    }
}

/// Strip title for a layout input title ("analog 1/2 in" -> "Analog 1/2", "adat 5/6 in" -> "ADAT 5/6", "spdif in" -> "S/PDIF").
func fw1814InputTitle(_ s: String) -> String {
    var t = s.hasSuffix(" in") ? String(s.dropLast(3)) : s
    t = t.replacingOccurrences(of: "analog", with: "Analog").replacingOccurrences(of: "adat", with: "ADAT").replacingOccurrences(of: "spdif", with: "S/PDIF")
    return t
}

/// Input-bus meter label for a layout title ("1/2" -> "Analog 1/2", "adt 5/6" -> "ADAT 5/6", "spdif" -> "S/PDIF").
func fw1814BusLabel(_ s: String) -> String {
    if s.hasPrefix("adt ") { return "ADAT " + String(s.dropFirst(4)) }
    if s == "spdif" { return "S/PDIF" }
    return "Analog " + s
}
