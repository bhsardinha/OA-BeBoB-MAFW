// Advanced window (docs/PANEL_REDESIGN_PLAN_v3.md, user 2026-10-27; a clone feature, the reference has nothing like it): the four modes,
// the gate (a model that offers it, at 44.1 / 48 kHz only), and the interface scene each mode keeps. Pure logic (Foundation only), unit-tested
// in Tests/LayoutTests.swift. A scene is the preset keys without mute and dim (those belong to the room, not to a way of working); applying
// one goes through DeviceModel.restore, the same batch write as loading a preset.
import Foundation

enum AdvancedMode: String {
    case daw, monitor, console, hybrid

    static let all: [AdvancedMode] = [.daw, .monitor, .console, .hybrid]
    /// Dual (two units as one aggregate) offers these only (user, 2026-10-27; plan v3 §4)
    static let dual: [AdvancedMode] = [.daw, .console]
    /// The mode a first opening lands in (user, 2026-10-27)
    static let first = AdvancedMode.daw

    var name: String {
        switch self { case .daw: return "DAW"; case .monitor: return "Monitor"; case .console: return "Console"; case .hybrid: return "Hybrid" }
    }
    var title: String {
        switch self {
        case .daw: return "Everything in the DAW"
        case .monitor: return "Direct monitoring"
        case .console: return "External mixer"
        case .hybrid: return "Outboard and reamp"
        }
    }
    var summary: String {
        switch self {
        case .daw: return "you monitor through the DAW; the panel is meters and a control room"
        case .monitor: return "cue mixes inside the interface, zero latency, mono first"
        case .console: return "the interface as a converter for an analog console"
        case .hybrid: return "DAW \u{2192} outboard \u{2192} DAW loops with send and return meters"
        }
    }
    /// What the interface's mixer does in this mode (the mode bar's chip)
    var interfaceState: String {
        switch self {
        case .daw: return "Interface mixer: passthrough \u{00B7} DAW 1\u{2013}4 \u{2192} Line 1\u{2013}4 at 0 dB \u{00B7} inputs not monitored"
        case .monitor: return "Interface mixer: cue mixes on Mix 1/2 \u{00B7} Mix 3/4 \u{00B7} Aux"
        case .console: return "Passthrough at unity \u{00B7} every interface level at 0 dB"
        case .hybrid: return "Passthrough \u{00B7} loop sends at unity \u{00B7} loop returns not monitored"
        }
    }
}

enum AdvancedGate {
    /// Only 44.1 and 48 kHz (user, 2026-10-27): there the FW1814 has its full mixer and 8 ADAT channels each way (kext FW1814Tables.h)
    static let rates: [UInt32] = [44100, 48000]
    /// The menu item exists for a model that offers the window (DeviceProfile.advancedPanel)
    static func offered(modelAllows: Bool) -> Bool { return modelAllows }
    /// ... and works at the gated rates; outside them the window shows why instead of the mode
    static func available(modelAllows: Bool, rate: UInt32) -> Bool { return modelAllows && rates.contains(rate) }
}

/// One Core Audio channel of a meter view: stable id (names are stored by it), default name, jack, Core Audio channel, peak slot, group.
struct AdvancedChannel: Equatable {
    let id: String
    let name: String
    let jack: String
    let ca: Int
    let slot: Int
    let group: String
}
/// A phones output (two peak slots, L and R).
struct AdvancedPhones: Equatable {
    let number: Int
    let slots: (Int, Int)
    static func == (a: AdvancedPhones, b: AdvancedPhones) -> Bool { return a.number == b.number && a.slots == b.slots }
}
struct AdvancedChannels {
    var inputs: [AdvancedChannel]
    var outputs: [AdvancedChannel]
    var phones: [AdvancedPhones]
    static let none = AdvancedChannels(inputs: [], outputs: [], phones: [])
}

/// Hybrid mode: one outboard loop, DAW -> send output(s) -> device -> return input(s) -> DAW (AdvancedChannel ids; 1 = mono, 2 = stereo).
struct HybridLoop: Equatable {
    var name: String
    var device: String
    var sends: [String]
    var returns: [String]
    var plist: [String: Any] { return ["Name": name, "Device": device, "Sends": sends, "Returns": returns] }
    init(name: String, device: String, sends: [String], returns: [String]) { self.name = name; self.device = device; self.sends = sends; self.returns = returns }
    init?(plist d: [String: Any]) {
        guard let n = d["Name"] as? String, let s = d["Sends"] as? [String], let r = d["Returns"] as? [String], !s.isEmpty, s.count == r.count else { return nil }
        self.init(name: n, device: d["Device"] as? String ?? "", sends: s, returns: r)
    }
    /// The level difference shown on the card: return peak minus send peak (dB), nil while either is silent
    static func difference(send: Int, ret: Int) -> Int? { return send <= -64 || ret <= -64 ? nil : ret - send }
    /// Outputs a loop may use as its send: not Line 1/2 (the monitor speakers), not already a send of another loop
    static func freeSends(_ outputs: [AdvancedChannel], loops: [HybridLoop]) -> [AdvancedChannel] {
        let used = Set(loops.flatMap { $0.sends })
        return outputs.filter { $0.id != "out.l1" && $0.id != "out.l2" && !used.contains($0.id) }
    }
    /// Output channels (1-based, MAOutputLevels) of the sends that go through the mixer (Line outs); ADAT / S/PDIF outs are direct
    static func lineChannels(_ sends: [String]) -> [Int] { return sends.compactMap { $0.hasPrefix("out.l") ? Int($0.dropFirst(5)) : nil } }
}

/// One output whose level would go up when a scene is applied (the switch asks first, plan v2 §3).
struct LevelRise: Equatable {
    let key: String          // MAOutputLevels / MAHeadphoneLevels
    let channel: Int         // 1-based
    let from: Int            // half-dB steps; MALevel.minimum when muted
    let to: Int
}

enum AdvancedScene {
    /// The keys a mode keeps: the preset keys without the global mute and dim
    static let keys: [MAKey] = DeviceModel.presetKeys.filter { $0.name != MAKeys.globalMute.name && $0.name != MAKeys.globalDim.name }

    /// The scene part of a full property dictionary (a DeviceModel cache or a preset snapshot)
    static func capture(_ props: [String: Any]) -> [String: Any] {
        var d: [String: Any] = [:]
        for k in keys { if let v = props[k.name] { d[k.name] = v } }
        return d
    }

    static func ints(_ s: [String: Any], _ k: MAKey) -> [Int] {
        guard let a = s[k.name] as? [Any] else { return [] }
        return a.map { DeviceModel.int($0) }
    }
    private static func put(_ s: inout [String: Any], _ k: MAKey, _ v: [Int]) {
        guard s[k.name] != nil else { return }                       // only keys the unit publishes
        s[k.name] = v.map { NSNumber(value: Int32(truncatingIfNeeded: $0)) }
    }
    private static func fill(_ s: inout [String: Any], _ k: MAKey, _ value: Int) { put(&s, k, ints(s, k).map { _ in value }) }

    /// DAW / Hybrid: the interface mixer as a passthrough. DAW returns at 0 dB to their own output pair (element i -> bit i, the kext
    /// default, Q-MAMAP row 25), no input monitored (routing 0, the factory default, manual §9), no solo, outputs on their main mix, phones 1
    /// on out 1/2 and phones 2 on out 3/4 (FW1814 masks 1 and 2, A2-P13 §5), the front knob on Line 1/2. Output and phones levels are kept: a passthrough never
    /// makes anything louder by itself.
    static func passthrough(from current: [String: Any]) -> [String: Any] {
        var s = current
        fill(&s, MAKeys.inputRoutingV2, 0)
        put(&s, MAKeys.swReturnRoutingV2, ints(s, MAKeys.swReturnRoutingV2).enumerated().map { 1 << $0.offset })
        fill(&s, MAKeys.swReturnLevels, MALevel.unity)
        fill(&s, MAKeys.swReturnMute, 0)
        fill(&s, MAKeys.swReturnSolo, 0)
        fill(&s, MAKeys.inputSolo, 0)
        fill(&s, MAKeys.outputSolo, 0)
        fill(&s, MAKeys.monitorAuxOnOutput, 0)
        if s[MAKeys.headphoneSourceV2.name] != nil { s[MAKeys.headphoneSourceV2.name] = NSNumber(value: UInt32(1)) }
        if s[MAKeys.headphoneSecondSource.name] != nil { s[MAKeys.headphoneSecondSource.name] = NSNumber(value: UInt32(2)) }
        // the front knob drives the monitor (Line 1/2): output pair 1 (MARotaryControlV2 bit 32, A2-P13 §2)
        if s[MAKeys.rotaryControl.name] != nil { s[MAKeys.rotaryControl.name] = NSNumber(value: MARotaryMask.outputPairBit(1)) }
        return s
    }

    /// Console: the passthrough with every output at 0 dB and unmuted (unity, plan v2 §4), and the front knob on nothing (an empty
    /// MARotaryControlV2 mask) so a turn cannot take the outputs off unity. **unverified:** that the FW1814 kext accepts the empty mask
    /// without a default (Q-MAMAP row 29 gives 0xFFFF00000000 as the factory value only).
    static func unity(from current: [String: Any]) -> [String: Any] {
        var s = passthrough(from: current)
        fill(&s, MAKeys.outputLevels, MALevel.unity)
        fill(&s, MAKeys.outputMute, 0)
        fill(&s, MAKeys.auxSendLevels, MARange.levelMin)        // no CUE on (the Console's PFL uses the Aux)
        if s[MAKeys.rotaryControl.name] != nil { s[MAKeys.rotaryControl.name] = NSNumber(value: UInt64(0)) }
        return s
    }

    /// Console's unity check: the interface levels between the DAW and the console (DAW returns 1-4 into the mixer, Line 1-4 out) that are not
    /// at 0 dB or are muted, as readable names. Empty = calibrated.
    static func offUnity(_ s: [String: Any]) -> [String] {
        var out: [String] = []
        let ol = ints(s, MAKeys.outputLevels), om = ints(s, MAKeys.outputMute), rl = ints(s, MAKeys.swReturnLevels), rm = ints(s, MAKeys.swReturnMute)
        for i in 0..<ol.count where ol[i] != MALevel.unity || (i < om.count && om[i] != 0) { out.append("Line \(i + 1)") }
        for i in 0..<rl.count where rl[i] != MALevel.unity || (i < rm.count && rm[i] != 0) { out.append("DAW \(i + 1)") }
        return out
    }

    /// The scene a mode gets the first time it is entered: Monitor keeps the mixer as it is (the user's own cue mixes), the others start
    /// from their template.
    static func template(_ mode: AdvancedMode, current: [String: Any]) -> [String: Any] {
        switch mode {
        case .monitor: return capture(current)
        case .daw, .hybrid: return capture(passthrough(from: current))
        case .console: return capture(unity(from: current))
        }
    }

    /// Outputs and phones that would be louder after `target` replaces `current` (a muted channel counts as the minimum level).
    static func rises(from current: [String: Any], to target: [String: Any]) -> [LevelRise] {
        var out: [LevelRise] = []
        for (lv, mu) in [(MAKeys.outputLevels, MAKeys.outputMute), (MAKeys.headphoneLevels, MAKeys.headphoneMute)] {
            let a = ints(current, lv), b = ints(target, lv), am = ints(current, mu), bm = ints(target, mu)
            guard !b.isEmpty else { continue }
            for i in 0..<min(a.count, b.count) {
                let x = (i < am.count && am[i] != 0) ? MALevel.minimum : a[i]
                let y = (i < bm.count && bm[i] != 0) ? MALevel.minimum : b[i]
                if y > x { out.append(LevelRise(key: lv.name, channel: i + 1, from: x, to: y)) }
            }
        }
        return out
    }
}

/// What a mode switch does: the scene to write and the rises to confirm. The scene of the mode left is the hardware as it is now.
struct AdvancedSwitch {
    let from: AdvancedMode
    let to: AdvancedMode
    let leaving: [String: Any]       // stored for `from`
    let applying: [String: Any]      // written for `to`
    let rises: [LevelRise]

    static func plan(from: AdvancedMode, to: AdvancedMode, current: [String: Any], stored: [String: Any]?) -> AdvancedSwitch {
        let leaving = AdvancedScene.capture(current)
        let target = stored ?? AdvancedScene.template(to, current: current)
        return AdvancedSwitch(from: from, to: to, leaving: leaving, applying: target, rises: AdvancedScene.rises(from: current, to: target))
    }
}

/// M6 (plan v2 §3): a mode the user made with ＋ — its own name, scene and view options on top of a built-in mode's view and rules.
struct CustomMode: Equatable {
    let id: String              // "custom-<n>", the key of its scene and view options in the unit's store
    var name: String
    let base: AdvancedMode
    var plist: [String: Any] { return ["ID": id, "Name": name, "Base": base.rawValue] }
    init(id: String, name: String, base: AdvancedMode) { self.id = id; self.name = name; self.base = base }
    init?(plist d: [String: Any]) {
        guard let i = d["ID"] as? String, let n = d["Name"] as? String, let b = (d["Base"] as? String).flatMap({ AdvancedMode(rawValue: $0) }) else { return nil }
        self.init(id: i, name: n, base: b)
    }
    /// Trimmed, at most 18 characters (it is a header segment); empty -> fallback
    static func clean(_ s: String, fallback: String) -> String {
        let t = s.trimmingCharacters(in: .whitespacesAndNewlines)
        return t.isEmpty ? fallback : String(t.prefix(18))
    }
}

/// The front knob (MARotaryControlV2) as the Advanced header shows and sets it (plan v1 idea 7): one group at a time (the interface's rule,
/// MARotaryMask.group), any pairs within it. Targets offered: Line 1/2 and 3/4, In 1/2 ... 7/8 (analog), DAW 1/2 and 3/4, the Aux out,
/// the phones.
enum KnobTarget {
    struct Item { let title: String; let bit: UInt64; let group: MARotaryMask.Group }
    static let items: [Item] = [Item(title: "Line 1/2", bit: MARotaryMask.outputPairBit(1), group: .output),
                                Item(title: "Line 3/4", bit: MARotaryMask.outputPairBit(2), group: .output)]
        + (1...4).map { Item(title: "In \(2 * $0 - 1)/\(2 * $0)", bit: MARotaryMask.inputPairBit($0), group: .input) }
        + [Item(title: "DAW 1/2", bit: MARotaryMask.swReturnPairBit(1), group: .swReturn),
           Item(title: "DAW 3/4", bit: MARotaryMask.swReturnPairBit(2), group: .swReturn),
           Item(title: "Aux out", bit: MARotaryMask.auxOutput, group: .aux),
           Item(title: "Phones", bit: MARotaryMask.headphone, group: .headphone)]
    /// "Line 1/2 + Line 3/4", "nothing", or "other" for bits the list does not name
    static func describe(_ mask: UInt64) -> String {
        if mask == 0 { return "nothing" }
        let on = items.filter { mask & $0.bit == $0.bit }
        return on.isEmpty ? "other" : on.map { $0.title }.joined(separator: " + ")
    }
    /// Choosing an item: in the current group it toggles; in another group it becomes the only target
    static func choose(_ item: Item, current mask: UInt64) -> UInt64 {
        if MARotaryMask.group(of: mask) == item.group { return mask & item.bit == item.bit ? mask & ~item.bit : mask | item.bit }
        return item.bit
    }
}
