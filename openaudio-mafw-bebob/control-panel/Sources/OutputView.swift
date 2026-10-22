// PN.3 / AP-5.1 -- the output tab: the main outputs (and a model's digital output bus strip), aux out and phones, with the software-return bus
// and input bus meters beside them. What exists comes from the device profile's OutputPlan (Sources/Models/<Model>/). Index rules A2-P13 §1 /
// PNAP §2: output pairs, phones (cookie 0x5fd), aux out. Balance is kept apart from the faders by DeviceModel (user request 2026-10-16; the
// reference's cookie 0x5f6 rule mixed them); there is no balance key.
import AppKit

final class OutputTab: StripPage, PanelTab {
    private let ctx: PanelContext
    private var strips: [(strip: StripView, spec: OutputStripPlan)] = []
    private var digital: BusStripView?
    private var digitalMainAux: PillButton?
    private var plan: OutputPlan?
    private var buses: MeterBridge?
    private var builtFor: ChannelLayout?
    private var bridgeX: CGFloat = 0, bridgeHeight: CGFloat = 0

    init(context: PanelContext) {
        ctx = context
        super.init(frame: .zero)
        rebuild()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    func rebuild() {
        let layout = ctx.layout
        let plan = ctx.profile.output(layout)
        self.plan = plan
        builtFor = layout
        strips.removeAll(); digital = nil; digitalMainAux = nil
        let m = ctx.model
        let sources = ctx.profile.phonesSources
        // One extra bottom row on every strip when a strip needs it: four headphone sources (2 x 2 grid) or the aux pan pair.
        let monRows = (sources.count > 3 || (plan.main + plan.other).contains { $0.pan != nil }) ? 2 : 1

        func make(_ spec: OutputStripPlan) -> StripView {
            let s = StripView(title: spec.title, features: spec.features, kind: .output, law: DeviceModel.law(spec.levels),
                              monSources: spec.phones != nil ? sources : [], bottomRows: monRows)
            let (l, r) = DeviceModel.channels(ofPair: spec.pairInGroup)
            s.onLevel = { [weak s] side, v in
                let linked = m.pairFlag(spec.link, pair: spec.pairInGroup)
                m.setLevel(spec.levels, channel: side == 0 ? l : r, value: v, linked: linked)
                if linked { s?.setLevels(m.faderLevel(spec.levels, channel: l), m.faderLevel(spec.levels, channel: r)) }
            }
            s.onBalance = { b in m.setBalance(spec.levels, pair: spec.pairInGroup, balance: b) }
            s.onLink = { on in m.setPairFlag(spec.link, pair: spec.pairInGroup, on: on); m.commit() }
            s.onMute = { on in m.setPairFlag(spec.mute, pair: spec.pairInGroup, on: on); m.commit() }
            if let solo = spec.solo { s.onSolo = { on in m.setPairFlag(solo, pair: spec.pairInGroup, on: on); m.commit() } }
            if let rot = spec.rotary { s.onCtrl = { on in m.setRotaryBit(rot.bit, on: on) } }
            if let mp = spec.monitorPair { s.onMainAux = { aux in m.setMonitorAux(pair: mp, aux: aux) } }
            if let ph = spec.phones { s.onMon = { mask in m.setPhonesSource(phones: ph, mask: mask) } }
            if let pan = spec.pan { s.onPan = { side, v in m.setElement(pan, side == 0 ? l : r, v) } }
            s.onCommit = { m.commit() }
            if spec.hideLastMon { s.monButtons.last?.isHidden = true }
            return s
        }

        var views: [NSView] = []
        let top = StripPage.stripTop
        let height = StripView.Geometry(.output, bottomRows: monRows).height
        var x = Theme.pad
        func put(_ v: NSView) { v.setFrameOrigin(NSPoint(x: x, y: top)); views.append(v); x += v.frame.width + Theme.stripGap }

        // Main outputs + the model's digital output bus strip
        let s1 = x
        for spec in plan.main { let s = make(spec); strips.append((s, spec)); put(s) }
        if let b = plan.digitalBus {
            let v = BusStripView(title: b.title, channels: b.slots.count, kind: .output, numbers: b.numbers); digital = v
            if let pair = b.mainAuxPair {
                let t = PillButton("main"); digitalMainAux = t
                StripView.styleMainAux(t, aux: false)
                t.onClick = { btn in
                    StripView.styleMainAux(btn, aux: btn.isOn)
                    m.setMonitorAux(pair: pair, aux: btn.isOn)
                }
                v.addBottomButton(t)
            }
            put(v)
        }
        views.append(StripPage.section(plan.mainTitle, x: s1, width: x - s1))
        x += Theme.sectionGap - Theme.stripGap

        // Aux and headphones
        let s2 = x
        for spec in plan.other { let s = make(spec); strips.append((s, spec)); put(s) }
        views.append(StripPage.section(plan.otherTitle, x: s2, width: x - s2))
        x += Theme.sectionGap - Theme.stripGap
        guard let groups = plan.bus else {
            buses = nil; bridgeHeight = 0
            setPage(views, size: NSSize(width: x - Theme.sectionGap + Theme.stripGap + Theme.pad, height: top + height + Theme.pad))
            refresh()
            return
        }

        // Bus meters beside the strips, same height
        let bridge = MeterBridge(axis: .column, groups: groups.map { MeterBridge.Group(title: $0.title, labels: $0.labels) })
        // Full strip height when the rows fill most of it, else just the rows.
        let rows = bridge.columnContentHeight
        bridgeX = x; bridgeHeight = rows > 0.7 * height ? height : rows
        bridge.layout(width: 260, height: bridgeHeight)
        bridge.setFrameOrigin(NSPoint(x: x, y: top))
        bridge.setEnabled(groups.flatMap { $0.enabled })
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
        recenter()
    }

    var preferredSize: NSSize { return NSSize(width: bridgeX + 380 + Theme.pad, height: page.frame.height) }

    /// The bus meter panel takes the free width right of the strips (260...560 pt), so the tab has no empty band on the right.
    private func fitBridge() {
        guard let b = buses else { return }
        let w = max(260, min(560, (bounds.width - bridgeX - Theme.pad).rounded(.down)))
        if abs(b.frame.width - w) > 0.5 { b.layout(width: w, height: bridgeHeight) }
        page.setFrameSize(NSSize(width: bridgeX + w + Theme.pad, height: page.frame.height))
        recenter()
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
            if let pan = spec.pan { s.setPan(m.element(pan, l), m.element(pan, r)) }
            if let ph = spec.phones {
                let key = ph == 1 ? MAKeys.headphoneSourceV2 : MAKeys.headphoneSecondSource
                s.setMon(UInt32(truncatingIfNeeded: m.number(key) ?? 0))
            }
        }
        if let t = digitalMainAux, let pair = plan?.digitalBus?.mainAuxPair { let aux = m.monitorAux(pair: pair); t.isOn = aux; StripView.styleMainAux(t, aux: aux) }
        updateMeters()
    }

    func updateMeters() {
        func db(_ slot: Int) -> Int { return slot < 0 ? MARange.peakFloorDB : ctx.db(slot) }
        for (s, spec) in strips { s.setMeters(db(spec.meterSlots.0), db(spec.meterSlots.1)) }
        if let d = digital, let bp = plan?.digitalBus {
            for i in 0..<d.meters.count { d.meters[i].db = ctx.db(bp.slots[i]) }
            d.setGreyed(from: bp.greyedFrom)
        }
        buses?.setValues((plan?.bus ?? []).flatMap { g in g.slots.map { $0.map { ctx.db($0) } } })
    }
}
