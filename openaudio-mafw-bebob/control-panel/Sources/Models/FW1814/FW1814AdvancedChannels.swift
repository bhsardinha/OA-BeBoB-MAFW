// FW1814 channels of the Advanced window's meter views (PANEL_REDESIGN_PLAN_v3, DAW and Console modes): each Core Audio channel with its jack
// and peak slot. Core Audio order from the kext tables (FW1814Tables.h: ADAT 16 in / 12 out = analog 1-8 then ADAT 1-8 in, Line 1-4 then
// ADAT 1-8 out; S/PDIF 10 / 6 = S/PDIF L/R after the analog channels). Peak slots as FW1814Profile.meters (input 11-18 = ADAT in, output 5-6
// = S/PDIF out, 7-14 = ADAT out). Pure logic, unit-tested.
import Foundation

func fw1814AdvancedChannels(_ l: ChannelLayout, peaks p: PeakMap) -> AdvancedChannels {
    var ins = (1...8).map { n in AdvancedChannel(id: "in.a\(n)", name: "In \(n)", jack: n <= 2 ? "Mic/Line \(n)" : "Line \(n)", ca: n, slot: p.input(n), group: "Analog") }
    if l.adatIn {
        ins += (1...8).map { n in AdvancedChannel(id: "in.adat\(n)", name: "ADAT \(n)", jack: "ADAT \(n)", ca: 8 + n, slot: p.input(10 + n), group: "ADAT") }
    } else {
        ins += [AdvancedChannel(id: "in.spdifL", name: "S/PDIF L", jack: "S/PDIF L", ca: 9, slot: p.input(9), group: "S/PDIF"),
                AdvancedChannel(id: "in.spdifR", name: "S/PDIF R", jack: "S/PDIF R", ca: 10, slot: p.input(10), group: "S/PDIF")]
    }
    var outs = (1...4).map { n in AdvancedChannel(id: "out.l\(n)", name: "Line \(n)", jack: "Line out \(n)", ca: n, slot: p.output(n), group: "Line") }
    if l.adatOut {
        outs += (1...8).map { n in AdvancedChannel(id: "out.adat\(n)", name: "ADAT \(n)", jack: "ADAT out \(n)", ca: 4 + n, slot: p.output(6 + n), group: "ADAT out") }
    } else {
        outs += [AdvancedChannel(id: "out.spdifL", name: "S/PDIF L", jack: "S/PDIF out L", ca: 5, slot: p.output(5), group: "S/PDIF out"),
                 AdvancedChannel(id: "out.spdifR", name: "S/PDIF R", jack: "S/PDIF out R", ca: 6, slot: p.output(6), group: "S/PDIF out")]
    }
    let phones = (1...2).map { n in AdvancedPhones(number: n, slots: (p.phones(2 * n - 1), p.phones(2 * n))) }
    return AdvancedChannels(inputs: ins, outputs: outs, phones: phones)
}
