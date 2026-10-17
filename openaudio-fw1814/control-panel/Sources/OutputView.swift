// PN.3 -- the output tab (wireframe PN1): 1/2 out, 3/4 out, the digital output strip, the aux out bus, phones 1 and 2, with the software
// return bus and input bus meters beside them. Index rules A2-P13 §1: output pair 1,2 = analog, pair 3 = S/PDIF out, pairs 4...7 = ADAT,
// pair 8/9 = phones 1/2 (cookie 0x5fd), pair 10 = aux out. Balance is derived from the two channel levels and written back as levels
// is kept apart from the faders by DeviceModel (user request 2026-10-16; the reference's cookie 0x5f6 rule mixed them); there is no balance key.
import AppKit

private struct OutputSpec {
    let title: String
    let levels: MAKey, mute: MAKey, link: MAKey
    let solo: MAKey?
    let pairInGroup: Int            // pair index inside the group's channel array (channels 2p-1, 2p)
    let monitorPair: Int?           // output pair for the main/aux toggle (MAMonitorAuxOnOutput), nil = none
    let rotary: (group: MARotaryMask.Group, bit: UInt64)?
    let peakChannels: (Int, Int)    // output channel ids for the meters (A2-P13 §6)
    let phones: Int?                // 1 or 2
}

/// Input-bus meter label for a ChannelLayout title ("1/2" -> "Analog 1/2", "adt 5/6" -> "ADAT 5/6", "spdif" -> "S/PDIF").
func displayBusLabel(_ s: String) -> String {
    if s.hasPrefix("adt ") { return "ADAT " + String(s.dropFirst(4)) }
    if s == "spdif" { return "S/PDIF" }
    return "Analog " + s
}

final class OutputTab: StripPage, PanelTab {
    private let ctx: PanelContext
    private var strips: [(strip: StripView, spec: OutputSpec)] = []
    private var digital: BusStripView?
    private var digitalMainAux: PillButton?
    private var buses: MeterBridge?
    private var builtFor: ChannelLayout?
    private var inputBusLayout: [MeterLabel] = []
    private var bridgeX: CGFloat = 0, bridgeHeight: CGFloat = 0

    init(context: PanelContext) {
        ctx = context
        super.init(frame: .zero)
        rebuild()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    func rebuild() {
        let layout = ctx.layout
        builtFor = layout
        strips.removeAll(); digital = nil; digitalMainAux = nil
        let m = ctx.model

        func make(_ spec: OutputSpec, features: StripFeatures) -> StripView {
            let s = StripView(title: spec.title, features: features, kind: .output)
            let (l, r) = DeviceModel.channels(ofPair: spec.pairInGroup)
            s.onLevel = { [weak s] side, v in
                let linked = m.pairFlag(spec.link, pair: spec.pairInGroup)
                m.setLevel(spec.levels, channel: side == 0 ? l : r, value: v, linked: linked)
                if linked { s?.setLevels(m.faderLevel(spec.levels, channel: l), m.faderLevel(spec.levels, channel: r)) }
            }
            s.onBalance = { [weak s] b in
                m.setBalance(spec.levels, pair: spec.pairInGroup, balance: b)
            }
            s.onLink = { on in m.setPairFlag(spec.link, pair: spec.pairInGroup, on: on); m.commit() }
            s.onMute = { on in m.setPairFlag(spec.mute, pair: spec.pairInGroup, on: on); m.commit() }
            if let solo = spec.solo { s.onSolo = { on in m.setPairFlag(solo, pair: spec.pairInGroup, on: on); m.commit() } }
            if let rot = spec.rotary { s.onCtrl = { on in m.setRotaryBit(rot.bit, on: on) } }
            if let mp = spec.monitorPair { s.onMainAux = { aux in m.setMonitorAux(pair: mp, aux: aux) } }
            if let ph = spec.phones { s.onMon = { src in m.setPhonesSource(phones: ph, source: src) } }
            s.onCommit = { m.commit() }
            return s
        }

        let out12 = OutputSpec(title: "Out 1/2", levels: MAKeys.outputLevels, mute: MAKeys.outputMute, link: MAKeys.outputLink, solo: MAKeys.outputSolo,
                               pairInGroup: 1, monitorPair: 1, rotary: (.output, MARotaryMask.outputPairBit(1)), peakChannels: (1, 2), phones: nil)
        let out34 = OutputSpec(title: "Out 3/4", levels: MAKeys.outputLevels, mute: MAKeys.outputMute, link: MAKeys.outputLink, solo: MAKeys.outputSolo,
                               pairInGroup: 2, monitorPair: 2, rotary: (.output, MARotaryMask.outputPairBit(2)), peakChannels: (3, 4), phones: nil)
        let aux = OutputSpec(title: "Aux Out", levels: MAKeys.auxOutputLevels, mute: MAKeys.auxOutputMute, link: MAKeys.auxOutputLink, solo: nil,
                             pairInGroup: 1, monitorPair: nil, rotary: (.aux, MARotaryMask.auxOutput), peakChannels: (19, 20), phones: nil)
        let hp1 = OutputSpec(title: "Phones 1", levels: MAKeys.headphoneLevels, mute: MAKeys.headphoneMute, link: MAKeys.headphoneLink, solo: nil,
                             pairInGroup: 1, monitorPair: nil, rotary: nil, peakChannels: (15, 16), phones: 1)
        let hp2 = OutputSpec(title: "Phones 2", levels: MAKeys.headphoneLevels, mute: MAKeys.headphoneMute, link: MAKeys.headphoneLink, solo: nil,
                             pairInGroup: 2, monitorPair: nil, rotary: nil, peakChannels: (17, 18), phones: 2)

        var views: [NSView] = []
        let top = StripPage.stripTop
        let height = StripView.Geometry(.output).height
        var x = Theme.pad
        func put(_ v: NSView) { v.setFrameOrigin(NSPoint(x: x, y: top)); views.append(v); x += v.frame.width + Theme.stripGap }

        // Main outputs + the digital output strip
        let s1 = x
        // 176.4 / 192 kHz: the interface's DSP mixer is dead (user, hardware, 2026-10-17; the manual says so too), so the main/aux monitor
        // choice, the aux out and every bus meter are gone; only output levels, mutes and the phones remain.
        let quad = layout.rateClass == .quad
        for spec in [out12, out34] {
            let s = make(spec, features: quad ? [.balance, .ctrl, .solo, .linkMute, .faders] : [.balance, .ctrl, .solo, .linkMute, .mainAux, .faders])
            strips.append((s, spec)); put(s)
        }
        switch layout.returnBus {
        case .adat:
            let b = BusStripView(title: "ADAT Out", channels: 8, kind: .output); digital = b; put(b)
        case .spdif:
            let b = BusStripView(title: "S/PDIF Out", channels: 2, kind: .output, numbers: ["L", "R"]); digital = b
            let t = PillButton("main"); digitalMainAux = t
            StripView.styleMainAux(t, aux: false)
            t.onClick = { btn in
                StripView.styleMainAux(btn, aux: btn.isOn)
                m.setMonitorAux(pair: ChannelLayout.spdifOutputPair, aux: btn.isOn)
            }
            b.addBottomButton(t); put(b)
        case .none: break
        }
        views.append(StripPage.section("Main Outputs", x: s1, width: x - s1))
        x += Theme.sectionGap - Theme.stripGap

        // Aux and headphones
        let s2 = x
        let hpFeatures: StripFeatures = [.balance, .linkMute, .mon, .faders]
        let auxFeatures: StripFeatures = [.balance, .ctrl, .linkMute, .faders]
        for (spec, f) in (quad ? [(hp1, hpFeatures), (hp2, hpFeatures)] : [(aux, auxFeatures), (hp1, hpFeatures), (hp2, hpFeatures)]) {
            let s = make(spec, features: f)
            if quad && spec.phones != nil { s.monButtons.last?.isHidden = true }      // phones source "aux" is a DSP-mixer bus: not at 176.4/192 kHz
            strips.append((s, spec)); put(s)
        }
        views.append(StripPage.section(quad ? "Phones" : "Aux & Phones", x: s2, width: x - s2))
        x += Theme.sectionGap - Theme.stripGap
        if quad {
            buses = nil; inputBusLayout = []; bridgeHeight = 0
            setPage(views, size: NSSize(width: x - Theme.sectionGap + Theme.stripGap + Theme.pad, height: top + height + Theme.pad))
            refresh()
            return
        }

        // Bus meters beside the strips, same height
        inputBusLayout = layout.inputBusMeters
        let bridge = MeterBridge(axis: .column, groups: [MeterBridge.Group(title: "Software Return Bus", labels: ["1/2", "3/4"]),
                                                        MeterBridge.Group(title: "Input Bus", labels: inputBusLayout.map { displayBusLabel($0.title) })])
        // Full strip height when the rows fill most of it, else just the rows (176.4/192 kHz has three pairs).
        let rows = bridge.columnContentHeight
        bridgeX = x; bridgeHeight = rows > 0.7 * height ? height : rows
        bridge.layout(width: 260, height: bridgeHeight)
        bridge.setFrameOrigin(NSPoint(x: x, y: top))
        bridge.setEnabled([true, true] + inputBusLayout.map { $0.enabled })
        buses = bridge
        views.append(bridge)
        views.append(StripPage.section("Bus Meters", x: x, width: 260))
        setPage(views, size: NSSize(width: x + 260 + Theme.pad, height: top + height + Theme.pad))
        fitBridge()
        refresh()
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        fitBridge()
    }

    var preferredSize: NSSize { return NSSize(width: bridgeX + 380 + Theme.pad, height: page.frame.height) }

    /// The bus meter panel takes the free width right of the strips (260...560 pt), so the tab has no empty band on the right.
    private func fitBridge() {
        guard let b = buses else { return }
        let w = max(260, min(560, (bounds.width - bridgeX - Theme.pad).rounded(.down)))
        if abs(b.frame.width - w) > 0.5 { b.layout(width: w, height: bridgeHeight) }
        page.setFrameSize(NSSize(width: bridgeX + w + Theme.pad, height: page.frame.height))
    }

    func refresh() {
        if builtFor != ctx.layout { rebuild(); return }
        let m = ctx.model
        for (s, spec) in strips {
            let (l, r) = DeviceModel.channels(ofPair: spec.pairInGroup)
            s.setLevels(m.faderLevel(spec.levels, channel: l), m.faderLevel(spec.levels, channel: r))
            s.setBalance(m.balance(spec.levels, pair: spec.pairInGroup))
            s.setFlags(link: m.pairFlag(spec.link, pair: spec.pairInGroup), mute: m.pairFlag(spec.mute, pair: spec.pairInGroup),
                       solo: spec.solo.map { m.pairFlag($0, pair: spec.pairInGroup) } ?? false)
            if let rot = spec.rotary { let c = ctx.ctrlState(group: rot.group, bit: rot.bit); s.setCtrl(enabled: c.enabled, on: c.on) }
            if let mp = spec.monitorPair { s.setMainAux(aux: m.monitorAux(pair: mp)) }
            if let ph = spec.phones {
                let key = ph == 1 ? MAKeys.headphoneSourceV2 : MAKeys.headphoneSecondSource
                s.setMon(MAHeadphoneSource(rawValue: UInt32(truncatingIfNeeded: m.number(key) ?? 0)))
            }
        }
        if let t = digitalMainAux { let aux = m.monitorAux(pair: ChannelLayout.spdifOutputPair); t.isOn = aux; StripView.styleMainAux(t, aux: aux) }
        updateMeters()
    }

    func updateMeters() {
        let quad = ctx.layout.rateClass == .quad
        for (s, spec) in strips {
            if quad {
                // The hardware output peaks come from the DSP mixer and freeze when it stops. What can still be shown is the software peak of what
                // Core Audio sends (kext peak map, SW-return slots 1...4 = outputs 1...4); the phones have no meter at all.
                let main = spec.peakChannels.0 <= 4
                s.setMeters(main ? ctx.db(PeakSlot.swReturn(spec.peakChannels.0)) : MARange.peakFloorDB, main ? ctx.db(PeakSlot.swReturn(spec.peakChannels.1)) : MARange.peakFloorDB)
                continue
            }
            s.setMeters(ctx.db(PeakSlot.output(spec.peakChannels.0)), ctx.db(PeakSlot.output(spec.peakChannels.1)))
        }
        if let d = digital {
            // S/PDIF out = output channels 5, 6; ADAT = channels 7...14 (order consistent with the kext peak maps, A2-P13 §1).
            let first = ctx.layout.returnBus == .adat ? 7 : 5
            for i in 0..<d.meters.count { d.meters[i].db = ctx.db(PeakSlot.output(first + i)) }
            d.setGreyed(from: ctx.layout.adatBusHalfGreyed ? 4 : d.meters.count)
        }
        // SW return bus 1/2, 3/4; input bus: analog pairs 1...4 = channels 1...8, then the S/PDIF pair (5) or ADAT pairs (6...9), as in the strip list.
        var v: [[Int]] = [[ctx.db(PeakSlot.swReturn(1)), ctx.db(PeakSlot.swReturn(2))], [ctx.db(PeakSlot.swReturn(3)), ctx.db(PeakSlot.swReturn(4))]]
        let pairs = ctx.layout.inputStrips.map { $0.pair }
        v += pairs.map { p -> [Int] in let (l, r) = DeviceModel.channels(ofPair: p); return [ctx.db(PeakSlot.input(l)), ctx.db(PeakSlot.input(r))] }
        buses?.setValues(v)
    }
}
