// FireWire Audiophile profile (module Models/FWAudiophile): the official panel's CXAudiophileDevice counts and CFWDJ* views, drawn in our panel's
// layout (user decision 2026-10-19: the official screens are only the checklist of controls). Sources: A2-PNAP §1-§7 (counts, id spaces,
// cookies -> keys, measured control map), capture 91 (screens), capture 92 (factory state), kext models/FWAudiophile (channel table).
import AppKit

final class AudiophileProfile: DeviceProfile {
    static let shared = AudiophileProfile()

    let productName = "FireWire Audiophile"
    let windowTitle = "M-Audio FireWire Audiophile"
    let aboutPhoto = "fwap-front"                          // Resources/fwap-front.png: cut-out of docs/audiophile front panel.webp (user-supplied), 118 px high like the FW1814
    let aboutTitle = "FireWire Audiophile"
    let aboutCaption = "M-Audio FireWire Audiophile"                     // the About banner adds the edition (PanelEdition.caption)
    let counts = TranslatorCounts(outputs: 10, mixer: 10, inputs: 4, headphone: 2)        // CXAudiophileDevice vtable, PNAP §1 (R84)
    /// kSupportedSampleRatesArray @0x2f720 (kext FWAudiophileTables.h): no 176.4 / 192 kHz on this model
    let defaultRates: [UInt32] = [44100, 48000, 88200, 96000]
    /// _extracted/firmware/FWAP.bcd header build date (offset 0x08) = 20070504102212 (= ctl-services normal.rs:120-135); the bench Audiophile
    /// runs exactly this build (capture 89 MAFirmwareInfo)
    let minimumFirmware = "20070504102212"
    /// cookie 0x5f1 sends three destination flags: bit 0 mixer 1/2 (out 1/2), bit 1 mixer 3/4, bit 2 mixer 5/6 (S/PDIF) (PNAP §3, §7)
    let routeLabels = ["1/2", "3/4", "spd"]
    /// MAHeadphoneSourceV2 masks 1 / 2 / 4 / 8 = mixer 1/2, 3/4, 5/6 (S/PDIF), aux (PNAP §7; kext: lowest bit -> selector FB 4 input 0-3)
    let phonesSources = [PhonesSource(mask: 1, button: "1/2", name: "Out 1/2", short: "out 1/2"),
                         PhonesSource(mask: 2, button: "3/4", name: "Out 3/4", short: "out 3/4"),
                         PhonesSource(mask: 4, button: "spd", name: "S/PDIF", short: "spdif"),
                         PhonesSource(mask: 8, button: "aux", name: "Aux", short: "aux")]
    /// MAClockSource 0x40000000 internal / 0x40000001 external S/PDIF (PNAP §7; kext SetClockSourceInternal @0xb9d4 accepts only these)
    let hardware = HardwarePlan(sync: [SyncOption(title: "Internal", sync: .internalUnmuted, enabled: { _ in true }),
                                       SyncOption(title: "External - S/PDIF", sync: .externalDigital, enabled: { _ in true })],
                                digitalPorts: false, abSwitchTitle: "Headphone A/B", abSheetName: "Headphone", abSwitch: true)

    func hardwareCard(_ ctx: PanelContext) -> HardwareCard? { return nil }

    // Mixer pairs: inputs 1 = analog, 2 = S/PDIF; SW returns 1/2, 3/4, 5/6 = S/PDIF (PNAP §2).
    func mixer(_ l: ChannelLayout) -> MixerPlan {
        let p = peaks
        func o(_ id: Int) -> Int { return p.output(id) }
        return MixerPlan(pageNote: nil,
                         returns: [StripPlan(title: "Return 1/2", pair: 1, enabled: true), StripPlan(title: "Return 3/4", pair: 2, enabled: true),
                                   StripPlan(title: "Return S/PDIF", pair: 3, enabled: true)],
                         returnBus: nil,
                         inputs: [StripPlan(title: "Analog 1/2", pair: 1, enabled: true), StripPlan(title: "S/PDIF", pair: 2, enabled: true)],
                         inputsNote: nil,
                         bus: [MeterGroupPlan(title: "Output Bus", labels: ["1/2", "3/4", "S/PDIF"], slots: [[o(1), o(2)], [o(3), o(4)], [o(5), o(6)]], enabled: [true, true, true]),
                               MeterGroupPlan(title: "Aux Bus", labels: ["L / R"], slots: [[p.auxOut(1), p.auxOut(2)]], enabled: [true])])
    }

    // Outputs: pairs 1/2, 3/4 analog and 5/6 S/PDIF (each with fader, solo, main/aux: MAMonitorAuxOnOutput[3]), aux master, one phones pair.
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
                                  phones: nil, features: [.balance, .ctrl, .linkMute, .pan, .faders], hideLastMon: false, pan: MAKeys.auxOutputPan2)   // official Audiophile aux strip: bal on top, pan L/R below (capture 91 02-output.png; cookie 0x5f5):
                                  // one pan per channel so the aux bus can be used as two mono channels (user, 2026-10-20)
        let phones = OutputStripPlan(title: "Phones", levels: MAKeys.headphoneLevels, mute: MAKeys.headphoneMute, link: MAKeys.headphoneLink, solo: nil,
                                     pairInGroup: 1, monitorPair: nil, rotary: nil, meterSlots: (p.phones(1), p.phones(2)),
                                     phones: 1, features: [.balance, .linkMute, .mon, .faders], hideLastMon: false, pan: nil)
        let sw = p.swReturn
        return OutputPlan(mainTitle: "Main Outputs", main: [out("Out 1/2", 1), out("Out 3/4", 2), out("S/PDIF Out", 3)], digitalBus: nil,
                          otherTitle: "Aux & Phones", other: [aux, phones],
                          bus: [MeterGroupPlan(title: "Software Return Bus", labels: ["1/2", "3/4", "S/PDIF"], slots: [[sw(1), sw(2)], [sw(3), sw(4)], [sw(5), sw(6)]], enabled: [true, true, true]),
                                MeterGroupPlan(title: "Input Bus", labels: ["Analog 1/2", "S/PDIF"], slots: [[p.input(1), p.input(2)], [p.input(3), p.input(4)]], enabled: [true, true])])
    }

    func meters(_ l: ChannelLayout, narrow: Bool) -> [(String, [MeterItem])] {
        let p = peaks
        func it(_ label: String, _ slots: [Int], _ group: String) -> MeterItem { return MeterItem(label: label, slots: slots, group: group, enabled: true) }
        func spdif(_ a: Int, _ b: Int) -> [MeterItem] { return [it("L", [a], "S/PDIF"), it("R", [b], "S/PDIF")] }   // L and R in both views (user, 2026-10-20)
        let input = [it("1", [p.input(1)], "Analog"), it("2", [p.input(2)], "Analog")] + spdif(p.input(3), p.input(4))
        let sw = (1...4).map { it("\($0)", [p.swReturn($0)], "") } + spdif(p.swReturn(5), p.swReturn(6))
        let out = (1...4).map { it("\($0)", [p.output($0)], "Analog") } + spdif(p.output(5), p.output(6))
        return [("Input", input), ("Software Return", sw), ("Output", out)]
    }
}
