// PN.3 -- the mixer tab (wireframe PN1): sw rtn strips and the digital return bus on the left, input strips on the right, output bus and
// aux bus meters underneath. Every write goes through DeviceModel -> IORegistryEntrySetCFProperties (never IOConnectCallMethod).
// Index rules: A2-P13 §1 (mixer channel id 1...32, pair p = channels 2p-1, 2p). The aux-send element of a strip is its mixer channel id
// (input) or 18 + SW-return channel (SW return): the SW-return half is INFERRED from the 22-element group (18 + 4), P13 §4.
import AppKit

/// Strip title shown for a ChannelLayout input title ("analog 1/2 in" -> "Analog 1/2", "adat 5/6 in" -> "ADAT 5/6", "spdif in" -> "S/PDIF").
func displayInputTitle(_ s: String) -> String {
    var t = s.hasSuffix(" in") ? String(s.dropLast(3)) : s
    t = t.replacingOccurrences(of: "analog", with: "Analog").replacingOccurrences(of: "adat", with: "ADAT").replacingOccurrences(of: "spdif", with: "S/PDIF")
    return t
}

/// Scrolling page shared by the mixer and output tabs: a flipped document view in a scroll view that fills the tab.
class StripPage: FlippedView {
    let scroll = NSScrollView()
    let page = FlippedView()

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
        addSubview(scroll)
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    /// Replaces the page content and sets its size.
    func setPage(_ views: [NSView], size: NSSize) {
        page.subviews.forEach { $0.removeFromSuperview() }
        views.forEach { page.addSubview($0) }
        page.frame = NSRect(origin: .zero, size: size)
        if scroll.documentView !== page { scroll.documentView = page }
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
    private var swStrips: [(strip: StripView, pair: Int)] = []     // SW pair 1, 2 = "1/2 sw rtn", "3/4 sw rtn"
    private var busStrip: BusStripView?
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
        builtFor = layout
        inputStrips.removeAll(); swStrips.removeAll(); busStrip = nil
        var views: [NSView] = []
        let top = StripPage.stripTop
        var x = Theme.pad
        func put(_ v: NSView) { v.setFrameOrigin(NSPoint(x: x, y: top)); views.append(v); x += v.frame.width + Theme.stripGap }

        // 176.4 / 192 kHz: the DSP mixer is dead (user, hardware, 2026-10-17): no inputs, returns, aux sends, routing or input meters to show.
        if layout.rateClass == .quad {
            let n = NoteView("At 176.4 and 192 kHz the interface's internal mixer is off: audio goes straight through Core Audio, so there are no inputs, software returns, aux sends or routing here, and no input metering. Output levels and the phones are on the Output tab.")
            n.frame = NSRect(x: Theme.pad, y: top, width: 5 * Theme.stripWidth + 4 * Theme.stripGap, height: 150)
            setPage([n], size: NSSize(width: n.frame.maxX + Theme.pad, height: top + 150 + Theme.pad))
            return
        }

        // Software returns + the digital return bus
        let s1 = x
        for k in 1...2 {
            let s = StripView(title: k == 1 ? "Return 1/2" : "Return 3/4", features: [.aux, .ctrl, .solo, .linkMute, .route, .faders], kind: .mixer)
            wireSW(s, pair: k)
            swStrips.append((s, k)); put(s)
        }
        switch layout.returnBus {
        case .adat: let b = BusStripView(title: "ADAT Bus", channels: 8, kind: .mixer); busStrip = b; put(b)
        case .spdif: let b = BusStripView(title: "S/PDIF Bus", channels: 2, kind: .mixer, numbers: ["L", "R"]); busStrip = b; put(b)
        case .none: break
        }
        views.append(StripPage.section("Software Returns", x: s1, width: x - s1))
        x += Theme.sectionGap - Theme.stripGap

        // Inputs
        let s2 = x
        for info in layout.inputStrips {
            let s = StripView(title: displayInputTitle(info.title), features: [.aux, .pan, .ctrl, .solo, .linkMute, .route, .faders], kind: .mixer)
            wireInput(s, pair: info.pair)
            inputStrips.append((s, info.pair)); put(s)
        }
        if !layout.digitalAvailable {
            let n = NoteView("Inputs 3-8 and the digital ports are not available at 176.4 and 192 kHz: the interface runs\u{00A0}2\u{00A0}in\u{00A0}/\u{00A0}4\u{00A0}out.")
            n.frame = NSRect(x: x, y: top, width: 3 * Theme.stripWidth + 2 * Theme.stripGap, height: StripView.Geometry(.mixer).height)
            views.append(n); x += n.frame.width + Theme.stripGap
        }
        views.append(StripPage.section("Inputs", x: s2, width: x - s2))
        let right = x - Theme.stripGap

        // Bus meters under the strips, spanning their width
        let bridge = MeterBridge(axis: .row, groups: [MeterBridge.Group(title: "Output Bus", labels: ["1/2", "3/4"]),
                                                     MeterBridge.Group(title: "Aux Bus", labels: ["L / R"])])
        let by = top + StripView.Geometry(.mixer).height + 16
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
        s.onLevel = { [weak s] side, v in
            let linked = m.pairFlag(MAKeys.inputLink, pair: p)
            m.setLevel(MAKeys.inputLevels, channel: side == 0 ? l : r, value: v, linked: linked)
            if linked { s?.setLevels(m.element(MAKeys.inputLevels, l), m.element(MAKeys.inputLevels, r)) }
        }
        s.onAux = { side, v in m.setElement(MAKeys.auxSendLevels, side == 0 ? l : r, v) }
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
        s.onLevel = { [weak s] side, v in
            let linked = m.pairFlag(MAKeys.swReturnLink, pair: p)
            m.setLevel(MAKeys.swReturnLevels, channel: side == 0 ? l : r, value: v, linked: linked)
            if linked { s?.setLevels(m.element(MAKeys.swReturnLevels, l), m.element(MAKeys.swReturnLevels, r)) }
        }
        s.onAux = { side, v in m.setElement(MAKeys.auxSendLevels, 18 + (side == 0 ? l : r), v) }
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
        let layout = ctx.layout
        for (s, p) in inputStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            if let info = layout.inputStrips.first(where: { $0.pair == p }) { s.setStripEnabled(info.enabled) }
            s.setLevels(m.element(MAKeys.inputLevels, l), m.element(MAKeys.inputLevels, r))
            s.setAux(m.element(MAKeys.auxSendLevels, l), m.element(MAKeys.auxSendLevels, r))
            s.setPan(m.element(MAKeys.inputPan2, l), m.element(MAKeys.inputPan2, r))
            s.setFlags(link: m.pairFlag(MAKeys.inputLink, pair: p), mute: m.pairFlag(MAKeys.inputMute, pair: p), solo: m.pairFlag(MAKeys.inputSolo, pair: p))
            let c = ctx.ctrlState(group: .input, bit: MARotaryMask.inputPairBit(p)); s.setCtrl(enabled: c.enabled, on: c.on)
            s.setRoute(mask: m.element(MAKeys.inputRoutingV2, p))
        }
        for (s, p) in swStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            s.setLevels(m.element(MAKeys.swReturnLevels, l), m.element(MAKeys.swReturnLevels, r))
            s.setAux(m.element(MAKeys.auxSendLevels, 18 + l), m.element(MAKeys.auxSendLevels, 18 + r))
            s.setFlags(link: m.pairFlag(MAKeys.swReturnLink, pair: p), mute: m.pairFlag(MAKeys.swReturnMute, pair: p), solo: m.pairFlag(MAKeys.swReturnSolo, pair: p))
            let c = ctx.ctrlState(group: .swReturn, bit: MARotaryMask.swReturnPairBit(p)); s.setCtrl(enabled: c.enabled, on: c.on)
            s.setRoute(mask: m.element(MAKeys.swReturnRoutingV2, p))
        }
        updateMeters()
    }

    func updateMeters() {
        for (s, p) in inputStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            s.setMeters(ctx.db(PeakSlot.input(l)), ctx.db(PeakSlot.input(r)))
        }
        for (s, p) in swStrips {
            let (l, r) = DeviceModel.channels(ofPair: p)
            s.setMeters(ctx.db(PeakSlot.swReturn(l)), ctx.db(PeakSlot.swReturn(r)))
        }
        // Digital return bus meters: S/PDIF = SW-return slots 5, 6; ADAT = slots 7...14 (PeakSlot.swReturnADAT; was 5... = 2 strips early, found on hardware 2026-10-17).
        if let b = busStrip {
            let adat = ctx.layout.returnBus == .adat
            for i in 0..<b.meters.count { b.meters[i].db = ctx.db(adat ? PeakSlot.swReturnADAT(1 + i) : PeakSlot.swReturn(5 + i)) }
            b.setGreyed(from: ctx.layout.adatBusHalfGreyed ? 4 : b.meters.count)
        }
        buses?.setValues([[ctx.db(PeakSlot.output(1)), ctx.db(PeakSlot.output(2))], [ctx.db(PeakSlot.output(3)), ctx.db(PeakSlot.output(4))],
                          [ctx.db(PeakSlot.output(19)), ctx.db(PeakSlot.output(20))]])
    }
}
