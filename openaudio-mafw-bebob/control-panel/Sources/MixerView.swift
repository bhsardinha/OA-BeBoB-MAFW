// PN.3 / AP-5.1 -- the mixer tab: software-return strips (and a model's digital return bus) on the left, input strips on the right, bus meters
// underneath. What exists comes from the device profile's MixerPlan (Sources/Models/<Model>/); every write goes through DeviceModel ->
// IORegistryEntrySetCFProperties. Index rules: the profile's translator (mixer pair p = channels 2p-1, 2p; aux-send element = mixer channel id).
import AppKit

/// Scrolling page shared by the mixer and output tabs: a flipped document view in a scroll view that fills the tab. The content (`page`)
/// sits centred in a holder at least as wide as the visible area (user, 2026-10-20: tab contents centred, not left-aligned).
class StripPage: FlippedView {
    let scroll = NSScrollView()
    let page = FlippedView()
    private let holder = FlippedView()

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        scroll.hasHorizontalScroller = true
        scroll.hasVerticalScroller = true
        scroll.autohidesScrollers = true
        scroll.drawsBackground = false
        scroll.contentView.drawsBackground = false
        scroll.borderType = .noBorder
        scroll.frame = bounds
        scroll.autoresizingMask = [.width, .height]
        holder.addSubview(page)
        addSubview(scroll)
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        recenter()
    }

    /// Centres the content horizontally when the tab is wider than it; scrolls as before when it is narrower.
    func recenter() {
        let vis = scroll.contentSize
        let w = max(page.frame.width, vis.width)
        holder.frame = NSRect(x: 0, y: 0, width: w, height: page.frame.height)
        page.setFrameOrigin(NSPoint(x: ((w - page.frame.width) / 2).rounded(.down), y: 0))
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    /// Replaces the page content and sets its size.
    func setPage(_ views: [NSView], size: NSSize) {
        page.subviews.forEach { $0.removeFromSuperview() }
        views.forEach { page.addSubview($0) }
        page.frame = NSRect(origin: .zero, size: size)
        if scroll.documentView !== holder { scroll.documentView = holder }
        recenter()
    }

    /// A section caption over a run of strips.
    static func section(_ title: String, x: CGFloat, width: CGFloat) -> SectionLabel {
        let l = SectionLabel(title)
        l.frame = NSRect(x: x, y: Theme.pad + 2, width: width, height: 14)
        return l
    }
    static var stripTop: CGFloat { return Theme.pad + Theme.sectionLabelHeight }
}

/// Placeholder in the space of strips that do not exist at the current rate.
final class NoteView: FlippedView {
    let text: String
    init(_ text: String) { self.text = text; super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func draw(_ dirtyRect: NSRect) {
        let p = NSBezierPath(roundedRect: bounds.insetBy(dx: 1, dy: 1), xRadius: Theme.radius, yRadius: Theme.radius)
        p.lineWidth = 1; p.setLineDash([4, 4], count: 2, phase: 0)
        Theme.text4.setStroke(); p.stroke()
        let ps = NSMutableParagraphStyle(); ps.alignment = .center; ps.lineBreakMode = .byWordWrapping; ps.lineSpacing = 3
        let a = NSAttributedString(string: text, attributes: [.font: Theme.font(12), .foregroundColor: Theme.text3, .paragraphStyle: ps])
        let r = bounds.insetBy(dx: 24, dy: 0)
        let h = ceil(a.boundingRect(with: NSSize(width: r.width, height: 400), options: [.usesLineFragmentOrigin]).height)
        a.draw(with: NSRect(x: r.minX, y: (bounds.midY - h / 2).rounded(), width: r.width, height: h + 4), options: [.usesLineFragmentOrigin])
    }
}

final class MixerTab: StripPage, PanelTab {
    private let ctx: PanelContext
    private var inputStrips: [(strip: StripView, pair: Int)] = []
    private var swStrips: [(strip: StripView, pair: Int)] = []
    private var busStrip: BusStripView?
    private var plan: MixerPlan?
    private var buses: MeterBridge?
    private var builtFor: ChannelLayout?

    init(context: PanelContext) {
        ctx = context
        super.init(frame: .zero)
        rebuild()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    func rebuild() {
        let layout = ctx.layout
        let plan = ctx.profile.mixer(layout)
        self.plan = plan
        builtFor = layout
        inputStrips.removeAll(); swStrips.removeAll(); busStrip = nil
        var views: [NSView] = []
        let top = StripPage.stripTop
        var x = Theme.pad
        func put(_ v: NSView) { v.setFrameOrigin(NSPoint(x: x, y: top)); views.append(v); x += v.frame.width + Theme.stripGap }

        if let note = plan.pageNote {
            let n = NoteView(note)
            n.frame = NSRect(x: Theme.pad, y: top, width: 5 * Theme.stripWidth + 4 * Theme.stripGap, height: 150)
            setPage([n], size: NSSize(width: n.frame.maxX + Theme.pad, height: top + 150 + Theme.pad))
            return
        }

        // Software returns + the model's digital return bus
        let routes = ctx.profile.routeLabels
        let routeRows = routes.count > 3 ? (routes.count + 1) / 2 : 1                 // FW410: five destinations, 3 rows of the 2-column grid
        let s1 = x
        for r in plan.returns {
            let s = StripView(title: r.title, features: [.aux, .ctrl, .solo, .linkMute, .route, .faders], kind: .mixer, routeLabels: routes, bottomRows: routeRows)
            wireSW(s, pair: r.pair)
            swStrips.append((s, r.pair)); put(s)
        }
        if let b = plan.returnBus {
            let v = BusStripView(title: b.title, channels: b.slots.count, kind: .mixer, numbers: b.numbers); busStrip = v; put(v)
        }
        views.append(StripPage.section("Software Returns", x: s1, width: x - s1))
        x += Theme.sectionGap - Theme.stripGap

        // Inputs
        let s2 = x
        for info in plan.inputs {
            let s = StripView(title: info.title, features: [.aux, .pan, .ctrl, .solo, .linkMute, .route, .faders], kind: .mixer, routeLabels: routes, bottomRows: routeRows)
            wireInput(s, pair: info.pair)
            inputStrips.append((s, info.pair)); put(s)
        }
        if let note = plan.inputsNote {
            let n = NoteView(note)
            n.frame = NSRect(x: x, y: top, width: 3 * Theme.stripWidth + 2 * Theme.stripGap, height: StripView.Geometry(.mixer, bottomRows: routeRows).height)
            views.append(n); x += n.frame.width + Theme.stripGap
        }
        views.append(StripPage.section("Inputs", x: s2, width: x - s2))
        let right = x - Theme.stripGap

        // Bus meters under the strips, spanning their width
        let bridge = MeterBridge(axis: .row, groups: plan.bus.map { MeterBridge.Group(title: $0.title, labels: $0.labels) })
        let by = top + StripView.Geometry(.mixer, bottomRows: routeRows).height + 16
        bridge.layout(width: right - Theme.pad, height: MeterBridge.rowHeight)
        bridge.setFrameOrigin(NSPoint(x: Theme.pad, y: by))
        buses = bridge
        views.append(bridge)
        setPage(views, size: NSSize(width: right + Theme.pad, height: by + MeterBridge.rowHeight + Theme.pad))
        refresh()
    }

    var preferredSize: NSSize { return page.frame.size }

    // MARK: wiring

    private func wireInput(_ s: StripView, pair p: Int) {
        let (l, r) = DeviceModel.channels(ofPair: p)
        let m = ctx.model
        let t = ctx.profile.counts
        s.onLevel = { [weak s] side, v in
            let linked = m.pairFlag(MAKeys.inputLink, pair: p)
            m.setLevel(MAKeys.inputLevels, channel: side == 0 ? l : r, value: v, linked: linked)
            if linked { s?.setLevels(m.element(MAKeys.inputLevels, l), m.element(MAKeys.inputLevels, r)) }
        }
        s.onAux = { side, v in m.setElement(MAKeys.auxSendLevels, t.auxSendElement(input: side == 0 ? l : r), v) }
        s.onPan = { side, v in m.setElement(MAKeys.inputPan2, side == 0 ? l : r, v) }
        s.onLink = { on in m.setPairFlag(MAKeys.inputLink, pair: p, on: on); m.commit() }
        s.onMute = { on in m.setPairFlag(MAKeys.inputMute, pair: p, on: on); m.commit() }
        s.onSolo = { on in m.setPairFlag(MAKeys.inputSolo, pair: p, on: on); m.commit() }
        s.onCtrl = { on in m.setRotaryBit(MARotaryMask.inputPairBit(p), on: on) }
        s.onRoute = { [weak self] dest, on in self?.ctx.setRoute(MAKeys.inputRoutingV2, element: p, dest: dest, on: on) }
        s.onCommit = { m.commit() }
    }

    private func wireSW(_ s: StripView, pair p: Int) {
        let (l, r) = DeviceModel.channels(ofPair: p)
        let m = ctx.model
        let t = ctx.profile.counts
        s.onLevel = { [weak s] side, v in
            let linked = m.pairFlag(MAKeys.swReturnLink, pair: p)
            m.setLevel(MAKeys.swReturnLevels, channel: side == 0 ? l : r, value: v, linked: linked)
            if linked { s?.setLevels(m.element(MAKeys.swReturnLevels, l), m.element(MAKeys.swReturnLevels, r)) }
        }
        s.onAux = { side, v in m.setElement(MAKeys.auxSendLevels, t.auxSendElement(swReturn: side == 0 ? l : r), v) }
        s.onLink = { on in m.setPairFlag(MAKeys.swReturnLink, pair: p, on: on); m.commit() }
        s.onMute = { on in m.setPairFlag(MAKeys.swReturnMute, pair: p, on: on); m.commit() }
        s.onSolo = { on in m.setPairFlag(MAKeys.swReturnSolo, pair: p, on: on); m.commit() }
        s.onCtrl = { on in m.setRotaryBit(MARotaryMask.swReturnPairBit(p), on: on) }
        s.onRoute = { [weak self] dest, on in self?.ctx.setRoute(MAKeys.swReturnRoutingV2, element: p, dest: dest, on: on) }
        s.onCommit = { m.commit() }
    }

    // MARK: refresh

    func refresh() {
        if builtFor != ctx.layout { rebuild(); return }
        let m = ctx.model
        let t = ctx.profile.counts
        for (s, p) in inputStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            if let info = plan?.inputs.first(where: { $0.pair == p }) { s.setStripEnabled(info.enabled) }
            s.setLevels(m.element(MAKeys.inputLevels, l), m.element(MAKeys.inputLevels, r))
            s.setAux(m.element(MAKeys.auxSendLevels, t.auxSendElement(input: l)), m.element(MAKeys.auxSendLevels, t.auxSendElement(input: r)))
            s.setPan(m.element(MAKeys.inputPan2, l), m.element(MAKeys.inputPan2, r))
            s.setFlags(link: m.pairFlag(MAKeys.inputLink, pair: p), mute: m.pairFlag(MAKeys.inputMute, pair: p), solo: m.pairFlag(MAKeys.inputSolo, pair: p))
            let c = ctx.ctrlState(group: .input, bit: MARotaryMask.inputPairBit(p)); s.setCtrl(enabled: c.enabled, on: c.on)
            s.setRoute(mask: m.element(MAKeys.inputRoutingV2, p))
        }
        for (s, p) in swStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            s.setLevels(m.element(MAKeys.swReturnLevels, l), m.element(MAKeys.swReturnLevels, r))
            s.setAux(m.element(MAKeys.auxSendLevels, t.auxSendElement(swReturn: l)), m.element(MAKeys.auxSendLevels, t.auxSendElement(swReturn: r)))
            s.setFlags(link: m.pairFlag(MAKeys.swReturnLink, pair: p), mute: m.pairFlag(MAKeys.swReturnMute, pair: p), solo: m.pairFlag(MAKeys.swReturnSolo, pair: p))
            let c = ctx.ctrlState(group: .swReturn, bit: MARotaryMask.swReturnPairBit(p)); s.setCtrl(enabled: c.enabled, on: c.on)
            s.setRoute(mask: m.element(MAKeys.swReturnRoutingV2, p))
        }
        updateMeters()
    }

    func updateMeters() {
        let pk = ctx.peaks
        for (s, p) in inputStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            s.setMeters(ctx.db(pk.input(l)), ctx.db(pk.input(r)))
        }
        for (s, p) in swStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            s.setMeters(ctx.db(pk.swReturn(l)), ctx.db(pk.swReturn(r)))
        }
        if let b = busStrip, let bp = plan?.returnBus {
            for i in 0..<b.meters.count { b.meters[i].db = ctx.db(bp.slots[i]) }
            b.setGreyed(from: bp.greyedFrom)
        }
        buses?.setValues((plan?.bus ?? []).flatMap { g in g.slots.map { $0.map { ctx.db($0) } } })
    }
}
