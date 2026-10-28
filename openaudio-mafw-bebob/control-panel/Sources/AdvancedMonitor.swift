// Advanced window, Monitor mode logic (PANEL_REDESIGN_PLAN_v3 §3, v2 §4; phase M4). Pure logic over a scene dictionary (the mixer keys),
// unit-tested in Tests/LayoutTests.swift. The user works with up to three cue mixes, one per hardware bus; this file says what the hardware
// can do with them and writes it:
//   * Mix 1/2 and Mix 3/4 share ONE level and ONE pan per input channel (MAInputLevels 18, MAInputPan2 18; special.rs analog_gains /
//     analog_balances) and route inputs PER PAIR (MAInputRoutingV2: 9 two-bit masks, bit 0 = Mix 1/2, bit 1 = Mix 3/4; A2 PN2 row 24).
//     A channel is in such a mix when its pair is routed there and its level is above the floor.
//   * DAW returns 1-4: one level per channel, routed per pair (MASWReturnRoutingV2), no pan (odd -> left, even -> right).
//   * The Aux has its own send per channel (MAAuxSendLevels 22: inputs 1-18, then returns 1-4 = elements 19-22, TranslatorCounts
//     auxSendElement) and no pan: a channel is in the Aux mix when its send is above the floor. It is a stereo pair, not a mono bus (user,
//     2026-10-28; snd-firewire-ctl-services special.rs:26-31 "22x2 aux mixer", aux-output-1/2): like the DAW returns, an odd channel lands
//     on Aux L only, an even one on Aux R only. M and S in the Aux act there only: a source the Aux silences (muted, or not soloed while
//     another source is) has its send at the floor and the send it gets back kept (`parked`, "auxkeep.in3").
//   * Mono mix (user, 2026-10-27): its sources are centred (pan 0) unless a stereo mix on the other bus pans them (shared pan): those are
//     reported as conflicts. A source joining a mix starts at centre unless a stereo mix already pans it.
import Foundation

enum MixBus: String {
    case mix12, mix34, aux
    static let all: [MixBus] = [.mix12, .mix34, .aux]
    /// routing bit of a pair mask (MAInputRoutingV2 / MASWReturnRoutingV2)
    var bit: Int { switch self { case .mix12: return 1; case .mix34: return 2; case .aux: return 0 } }
    var other: MixBus? { switch self { case .mix12: return .mix34; case .mix34: return .mix12; case .aux: return nil } }
    var label: String { switch self { case .mix12: return "Mix 1/2"; case .mix34: return "Mix 3/4"; case .aux: return "Aux" } }
    /// phones source mask (FW1814: 1 = out 1/2, 2 = out 3/4, 4 = aux; FW1814Profile.phonesSources)
    var phonesMask: Int { switch self { case .mix12: return 1; case .mix34: return 2; case .aux: return 4 } }
}

/// One hardware bus as the user sees it (user, 2026-10-28): Mix 1/2 or Mix 3/4 is either STEREO (one mix, a pan per source) or MONO = two
/// independent mono lines, one per output (Out 1 / Out 2, Out 3 / Out 4): a source IN on the odd line only is panned hard left, on the even
/// line only hard right, on both centred — the panel computes the pan and shows no pan control. The Aux is one stereo line with no pan:
/// odd channels on Aux L, even on Aux R (MonitorSource.auxSide).
/// `name` is the stereo mix's name or the odd line's; `rightName` the even line's.
struct MonitorMix: Equatable {
    let bus: MixBus
    var name: String
    var mono: Bool
    var rightName: String
    init(bus: MixBus, name: String, mono: Bool, rightName: String = "") {
        self.bus = bus; self.name = name; self.mono = mono
        self.rightName = rightName.isEmpty ? name + " R" : rightName
    }
    static let defaults = [MonitorMix(bus: .mix12, name: "Monitors", mono: false, rightName: "Out 2"),
                           MonitorMix(bus: .mix34, name: "Cue A", mono: true, rightName: "Cue B"),
                           MonitorMix(bus: .aux, name: "Aux", mono: false)]
    /// the lines the user mixes on this bus: one (stereo, or the Aux) or two (mono)
    var lines: [MixLine] { return mono && bus != .aux ? [MixLine(bus: bus, side: .left), MixLine(bus: bus, side: .right)] : [MixLine(bus: bus, side: nil)] }
    func name(_ l: MixLine) -> String { return l.side == .right ? rightName : name }
}

/// A mix the user works on: a whole bus (stereo, or the Aux) or one mono line of a bus (`side`).
struct MixLine: Equatable {
    let bus: MixBus
    let side: MixSide?
    /// "Out 3" for a mono line, the bus label otherwise
    var output: String {
        guard let s = side else { return bus.label }
        let n = (bus == .mix12 ? 1 : 3) + (s == .right ? 1 : 0)
        return "Out \(n)"
    }
    static func == (a: MixLine, b: MixLine) -> Bool { return a.bus == b.bus && a.side == b.side }
}

/// A source strip: one input channel (mono), an input pair the user joined (stereo source: keyboards, a stereo ADAT pair; plan v2 §4), or a
/// DAW return pair (stereo).
struct MonitorSource: Equatable {
    enum Kind { case input, daw }
    let kind: Kind
    let channels: [Int]          // input: [ch] (1-18) or [ch, ch + 1] (ch odd, a stereo source); daw: [1, 2] or [3, 4]
    let id: String
    var pair: Int { return (channels[0] + 1) / 2 }
    /// a mono source's pair partner channel (the hardware routes inputs and DAW returns in pairs); a stereo source is the whole pair
    var partner: Int? { return channels.count == 1 ? DeviceModel.partner(of: channels[0]) : nil }
    /// the partner as a source of the same kind
    var partnerSource: MonitorSource? {
        guard let p = partner else { return nil }
        return kind == .input ? MonitorSource.input(p) : MonitorSource.dawMono(p)
    }
    var isStereoInput: Bool { return kind == .input && channels.count == 2 }
    /// A mono DAW return (user, 2026-10-28): the FW1814 has no pan for the DAW returns (Q-ME §FWSettingsLevels::Init: SW returns get
    /// levels, mute, solo, link and pair routing only), so an odd return always lands on the left of a bus, an even one on the right
    var fixedSide: MixSide? { return kind == .daw && channels.count == 1 ? (channels[0] % 2 == 1 ? .left : .right) : nil }
    /// The Aux side of a single channel (no pan there: odd -> Aux L, even -> Aux R); nil for a stereo source (both)
    var auxSide: MixSide? { return channels.count == 1 ? (channels[0] % 2 == 1 ? .left : .right) : nil }
    /// "In 3", "DAW 2"
    var label: String { return (kind == .input ? "In " : "DAW ") + "\(channels[0])" }
    static func input(_ ch: Int) -> MonitorSource { return MonitorSource(kind: .input, channels: [ch], id: "in\(ch)") }
    /// input pair ch (odd) and ch + 1 as one stereo source
    static func stereo(_ ch: Int) -> MonitorSource { return MonitorSource(kind: .input, channels: [ch, ch + 1], id: "in\(ch)s") }
    static func daw(_ pair: Int) -> MonitorSource { return MonitorSource(kind: .daw, channels: [2 * pair - 1, 2 * pair], id: "daw\(pair)") }
    /// one DAW return channel (1-4) on its own
    static func dawMono(_ ch: Int) -> MonitorSource { return MonitorSource(kind: .daw, channels: [ch], id: "dawm\(ch)") }
}

enum MixSide { case left, right
    var other: MixSide { return self == .left ? .right : .left }
    var letter: String { return self == .left ? "L" : "R" }
}

struct MonitorMixer {
    private(set) var s: [String: Any]
    private(set) var changed = Set<String>()
    private(set) var notes: [String] = []
    /// the console law's floor: at or below it a level or send is off
    static let off = MARange.levelMin
    /// level a source gets when it joins a mix while silent (-12 dB in half-dB steps)
    /// level a source gets when it joins a mix while silent and the panel has no level of its own for it: 0 dB (user, 2026-10-28)
    static let joinLevel = 0
    /// Levels the panel took down to switch a source OFF (the pair stays routed for the partner, or the Aux send), kept so IN gives them
    /// back (user, 2026-10-28: "it always comes back as -12 dB"). Key: "lvl.in3", "lvl.daw2", "aux.in3".
    var parked: [String: Int] = [:]
    private static func key(_ aux: Bool, _ src: MonitorSource, _ ch: Int) -> String { return (aux ? "aux." : "lvl.") + (src.kind == .input ? "in" : "daw") + "\(ch)" }
    /// Aux mute and solo (user, 2026-10-28): flags "auxmute.in3" / "auxsolo.in3" (value 1) and the send a silenced source gets back,
    /// "auxkeep.in3". The channel's own mute and solo (MAInputMute / MAInputSolo, every mix) do not change.
    static func auxKey(_ what: String, _ src: MonitorSource, _ ch: Int) -> String { return "aux" + what + "." + (src.kind == .input ? "in" : "daw") + "\(ch)" }

    init(_ scene: [String: Any], parked p: [String: Int] = [:]) { s = scene; parked = p }

    /// Silences a source's level (or Aux send) and remembers what it was
    private mutating func park(_ src: MonitorSource, aux: Bool) {
        for ch in src.channels {
            let cur = aux ? at(MAKeys.auxSendLevels, auxElement(src, ch)) : at(src.kind == .input ? MAKeys.inputLevels : MAKeys.swReturnLevels, ch)
            if cur > MonitorMixer.off { parked[MonitorMixer.key(aux, src, ch)] = cur }
        }
        if aux { setSend(src, .aux, MonitorMixer.off) } else { setLevel(src, MonitorMixer.off) }
    }
    /// Gives back a parked level (or the join level) to a silent source
    private mutating func unpark(_ src: MonitorSource, aux: Bool) {
        for ch in src.channels {
            let v = parked.removeValue(forKey: MonitorMixer.key(aux, src, ch)) ?? MonitorMixer.joinLevel
            if aux { put(MAKeys.auxSendLevels, auxElement(src, ch), v) } else { put(src.kind == .input ? MAKeys.inputLevels : MAKeys.swReturnLevels, ch, v) }
        }
    }

    // MARK: reading
    private func arr(_ k: MAKey) -> [Int] { return AdvancedScene.ints(s, k) }
    private func at(_ k: MAKey, _ i: Int) -> Int { let a = arr(k); return i >= 1 && i <= a.count ? a[i - 1] : 0 }
    private var inputCount: Int { return arr(MAKeys.inputLevels).count }
    private func auxElement(_ src: MonitorSource, _ ch: Int) -> Int { return src.kind == .input ? ch : inputCount + ch }

    func level(_ src: MonitorSource) -> Int { return at(src.kind == .input ? MAKeys.inputLevels : MAKeys.swReturnLevels, src.channels[0]) }
    func routed(_ src: MonitorSource, _ bus: MixBus) -> Bool {
        guard bus != .aux else { return false }
        return at(src.kind == .input ? MAKeys.inputRoutingV2 : MAKeys.swReturnRoutingV2, src.pair) & bus.bit != 0
    }
    /// The fader of a source in a mix: the Aux send (while the Aux silences it, the send it gets back), or the channel level
    func send(_ src: MonitorSource, _ bus: MixBus) -> Int {
        guard bus == .aux else { return level(src) }
        let ch = src.channels[0]
        return parked[MonitorMixer.auxKey("keep", src, ch)] ?? at(MAKeys.auxSendLevels, auxElement(src, ch))
    }
    func member(_ src: MonitorSource, _ bus: MixBus) -> Bool {
        return bus == .aux ? send(src, .aux) > MonitorMixer.off : routed(src, bus) && level(src) > MonitorMixer.off
    }
    func auxMuted(_ src: MonitorSource) -> Bool { return parked[MonitorMixer.auxKey("mute", src, src.channels[0])] != nil }
    func auxSoloed(_ src: MonitorSource) -> Bool { return parked[MonitorMixer.auxKey("solo", src, src.channels[0])] != nil }
    /// some source is soloed in the Aux: only the soloed ones are heard there
    var auxSoloActive: Bool { return parked.keys.contains { $0.hasPrefix("auxsolo.") } }
    /// silent in the Aux by M, or by another source's S
    func auxSilenced(_ src: MonitorSource) -> Bool { return auxMuted(src) || (auxSoloActive && !auxSoloed(src)) }
    func pan(_ ch: Int) -> Int { return at(MAKeys.inputPan2, ch) }
    /// Mix 1/2 and Mix 3/4 share the level: a source in both shows it.
    func sharedLevel(_ src: MonitorSource) -> Bool { return member(src, .mix12) && member(src, .mix34) }
    /// A source in a mono mix that a stereo mix on the other bus pans off centre (the pan is shared).
    func panConflict(_ src: MonitorSource, mono bus: MixBus, mixes: [MonitorMix]) -> MixBus? {
        guard src.kind == .input, let o = bus.other, member(src, bus), member(src, o), pan(src.channels[0]) != 0,
              mixes.first(where: { $0.bus == o })?.mono == false else { return nil }
        return o
    }

    // MARK: writing
    private mutating func put(_ k: MAKey, _ i: Int, _ v: Int) {
        var a = arr(k)
        guard i >= 1 && i <= a.count, a[i - 1] != v else { return }
        a[i - 1] = v
        s[k.name] = a.map { NSNumber(value: Int32(truncatingIfNeeded: $0)) }
        changed.insert(k.name)
    }
    private mutating func setLevel(_ src: MonitorSource, _ v: Int) {
        for ch in src.channels { put(src.kind == .input ? MAKeys.inputLevels : MAKeys.swReturnLevels, ch, v) }
    }
    private mutating func route(_ src: MonitorSource, _ bus: MixBus, _ on: Bool) {
        let k = src.kind == .input ? MAKeys.inputRoutingV2 : MAKeys.swReturnRoutingV2
        let cur = at(k, src.pair)
        put(k, src.pair, on ? cur | bus.bit : cur & ~bus.bit)
    }

    /// Fader of a source in a mix: the Aux send, or the channel level (shared by Mix 1/2 and Mix 3/4).
    mutating func setSend(_ src: MonitorSource, _ bus: MixBus, _ v: Int) {
        let x = max(MonitorMixer.off, min(MARange.levelMax, v))
        if bus == .aux {
            if auxSilenced(src) { setKeptAuxSend(src, x) } else { for ch in src.channels { put(MAKeys.auxSendLevels, auxElement(src, ch), x) } }
        } else { setLevel(src, x) }
    }

    /// M in the Aux (user, 2026-10-28): the source's Aux send to the floor, kept for M again; Mix 1/2, Mix 3/4 and the channel's own
    /// mute (MAInputMute / MASWReturnMute) do not change.
    mutating func setAuxMute(_ src: MonitorSource, _ on: Bool) { setAuxFlag("mute", src, on) }
    /// S in the Aux (user, 2026-10-28): while any source is soloed there, only the soloed ones are heard in the Aux (the others' sends
    /// to the floor, kept); Mix 1/2, Mix 3/4 and MAInputSolo / MASWReturnSolo do not change.
    mutating func setAuxSolo(_ src: MonitorSource, _ on: Bool) { setAuxFlag("solo", src, on) }
    private mutating func setAuxFlag(_ what: String, _ src: MonitorSource, _ on: Bool) {
        for ch in src.channels { let k = MonitorMixer.auxKey(what, src, ch); if on { parked[k] = 1 } else { parked.removeValue(forKey: k) } }
        applyAux()
    }
    /// The fader of a source the Aux silences moves the send it gets back; the send stays at the floor
    mutating func setKeptAuxSend(_ src: MonitorSource, _ v: Int) {
        for ch in src.channels { parked[MonitorMixer.auxKey("keep", src, ch)] = max(MonitorMixer.off, min(MARange.levelMax, v)) }
    }
    /// Puts every Aux send where M and S say: a silenced source at the floor with its send kept, the others at their send. A kept send
    /// whose live send is above the floor was raised elsewhere (the standard panel): that ends its mute.
    mutating func applyAux() {
        let soloing = auxSoloActive
        for e in 1...max(1, arr(MAKeys.auxSendLevels).count) {
            let src = e <= inputCount ? MonitorSource.input(e) : MonitorSource.dawMono(e - inputCount), ch = src.channels[0]
            let keep = MonitorMixer.auxKey("keep", src, ch), cur = at(MAKeys.auxSendLevels, e)
            if parked[keep] != nil && cur > MonitorMixer.off { parked.removeValue(forKey: keep); parked.removeValue(forKey: MonitorMixer.auxKey("mute", src, ch)) }
            let silent = parked[MonitorMixer.auxKey("mute", src, ch)] != nil || (soloing && parked[MonitorMixer.auxKey("solo", src, ch)] == nil)
            if silent {
                if cur > MonitorMixer.off { parked[keep] = cur; put(MAKeys.auxSendLevels, e, MonitorMixer.off) }
            } else if let v = parked.removeValue(forKey: keep) { put(MAKeys.auxSendLevels, e, v) }
        }
    }

    mutating func setPan(_ ch: Int, _ v: Int) { put(MAKeys.inputPan2, ch, max(MARange.panMin, min(MARange.panMax, v))) }

    /// IN / OUT of a source in a mix, keeping the hardware rules (pairs, shared level).
    mutating func setMember(_ src: MonitorSource, _ bus: MixBus, _ on: Bool, mixes: [MonitorMix]) {
        guard member(src, bus) != on else { return }
        if bus == .aux {
            if on { unpark(src, aux: true) }
            else {
                // OFF ends its M and S in the Aux; the send it had (kept or live) is what IN gives back
                for ch in src.channels {
                    parked.removeValue(forKey: MonitorMixer.auxKey("mute", src, ch)); parked.removeValue(forKey: MonitorMixer.auxKey("solo", src, ch))
                    if let v = parked.removeValue(forKey: MonitorMixer.auxKey("keep", src, ch)), v > MonitorMixer.off { parked[MonitorMixer.key(true, src, ch)] = v }
                }
                park(src, aux: true)
            }
            applyAux()
            return
        }
        let other = bus.other!
        if on {
            let wasRouted = routed(src, bus)
            route(src, bus, true)
            if level(src) <= MonitorMixer.off { unpark(src, aux: false) }
            // the pan is shared by both buses: a source already in the other bus keeps it; otherwise a stereo source spreads hard left /
            // right and a mono one starts centred (on a mono bus the line logic then sets it, setLineMember)
            if src.isStereoInput {
                if !member(src, other) { setPan(src.channels[0], MARange.panMin); setPan(src.channels[1], MARange.panMax) }
            } else if src.kind == .input {
                if !member(src, other) { setPan(src.channels[0], 0) }
            }
            if !wasRouted, let ps = src.partnerSource {
                // the partner rides in with the pair: keep it silent unless the other mix uses it (then it joins this mix too)
                if member(ps, other) { notes.append("\(ps.label) joins too: the interface routes to Mix 1/2 and Mix 3/4 in pairs, and \(ps.label) is in \(other.label)") }
                else { park(ps, aux: false) }
            }
        } else {
            if let ps = src.partnerSource, member(ps, bus) {
                // the pair stays routed for the partner: silence this channel, unless that would also take it out of the other mix
                if member(src, other) {
                    route(src, bus, false)
                    notes.append("\(ps.label) leaves too: the interface routes to Mix 1/2 and Mix 3/4 in pairs, and \(src.label) stays in \(other.label) at the same level")
                } else { park(src, aux: false) }
            } else {
                route(src, bus, false)
            }
        }
    }

    // MARK: lines (a stereo bus, the Aux, or one mono line of a bus)

    func member(_ src: MonitorSource, _ l: MixLine) -> Bool {
        guard let s = l.side else { return member(src, l.bus) }
        return sideMember(src, l.bus, s)
    }
    /// IN / OUT of a source on a line: the whole bus (stereo, Aux) or one mono line (the pan follows, setSideMember)
    mutating func setLineMember(_ src: MonitorSource, _ l: MixLine, _ on: Bool, mixes: [MonitorMix]) {
        if let s = l.side { setSideMember(src, l.bus, s, on, mixes: mixes) } else { setMember(src, l.bus, on, mixes: mixes) }
    }
    /// The partner a pair routing forced into this bus: in the bus, and in the other bus at a level (the level is shared), while the user
    /// did not link the pair — the panel marks it (hardware rule, not a choice)
    func forcedPartner(_ src: MonitorSource, _ bus: MixBus) -> Bool {
        guard bus != .aux, let ps = src.partnerSource, let o = bus.other else { return false }
        return member(src, bus) && member(src, o) && member(ps, bus)
    }
    /// Whether a line can carry the source at all: a mono DAW return only reaches its fixed side
    func reachable(_ src: MonitorSource, _ l: MixLine) -> Bool {
        guard let f = src.fixedSide, let s = l.side else { return true }
        return f == s
    }

    /// Mono mix: centre its sources unless a stereo mix on the other bus pans them.
    mutating func centre(_ bus: MixBus, mixes: [MonitorMix]) {
        guard bus != .aux else { return }
        let other = bus.other!
        let otherStereo = mixes.first(where: { $0.bus == other })?.mono == false
        for ch in 1...max(1, inputCount) {
            let src = MonitorSource.input(ch)
            guard member(src, bus), !(otherStereo && member(src, other)) else { continue }
            setPan(ch, 0)
        }
    }

    // MARK: mono lines (a bus used as two independent mono outputs)

    /// On the `side` line of a mono bus: the source is in the bus and not panned hard to the other side (centre = both lines). DAW returns
    /// have no pan (odd left, even right): on both.
    func sideMember(_ src: MonitorSource, _ bus: MixBus, _ side: MixSide) -> Bool {
        guard member(src, bus) else { return false }
        if let f = src.fixedSide { return f == side }
        guard src.kind == .input else { return true }
        let p = pan(src.channels[0])
        return side == .left ? p != MARange.panMax : p != MARange.panMin
    }
    /// IN / OUT of a source on one line of a mono bus: joining puts it hard on that side (or centre when it was on the other line only:
    /// both); leaving moves a both-lines source to the other line, or out of the bus. One level per source: the same on both lines.
    mutating func setSideMember(_ src: MonitorSource, _ bus: MixBus, _ side: MixSide, _ on: Bool, mixes: [MonitorMix]) {
        if let f = src.fixedSide { if f == side { setMember(src, bus, on, mixes: mixes) }; return }      // the other line is out of reach
        guard bus != .aux, src.kind == .input else { setMember(src, bus, on, mixes: mixes); return }
        let hard = side == .left ? MARange.panMin : MARange.panMax
        if on {
            if !member(src, bus) { setMember(src, bus, true, mixes: mixes); for ch in src.channels { setPan(ch, hard) } }
            else if !sideMember(src, bus, side) { for ch in src.channels { setPan(ch, 0) } }
            else { return }
        } else {
            guard sideMember(src, bus, side) else { return }
            if sideMember(src, bus, side.other) { for ch in src.channels { setPan(ch, -hard) } }
            else { setMember(src, bus, false, mixes: mixes); return }
        }
        if let o = bus.other, member(src, o) {
            notes.append("\(src.label) is also in \(o.label): the pan is shared, so it sits on the same side there")
        }
    }

    /// The user linked input ch (odd) with ch + 1 (MAInputLink): the partner takes ch's level and Aux send; in a stereo mix the pair spreads
    /// hard left / right, otherwise the partner takes ch's pan (same lines).
    mutating func link(_ ch: Int, mixes: [MonitorMix]) {
        let p = ch + 1
        put(MAKeys.inputLevels, p, at(MAKeys.inputLevels, ch))
        put(MAKeys.auxSendLevels, p, at(MAKeys.auxSendLevels, ch))
        for what in ["keep", "mute", "solo"] {
            let pk = MonitorMixer.auxKey(what, .input(p), p)
            if let v = parked[MonitorMixer.auxKey(what, .input(ch), ch)] { parked[pk] = v } else { parked.removeValue(forKey: pk) }
        }
        let inStereo = [MixBus.mix12, .mix34].contains { b in mixes.first(where: { $0.bus == b })?.mono == false && member(.input(ch), b) }
        if inStereo { setPan(ch, MARange.panMin); setPan(p, MARange.panMax) } else { setPan(p, pan(ch)) }
    }

    /// A mix turned stereo: its stereo sources (input pairs, first channel odd) spread hard left / right.
    mutating func spread(_ bus: MixBus, stereoPairs: [Int]) {
        guard bus != .aux else { return }
        for ch in stereoPairs where member(MonitorSource.stereo(ch), bus) { setPan(ch, MARange.panMin); setPan(ch + 1, MARange.panMax) }
    }

    /// The scene keys this mixer changed, ready for DeviceModel.restore.
    var writes: [String: Any] { var d: [String: Any] = [:]; for k in changed { d[k] = s[k] }; return d }
}
