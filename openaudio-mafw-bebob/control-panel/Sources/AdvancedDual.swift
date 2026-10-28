// Advanced window, Dual (PANEL_REDESIGN_PLAN_v3 §4, phase M7; user 2026-10-27): two FW1814s used as one macOS aggregate device (32 in / 24 out
// at 44.1 / 48 kHz with ADAT), DAW and Console modes only, always on the aggregate. The driver exposes each unit as its own Core Audio device,
// as the official driver does (capture 94, R93); MAEnableAggregateEngine answers "unsupported" in the reference v1.11.2 (Q-MAMAP row 37) and
// in the clone (FWAudioDevice.h), so "Dual" is an aggregate made with AudioHardwareCreateAggregateDevice (10.13 SDK AudioHardware.h, 10.9+).
// Unit B follows unit A's word clock (the FW1814 has word clock in and out, manual §4). Pure logic (Foundation), unit-tested; the Core Audio
// calls are in DualAggregate.swift.
import Foundation

/// What the setup checklist needs to know about one unit.
struct DualUnitState {
    let guid: String
    let rate: UInt32
    let sync: MAClockSource.Sync
    let locked: Bool
    let bus: UInt64?            // registry id of the FireWire controller above the unit (nil = unknown)
}

/// The aggregate as Core Audio has it (nil sub-devices = not found).
struct DualAggregateState {
    var exists: Bool
    var subUIDs: [String]
    var masterUID: String?
    var drift: [String: Bool]   // sub-device UID -> drift compensation on
    var inputs: Int
    var outputs: Int
    static let missing = DualAggregateState(exists: false, subUIDs: [], masterUID: nil, drift: [:], inputs: 0, outputs: 0)
}

enum DualSetup {
    static let aggregateUID = "com.openaudio.bebobmafw.dual1814"
    static let aggregateName = "Dual FireWire 1814"
    /// one FW1814 in ADAT mode at 48 kHz (capture 94)
    static let bytesPerCycleADAT = 976

    enum Fix: Equatable { case wordClockB, createAggregate, recreateAggregate }
    enum Status: Equatable { case ok, warn, fail, info }
    struct Item: Equatable {
        let status: Status
        let text: String
        let fix: Fix?
    }

    /// The Dual window works when both units run at the same rate and that rate is 44.1 or 48 kHz.
    static func available(_ a: DualUnitState, _ b: DualUnitState) -> Bool {
        return a.rate == b.rate && AdvancedGate.rates.contains(a.rate)
    }

    /// The aggregate's description for AudioHardwareCreateAggregateDevice: A then B, A the master (clock), no drift compensation on B (it is
    /// locked to A's word clock), visible to every application (not private).
    static func description(uidA: String, uidB: String) -> [String: Any] {
        return ["uid": aggregateUID, "name": aggregateName,
                "subdevices": [["uid": uidA, "drift": 0], ["uid": uidB, "drift": 0]],
                "master": uidA, "private": 0]
    }

    /// The checklist of the Dual setup sheet, in order: rate, A's clock, B's clock, the aggregate, the buses.
    static func check(_ a: DualUnitState, _ b: DualUnitState, uidA: String?, uidB: String?, aggregate g: DualAggregateState) -> [Item] {
        func kHz(_ r: UInt32) -> String { return r % 1000 == 0 ? "\(r / 1000)" : String(format: "%.1f", Double(r) / 1000) }
        var out: [Item] = []
        if available(a, b) { out.append(Item(status: .ok, text: "Both units at \(kHz(a.rate)) kHz", fix: nil)) }
        else { out.append(Item(status: .fail, text: "Rates: A \(kHz(a.rate)) kHz, B \(kHz(b.rate)) kHz \u{2014} both must run at 44.1 or 48 kHz, the same (set it in Audio MIDI Setup or the standard panel)", fix: nil)) }
        let aInternal = a.sync == .internalMuted || a.sync == .internalUnmuted
        out.append(aInternal ? Item(status: .ok, text: "Unit A runs on its internal clock (the master)", fix: nil)
                             : Item(status: .warn, text: "Unit A follows an external clock: B must follow the same source", fix: nil))
        if b.sync == .wordClock && b.locked { out.append(Item(status: .ok, text: "Unit B follows word clock and is locked (BNC from A\u{2019}s word clock out)", fix: nil)) }
        else if b.sync == .wordClock { out.append(Item(status: .fail, text: "Unit B is set to word clock but not locked: check the BNC cable from A\u{2019}s word clock out", fix: nil)) }
        else { out.append(Item(status: .fail, text: "Unit B is not on word clock: connect A\u{2019}s word clock out to B\u{2019}s word clock in, then set B to Word clock", fix: .wordClockB)) }
        if let ua = uidA, let ub = uidB {
            if !g.exists { out.append(Item(status: .fail, text: "No \u{201C}\(aggregateName)\u{201D} aggregate in Core Audio", fix: .createAggregate)) }
            else if g.subUIDs != [ua, ub] || g.masterUID != ua || g.drift[ub] == true {
                out.append(Item(status: .fail, text: "The \u{201C}\(aggregateName)\u{201D} aggregate is not A then B with A as master and no drift correction on B", fix: .recreateAggregate))
            } else { out.append(Item(status: .ok, text: "Aggregate \u{201C}\(aggregateName)\u{201D}: \(g.inputs) in / \(g.outputs) out, clock from A, no drift correction on B", fix: nil)) }
        } else {
            out.append(Item(status: .fail, text: "Core Audio does not list both units yet (they appear a few seconds after the driver)", fix: nil))
        }
        if let x = a.bus, let y = b.bus {
            out.append(x != y ? Item(status: .ok, text: "Each unit on its own FireWire bus", fix: nil)
                              : Item(status: .info, text: "Both units on one FireWire bus (\(bytesPerCycleADAT) bytes per cycle each in ADAT mode)", fix: nil))
        }
        return out
    }
    static func ready(_ items: [Item]) -> Bool { return !items.contains { $0.status == .fail } }

    /// Core Audio channel numbers in the aggregate: unit B's channels follow all of unit A's (A's input / output count).
    static func offset(unitB: Bool, countA: Int) -> Int { return unitB ? countA : 0 }
}
