// Advanced window, Monitor mode view (PANEL_REDESIGN_PLAN_v3 §3, v2 §4; phase M4): direct monitoring with up to three cue mixes, one per
// hardware bus (Mix 1/2, Mix 3/4, Aux). Left: the sources as mono strips (inputs) and stereo strips (DAW returns); their faders are the
// sources' levels in the selected mix. Middle: the cue-mix tiles (select, rename by double-click, MONO / STEREO badge). Right: the outputs
// (Line 1/2, Line 3/4, the phones) with the mix each plays, drawn as wires. Every hardware rule is in AdvancedMonitor.swift (MonitorMixer).
// Pan rule (user, 2026-10-27): every mono strip has a pan; greyed at centre for a mono mix, live for a stereo one (centre by default, full
// left / right possible); the Aux has none; amber when a stereo mix pans a source that a mono mix also uses (one pan per channel).
import AppKit

extension MixBus {
    var color: NSColor { switch self { case .mix12: return Theme.accent; case .mix34: return Theme.hex(0xB18CFF); case .aux: return Theme.green } }
}

/// Horizontal pan control: -255 ... 255, drag, double-click = centre. Greyed (disabled) or amber (shared-pan conflict).
final class PanBar: NSView {
    var value = 0 { didSet { if value != oldValue { needsDisplay = true } } }
    var isEnabled = true { didSet { needsDisplay = true } }
    var conflict = false { didSet { needsDisplay = true } }
    var onChange: ((Int) -> Void)?
    var onCommit: (() -> Void)?
    override var isFlipped: Bool { return true }
    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled || conflict ? 1 : 0.3) {
            let y = bounds.midY
            Theme.fill(NSRect(x: 0, y: y - 1, width: bounds.width, height: 2), radius: 1, Theme.track)
            Theme.text4.setFill(); NSRect(x: bounds.midX - 0.5, y: y - 4, width: 1, height: 8).fill()
            let f: CGFloat = CGFloat(value + 255) / 510.0
            let span: CGFloat = bounds.width - 8
            let x: CGFloat = f * span + 4
            Theme.fill(NSRect(x: x - 4, y: y - 6, width: 8, height: 12), radius: 2, conflict ? Theme.solo : Theme.text2)
        }
    }
    private func set(_ e: NSEvent) {
        let x = convert(e.locationInWindow, from: nil).x
        let span: CGFloat = max(1, bounds.width - 8)
        let f: CGFloat = (x - 4) / span
        var v = Int((f * 510 - 255).rounded())
        if abs(v) < 16 { v = 0 }                                  // a detent at centre
        v = max(-255, min(255, v))
        if v != value { value = v; onChange?(v) }
    }
    override func mouseDown(with e: NSEvent) {
        guard isEnabled else { return }
        if e.clickCount == 2 { value = 0; onChange?(0); onCommit?(); return }
        set(e)
    }
    override func mouseDragged(with e: NSEvent) { if isEnabled { set(e) } }
    override func mouseUp(with e: NSEvent) { if isEnabled { onCommit?() } }
}

/// One source strip: name, jack, IN, fader with its meter(s), value, pan (inputs), mute.
final class MonitorStrip: FlippedView {
    let source: MonitorSource
    let inButton = PillButton("IN"), mute = PillButton("M"), solo = PillButton("S")
    let fader = Fader()
    private let bars: [MeterBar]
    let slots: [Int]
    private let nameLabel = makeLabel("", size: 10.5, weight: .semibold, color: Theme.text, align: .center)
    private let jack = makeLabel("", size: 8.5, color: Theme.text3, align: .center)
    private let valueLabel = makeLabel("", size: 9.5, color: Theme.text2, align: .center)
    let pan = PanBar()
    private let panText = makeLabel("", size: 8.5, color: Theme.text3, align: .center)
    var member = true { didSet { alphaValue = member ? 1 : 0.42 } }
    var shared = false { didSet { needsDisplay = true } }
    var onRename: (() -> Void)?
    var contextMenu: (() -> NSMenu?)?
    override func menu(for event: NSEvent) -> NSMenu? { return contextMenu?() }

    init(source: MonitorSource, name: String, jack j: String, slots: [Int]) {
        self.source = source; self.slots = slots
        bars = slots.map { _ in let b = MeterBar(frame: .zero); b.showsClip = false; b.surface = Theme.well; return b }
        super.init(frame: .zero)
        nameLabel.stringValue = name; nameLabel.lineBreakMode = .byTruncatingTail; nameLabel.toolTip = name + " \u{2014} double-click to rename"
        jack.stringValue = j
        valueLabel.font = Theme.mono(9.5)
        inButton.solid = true; inButton.fontSize = 9
        mute.solid = true; mute.onColor = Theme.mute; mute.fontSize = 9
        solo.solid = true; solo.onColor = Theme.solo; solo.fontSize = 9
        solo.toolTip = "Solo (the interface mixer's solo: only soloed sources are heard on the mixes)"
        [nameLabel, jack, inButton, fader, valueLabel, mute, solo].forEach { addSubview($0) }
        bars.forEach { addSubview($0) }
        if source.kind == .input && !source.isStereoInput { addSubview(pan); addSubview(panText) }
        else { addSubview(panText); panText.stringValue = source.isStereoInput ? "stereo \u{00B7} L / R" : "stereo \u{00B7} no pan" }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    func setName(_ n: String) { nameLabel.stringValue = n }
    func showValue(_ v: Int) { valueLabel.stringValue = Theme.levelText(v) }
    /// Mono line or Aux: no pan control; the text says where the source goes ("Out 3", "Out 3 + 4")
    func showRoute(_ t: String) { pan.isHidden = true; panText.stringValue = t; panText.textColor = Theme.text3 }
    func showPan(_ v: Int, enabled: Bool, conflict: Bool) {
        pan.isHidden = false
        pan.value = enabled || conflict ? v : 0; pan.isEnabled = enabled; pan.conflict = conflict
        panText.stringValue = enabled || conflict ? Theme.panText(v) : "C"
        panText.textColor = conflict ? Theme.solo : Theme.text3
    }
    func updateMeters(_ db: (Int) -> Int) { for (i, b) in bars.enumerated() { b.db = db(slots[i]) } }

    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        let w = s.width
        nameLabel.frame = NSRect(x: 2, y: 8, width: w - 4, height: 14)
        jack.frame = NSRect(x: 2, y: 22, width: w - 4, height: 12)
        inButton.frame = NSRect(x: 5, y: 38, width: w - 10, height: 18)
        let fTop: CGFloat = 62, fBottom = s.height - 72
        let mw: CGFloat = CGFloat(bars.count) * 5 + CGFloat(bars.count - 1) * 2
        let gx = ((w - (20 + 4 + mw)) / 2).rounded()
        fader.frame = NSRect(x: gx, y: fTop, width: 20, height: max(40, fBottom - fTop))
        for (i, b) in bars.enumerated() { b.frame = NSRect(x: gx + 24 + CGFloat(i) * 7, y: fTop + Fader.trackPad, width: 5, height: max(30, fBottom - fTop - 2 * Fader.trackPad)) }
        valueLabel.frame = NSRect(x: 0, y: fBottom + 4, width: w, height: 13)
        pan.frame = NSRect(x: 6, y: fBottom + 20, width: w - 12, height: 14)
        panText.frame = NSRect(x: 0, y: fBottom + (source.kind == .input && !source.isStereoInput ? 34 : 24), width: w, height: 12)
        let bw = min(22, ((w - 8) / 2).rounded(.down))                   // M and S side by side, narrower on a 30 px strip
        mute.frame = NSRect(x: (w / 2 - 1 - bw).rounded(), y: s.height - 26, width: bw, height: 18)
        solo.frame = NSRect(x: (w / 2 + 1).rounded(), y: s.height - 26, width: bw, height: 18)
    }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 8, Theme.surface)
        if shared { Theme.solo.setFill(); NSBezierPath(ovalIn: NSRect(x: bounds.width - 10, y: 4, width: 6, height: 6)).fill() }
    }
    override func mouseDown(with e: NSEvent) {
        if e.clickCount == 2, convert(e.locationInWindow, from: nil).y < 36 { onRename?() }
    }
}

/// A mix tile: one line — a stereo bus, one mono line of a bus (its output, "MONO · OUT 3"), or the Aux — with its name, badge (click:
/// stereo <-> two mono lines), destinations and meters.
final class MixTile: FlippedView {
    let line: MixLine
    var bus: MixBus { return line.bus }
    var name = "" { didSet { needsDisplay = true } }
    var mono = true { didSet { needsDisplay = true } }
    var detail = "" { didSet { needsDisplay = true } }
    var selected = false { didSet { needsDisplay = true } }
    let bars = [MeterBar(frame: .zero), MeterBar(frame: .zero)]
    var onSelect: (() -> Void)?, onBadge: (() -> Void)?, onRename: (() -> Void)?
    private var badgeRect = NSRect.zero

    init(line: MixLine) {
        self.line = line
        super.init(frame: .zero)
        bars.forEach { $0.horizontal = true; $0.showsClip = false; $0.surface = Theme.well; addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        bars[0].frame = NSRect(x: 10, y: s.height - 18, width: s.width - 20, height: 4)
        bars[1].frame = NSRect(x: 10, y: s.height - 12, width: s.width - 20, height: 4)
    }
    override func draw(_ dirtyRect: NSRect) {
        let c = bus.color
        Theme.fill(bounds, radius: 10, selected ? c.blended(withFraction: 0.89, of: Theme.surface) ?? Theme.surface : Theme.surface)
        if selected {
            c.setStroke(); let p = NSBezierPath(roundedRect: bounds.insetBy(dx: 0.75, dy: 0.75), xRadius: 10, yRadius: 10); p.lineWidth = 1.5; p.stroke()
        }
        c.setFill(); NSBezierPath(ovalIn: NSRect(x: 10, y: 13, width: 8, height: 8)).fill()
        let b = line.side != nil ? "MONO \u{00B7} " + line.output.uppercased() : (mono ? "MONO" : "STEREO")
        let bw = Theme.textWidth(b, font: Theme.font(8.5, .bold), kern: 0.6) + 10
        badgeRect = NSRect(x: bounds.width - 10 - bw, y: 10, width: bw, height: 15)
        Theme.draw(name, in: NSRect(x: 24, y: 8, width: badgeRect.minX - 30, height: 18), font: Theme.font(12, .semibold), color: Theme.text, align: .left)
        Theme.fill(badgeRect, radius: 4, mono ? Theme.hex(0x2A2F37) : Theme.control)
        Theme.draw(b, in: badgeRect, font: Theme.font(8.5, .bold), color: mono ? Theme.hex(0xCFE3FF) : Theme.text2, kern: 0.6)
        Theme.draw(detail, in: NSRect(x: 10, y: 30, width: bounds.width - 20, height: 14), font: Theme.font(10), color: Theme.text3, align: .left)
    }
    override func mouseDown(with e: NSEvent) {
        let p = convert(e.locationInWindow, from: nil)
        if badgeRect.contains(p) { onBadge?(); return }
        if e.clickCount == 2 { onRename?(); return }
        onSelect?()
    }
}

/// An output card: name, the mix it plays (segmented), level with its meter, mute, note.
final class MonitorOutputCard: FlippedView {
    let title: String, sub: String
    let buses: [MixBus]
    let source: SegmentedBar
    let fader = HMeterFader(frame: .zero)
    let mute = PillButton("M")
    private let value = makeLabel("", size: 11, color: Theme.text2, align: .right)
    var note = "" { didSet { needsDisplay = true } }
    var slots: (Int, Int) = (-1, -1)
    /// A small control at the top right (Phones 1: the A/B switch menu)
    var accessory: NSView? { didSet { oldValue?.removeFromSuperview(); if let a = accessory { addSubview(a) }; setFrameSize(frame.size) } }

    init(title: String, sub: String, buses: [MixBus], names: [String], law: FaderLaw) {
        self.title = title; self.sub = sub; self.buses = buses
        source = SegmentedBar(items: names.map { .text($0) }, segmentWidth: buses.count == 2 ? 104 : 70)
        super.init(frame: .zero)
        fader.law = law
        value.font = Theme.mono(11)
        mute.solid = true; mute.onColor = Theme.mute
        [source, fader, value, mute].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    func showValue(_ v: Int) { value.stringValue = Theme.levelText(v, law: fader.law) }
    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        source.setFrameOrigin(NSPoint(x: 12, y: 30))
        fader.frame = NSRect(x: 12, y: 68, width: s.width - 24 - 46 - 26, height: 22)
        value.frame = NSRect(x: s.width - 12 - 26 - 46, y: 71, width: 42, height: 16)
        mute.frame = NSRect(x: s.width - 12 - 22, y: 69, width: 22, height: 20)
        fader.placeCap()
        if let a = accessory { a.frame = NSRect(x: s.width - 12 - 64, y: 5, width: 64, height: 20) }
    }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 10, Theme.surface)
        let tw = Theme.textWidth(title, font: Theme.font(12, .semibold))
        Theme.draw(title, in: NSRect(x: 12, y: 8, width: tw + 2, height: 16), font: Theme.font(12, .semibold), color: Theme.text, align: .left)
        Theme.draw(sub, in: NSRect(x: 18 + tw, y: 8, width: bounds.width - tw - 30, height: 16), font: Theme.font(10), color: Theme.text3, align: .left)
        if !note.isEmpty { Theme.draw(note, in: NSRect(x: 12, y: 94, width: bounds.width - 24, height: 14), font: Theme.font(10), color: Theme.text3, align: .left) }
    }
}

/// Wires from the mix tiles to the outputs they feed (static: redrawn when a source changes).
final class WiresView: FlippedView {
    var wires: [(from: NSRect, to: NSRect, color: NSColor, hot: Bool)] = [] { didSet { needsDisplay = true } }
    override var isOpaque: Bool { return false }
    override func hitTest(_ point: NSPoint) -> NSView? { return nil }
    override func draw(_ dirtyRect: NSRect) {
        for w in wires {
            let a = NSPoint(x: w.from.maxX, y: w.from.midY), b = NSPoint(x: w.to.minX, y: w.to.minY + 16)
            let dx = max(14, (b.x - a.x) * 0.6)
            let p = NSBezierPath(); p.move(to: a); p.curve(to: b, controlPoint1: NSPoint(x: a.x + dx, y: a.y), controlPoint2: NSPoint(x: b.x - dx, y: b.y))
            p.lineWidth = w.hot ? 2.2 : 1.5
            w.color.withAlphaComponent(w.hot ? 0.95 : 0.4).setStroke(); p.stroke()
            w.color.setFill(); NSBezierPath(ovalIn: NSRect(x: b.x - 2.6, y: b.y - 2.6, width: 5.2, height: 5.2)).fill()
        }
    }
}

final class AdvancedMonitorView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let ctx: PanelContext
    private let store: AdvancedStore
    private var selected: MixBus = .mix34
    private var side = MixSide.left                 // which mono line of a mono bus the strips show
    /// the line the strips show: a stereo bus, the Aux, or one mono line of a bus
    private var line: MixLine { return MixLine(bus: selected, side: mix(selected).mono && selected != .aux ? side : nil) }
    private let newMix = FlatButton("\u{FF0B} New mix")
    private let context = FlippedView(frame: .zero)
    private let contextLabel = makeLabel("", size: 11.5, color: Theme.text2)
    private let contextName = makeLabel("", size: 12.5, weight: .semibold, color: Theme.text)
    private let scroll = NSScrollView(frame: .zero)
    private let strips = FlippedView(frame: .zero)
    private var stripViews: [MonitorStrip] = []
    private var groupLabels: [(SectionLabel, CGFloat)] = []
    /// The source groups in order: title, strips, stereo (DAW returns are wider); nil strips = the ADAT fold button's slot
    private var groupList: [(title: String, views: [MonitorStrip]?, stereo: Bool)] = []
    private var tiles: [MixTile] = []
    private var cards: [(card: MonitorOutputCard, id: String)] = []
    private let wires = WiresView(frame: .zero)
    private let hint = makeLabel("", size: 10.5, color: Theme.text3)
    private let tilesLabel = SectionLabel("Cue mixes"), sourcesLabel = SectionLabel(""), outputsLabel = SectionLabel("Outputs")
    private var layoutKey: ChannelLayout?
    private var channels = AdvancedChannels.none
    private var noteTimer: Timer?

    init(context c: PanelContext, store: AdvancedStore) {
        ctx = c; self.store = store
        super.init(frame: .zero)
        scroll.drawsBackground = false
        scroll.hasHorizontalScroller = false            // no scroll bar (user, 2026-10-27): the strips fit the width (placeStrips)
        scroll.hasVerticalScroller = false
        scroll.documentView = strips
        context.wantsLayer = true; context.layer?.cornerRadius = 9; context.layer?.backgroundColor = Theme.surface.cgColor
        contextLabel.lineBreakMode = .byTruncatingTail
        [contextName, contextLabel].forEach { context.addSubview($0) }
        hint.maximumNumberOfLines = 14; hint.lineBreakMode = .byWordWrapping; hint.cell?.wraps = true
        hint.stringValue = "MONO = two independent lines (Out 1 / 2, Out 3 / 4); the panel pans each source to its lines. One level and pan per input for Mix 1/2 and 3/4 (amber dot). Inputs reach them in pairs: a partner in the other bus comes along (amber IN). Right-click: link a pair as stereo."
        [wires, context, scroll, tilesLabel, outputsLabel, hint, newMix].forEach { addSubview($0) }
        newMix.toolTip = "The FW1814 has three mix buses (Mix 1/2, Mix 3/4, Aux); a stereo one becomes two mono lines"
        newMix.onAction = { [weak self] in self?.newMixMenu() }
        rebuildTiles()
        rebuildCards()
        rebuild()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private var mixes: [MonitorMix] { return store.monitorMixes }
    /// A bus on an output card's source button: its name, or both line names, or "Out 3/4" when they do not fit the 70 pt button
    static func cardName(_ ms: [MonitorMix], _ b: MixBus) -> String {
        guard let m = ms.first(where: { $0.bus == b }) else { return b.label }
        guard m.mono && b != .aux else { return m.name }
        let both = m.name + "/" + m.rightName
        return Theme.textWidth(both, font: Theme.font(12, .semibold)) <= 62 ? both : (b == .mix12 ? "Out 1/2" : "Out 3/4")
    }
    /// Input pairs linked as one stereo source (panel-side, AdvancedStore.stereoInputs), by first channel
    private var linkedPairs: [Int] { return store.stereoInputs }
    private var builtLinks: [Int] = [], builtReturns: [Int] = []
    /// One tile per line: a stereo bus, each mono line of a mono bus, the Aux
    private func rebuildTiles() {
        tiles.forEach { $0.removeFromSuperview() }
        tiles = mixes.flatMap { $0.lines }.map { l -> MixTile in
            let t = MixTile(line: l)
            t.onSelect = { [weak self] in self?.selected = l.bus; self?.side = l.side ?? .left; self?.refresh() }
            t.onBadge = { [weak self] in self?.toggleMono(l.bus) }
            t.onRename = { [weak self] in self?.renameMix(l) }
            addSubview(t)
            return t
        }
        needsLayout = true
    }
    /// Where a source sits in the Aux (a stereo pair with no pan): its fixed side, or both for a stereo source
    /// Demo snapshot: the Aux selected with In 1-4 and DAW 1-2 in it, In 3 muted there (and In 1 soloed there with `solo`)
    func demoAux(solo: Bool = false) {
        selected = .aux; side = .left
        let ms = mixes
        apply { mx in
            for s in [MonitorSource.input(1), .input(2), .input(3), .input(4), .dawMono(1), .dawMono(2)] { mx.setMember(s, .aux, true, mixes: ms) }
            mx.setAuxMute(.input(3), true)
            if solo { mx.setAuxSolo(.input(1), true) }
        }
    }
    private func auxRoute(_ s: MonitorSource) -> String {
        guard let side = s.auxSide else { return "Aux L / R" }
        return side == .left ? "Aux L" : "Aux R"
    }
    private func mix(_ b: MixBus) -> MonitorMix { return mixes.first(where: { $0.bus == b }) ?? MonitorMix(bus: b, name: b.label, mono: true) }

    // MARK: sources

    private func rebuild() {
        let l = ctx.model.layout
        layoutKey = l
        channels = ctx.profile.advancedChannels(l)
        stripViews.forEach { $0.removeFromSuperview() }
        groupLabels.forEach { $0.0.removeFromSuperview() }
        stripViews = []; groupLabels = []
        let p = ctx.peaks
        // a hidden channel only leaves the view (view option per mode): its levels and routing in the mixes stay as they are
        let key = store.viewKey, hidden = store.hiddenIDs(key)
        let links = linkedPairs; builtLinks = links
        // inputs: the Core Audio channels map to mixer input channels by peak slot (input ch n = slot n - 1, PeakMap.input)
        func short(_ j: String) -> String { return j.replacingOccurrences(of: "Mic/Line ", with: "In ").replacingOccurrences(of: "Line ", with: "In ") }
        func strip(_ c: AdvancedChannel, with c2: AdvancedChannel? = nil) -> MonitorStrip {
            let ch = c.slot + 1
            let v: MonitorStrip
            if let b = c2 {
                v = MonitorStrip(source: .stereo(ch), name: store.name(c) + " + " + store.name(b), jack: short(c.jack) + "+" + String(short(b.jack).split(separator: " ").last ?? ""), slots: [c.slot, b.slot])
            } else {
                v = MonitorStrip(source: .input(ch), name: store.name(c), jack: short(c.jack), slots: [c.slot])
                v.onRename = { [weak self, weak v] in self?.rename(c) { v?.setName($0) } }
            }
            v.contextMenu = { [weak self] in
                guard let me = self else { return nil }
                let menu = hideChannelMenu(name: me.store.name(c), hiddenCount: hidden.count,
                                           hide: c2 == nil ? { me.store.setHidden(c.id, true, in: key); me.rebuild(); me.refresh() } : nil,
                                           showAll: { me.store.showAll(in: key); me.rebuild(); me.refresh() })
                menu.addItem(.separator())
                let a: MenuAction, title: String
                if c2 != nil {
                    title = "Split into two mono sources"
                    a = MenuAction { me.store.stereoInputs = me.store.stereoInputs.filter { $0 != ch }; me.rebuild(); me.refresh() }
                } else if ch % 2 == 1, let b = me.channels.inputs.first(where: { $0.slot == c.slot + 1 }) {
                    title = "Join \(me.store.name(c)) + \(me.store.name(b)) as one stereo source"
                    a = MenuAction {
                        let ms = me.mixes
                        me.store.stereoInputs = (me.store.stereoInputs + [ch]).sorted()
                        me.apply { $0.link(ch, mixes: ms) }                 // the partner takes the level, send and pan
                    }
                } else { return menu }
                let i = NSMenuItem(title: title, action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
                i.target = a; i.representedObject = a
                menu.addItem(i)
                return menu
            }
            return v
        }
        /// strips of a group: a joined pair (first channel odd, both shown) becomes one stereo strip
        func stripsOf(_ list: [AdvancedChannel]) -> [MonitorStrip] {
            var out: [MonitorStrip] = [], i = 0
            while i < list.count {
                let c = list[i], ch = c.slot + 1
                if ch % 2 == 1, links.contains(ch), i + 1 < list.count, list[i + 1].slot == c.slot + 1 {
                    out.append(strip(c, with: list[i + 1])); i += 2
                } else { out.append(strip(c)); i += 1 }
            }
            return out
        }
        let shown = channels.inputs.filter { !hidden.contains($0.id) }
        let analog = shown.filter { $0.group == "Analog" }, digital = shown.filter { $0.group != "Analog" }
        // every source on screen at once (user, 2026-10-28: no fold; with the mono DAW returns the strips fit the width, no scroll bar)
        let digitalTitle = channels.inputs.contains(where: { $0.group == "ADAT" }) ? "ADAT in" : "S/PDIF in"
        groupList = [("Analog in", stripsOf(analog), false), (digitalTitle, stripsOf(digital), false)]
        // DAW returns: mono by default (user, 2026-10-28), a pair joined as one stereo return on request (panel-side, AdvancedStore)
        func returnMenu(_ v: MonitorStrip, pair k: Int, joined: Bool) {
            v.contextMenu = { [weak self] in
                guard let me = self else { return nil }
                let menu = NSMenu(); menu.autoenablesItems = false
                let a = MenuAction {
                    me.store.stereoReturns = joined ? me.store.stereoReturns.filter { $0 != k } : (me.store.stereoReturns + [k]).sorted()
                    me.rebuild(); me.refresh()
                }
                let i = NSMenuItem(title: joined ? "Split into DAW \(2 * k - 1) and DAW \(2 * k) (mono returns)" : "Join DAW \(2 * k - 1) + DAW \(2 * k) as one stereo return",
                                   action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
                i.target = a; i.representedObject = a
                menu.addItem(i)
                return menu
            }
        }
        var daw: [MonitorStrip] = []
        for k in [1, 2] {
            if store.stereoReturns.contains(k) {
                let v = MonitorStrip(source: .daw(k), name: "DAW \(2 * k - 1)/\(2 * k)", jack: k == 1 ? "playback" : "click / cue", slots: [p.swReturn(2 * k - 1), p.swReturn(2 * k)])
                returnMenu(v, pair: k, joined: true); daw.append(v)
            } else {
                for ch in [2 * k - 1, 2 * k] {
                    let v = MonitorStrip(source: .dawMono(ch), name: "DAW \(ch)", jack: ch % 2 == 1 ? "lands left" : "lands right", slots: [p.swReturn(ch)])
                    returnMenu(v, pair: k, joined: false); daw.append(v)
                }
            }
        }
        builtReturns = store.stereoReturns
        groupList.append(("DAW playback", daw, false))
        for g in groupList {
            guard let vs = g.views else { continue }
            let lab = SectionLabel(g.title); strips.addSubview(lab); groupLabels.append((lab, 0))
            for v in vs { wireStrip(v); stripViews.append(v); strips.addSubview(v) }
        }
        layoutAll()
    }

    private func wireStrip(_ v: MonitorStrip) {
        let src = v.source
        v.inButton.onClick = { [weak self] b in
            guard let me = self else { return }
            let l = me.line, ms = me.mixes
            me.apply { $0.setLineMember(src, l, b.isOn, mixes: ms) }
        }
        v.fader.onChange = { [weak self] val in
            guard let me = self else { return }
            let m = me.ctx.model
            if me.selected == .aux {
                var mx = MonitorMixer(m.cache, parked: me.store.parkedLevels)
                if mx.auxSilenced(src) { mx.setKeptAuxSend(src, val); me.store.parkedLevels = mx.parked }    // M / another's S: the send stays cut
                else { for ch in src.channels { m.setElement(MAKeys.auxSendLevels, src.kind == .input ? ch : m.ints(MAKeys.inputLevels).count + ch, val) } }
            } else {
                for ch in src.channels { m.setElement(src.kind == .input ? MAKeys.inputLevels : MAKeys.swReturnLevels, ch, val) }
            }
            v.showValue(val)
        }
        v.fader.onCommit = { [weak self] in self?.ctx.model.commit() }
        v.pan.onChange = { [weak self] val in self?.ctx.model.setElement(MAKeys.inputPan2, src.channels[0], val) }
        v.pan.onCommit = { [weak self] in self?.ctx.model.commit() }
        v.mute.onClick = { [weak self] b in
            guard let me = self else { return }
            // in the Aux, M cuts this source's Aux send only (user, 2026-10-28); elsewhere it is the channel's mute in the interface mixer
            if me.selected == .aux { me.apply { $0.setAuxMute(src, b.isOn) }; return }
            let m = me.ctx.model
            for ch in src.channels { m.setElement(src.kind == .input ? MAKeys.inputMute : MAKeys.swReturnMute, ch, b.isOn ? 1 : 0) }
            m.commit()
        }
        v.solo.onClick = { [weak self] b in
            guard let me = self else { return }
            // in the Aux, S solos inside the Aux only (user, 2026-10-28); elsewhere it is the interface mixer's solo
            if me.selected == .aux { me.apply { $0.setAuxSolo(src, b.isOn) }; return }
            let m = me.ctx.model
            for ch in src.channels { m.setElement(src.kind == .input ? MAKeys.inputSolo : MAKeys.swReturnSolo, ch, b.isOn ? 1 : 0) }
            m.commit()
        }
    }

    private func wire(_ card: MonitorOutputCard, _ id: String) {
        guard let sp = specs[id] else { return }
        card.source.onSelect = { [weak self] i in
            guard let m = self?.ctx.model else { return }
            let bus = card.buses[i]
            if let pair = sp.pair { m.setMonitorAux(pair: pair, aux: bus == .aux); self?.refresh() }
            else if let ph = sp.phones { m.setPhonesSource(phones: ph, mask: UInt32(bus.phonesMask)) }
        }
        let key = sp.phones != nil ? MAKeys.headphoneLevels : MAKeys.outputLevels
        let ch = sp.channels[0], single = sp.channels.count == 1
        // a mono line's output has its own level (not linked to its neighbour)
        card.fader.onChange = { [weak self, weak card] v in self?.ctx.model.setLevel(key, channel: ch, value: v, linked: !single); card?.showValue(v) }
        card.fader.onCommit = { [weak self] in self?.ctx.model.commit() }
        let muteKey = sp.phones != nil ? MAKeys.headphoneMute : MAKeys.outputMute
        card.mute.onClick = { [weak self] b in
            guard let m = self?.ctx.model else { return }
            for c in sp.channels { m.setElement(muteKey, c, b.isOn ? 1 : 0) }
            m.commit()
        }
    }

    /// Runs MonitorMixer on the current mixer and writes what it changed in one batch; its notes go to the context bar.
    private func apply(_ f: (inout MonitorMixer) -> Void) {
        var mx = MonitorMixer(ctx.model.cache, parked: store.parkedLevels)
        f(&mx)
        if mx.parked != store.parkedLevels { store.parkedLevels = mx.parked }
        let w = mx.writes
        if !w.isEmpty { ctx.model.restore(w) }
        if let n = mx.notes.last { showNote(n) }
        refresh()
    }

    /// STEREO <-> MONO (two independent lines). Nothing is written when the lines appear: each source's pan already says which lines it is
    /// on (hard left = the odd line, hard right = the even line, anything else = both). Back to stereo, linked pairs spread hard L / R.
    private func toggleMono(_ bus: MixBus) {
        guard bus != .aux else { showNote("The Aux is a stereo pair with no pan: odd channels on Aux L, even on Aux R (hardware)."); return }
        var ms = store.monitorMixes
        guard let i = ms.index(where: { $0.bus == bus }) else { return }
        ms[i].mono = !ms[i].mono
        store.monitorMixes = ms
        // the bus's output pair: independent levels as two mono lines (the standard mixer's link off), linked again as one stereo mix
        let pair = bus == .mix12 ? 1 : 2, m = ctx.model
        m.setPairFlag(MAKeys.outputLink, pair: pair, on: !ms[i].mono)
        if !ms[i].mono { m.setElement(MAKeys.outputLevels, 2 * pair, m.element(MAKeys.outputLevels, 2 * pair - 1)) }
        m.commit()
        selected = bus; side = .left
        rebuildTiles(); relabelCards()
        if ms[i].mono { refresh() } else { let pairs = linkedPairs; apply { $0.spread(bus, stereoPairs: pairs) } }
    }

    private func ask(_ title: String, current: String, placeholder: String, done: @escaping (String) -> Void) {
        guard let w = window else { return }
        let a = NSAlert()
        a.messageText = title
        a.informativeText = "Kept by the panel for this interface. Leave empty for \u{201C}\(placeholder)\u{201D}."
        let f = NSTextField(frame: NSRect(x: 0, y: 0, width: 240, height: 24)); f.stringValue = current
        a.accessoryView = f; a.window.initialFirstResponder = f
        a.addButton(withTitle: "Rename"); a.addButton(withTitle: "Cancel")
        a.beginSheetModal(for: w) { r in if r == .alertFirstButtonReturn { done(f.stringValue) } }
    }
    private func rename(_ c: AdvancedChannel, done: @escaping (String) -> Void) {
        ask("Name for \(c.jack)", current: store.name(c), placeholder: c.name) { [weak self] t in
            guard let me = self else { return }
            me.store.setName(t, for: c.id); done(me.store.name(c))
        }
    }
    private func renameMix(_ l: MixLine) {
        let bus = l.bus, right = l.side == .right
        let d = MonitorMix.defaults.first(where: { $0.bus == bus })
        let def = (right ? d?.rightName : d?.name) ?? bus.label
        ask("Name for \(l.side == nil ? "the mix on " + bus.label : l.output)", current: mix(bus).name(l), placeholder: def) { [weak self] t in
            guard let me = self else { return }
            var ms = me.store.monitorMixes
            if let i = ms.index(where: { $0.bus == bus }) {
                let n = t.trimmingCharacters(in: .whitespacesAndNewlines)
                let v = n.isEmpty ? def : String(n.prefix(18))
                if right { ms[i].rightName = v } else { ms[i].name = v }
            }
            me.store.monitorMixes = ms
            me.relabelCards(); me.refresh()
        }
    }
    private func relabelCards() { rebuildCards(); layoutAll() }

    /// An output card: Line 1/2 and Line 3/4 as one stereo card, or — when their bus is two mono lines (user, 2026-10-28) — one card per
    /// output (Line 3, Line 4) with its own level and mute; then the phones.
    private struct CardSpec { let id: String; let title: String; let sub: String; let buses: [MixBus]; let channels: [Int]; let pair: Int?; let phones: Int? }
    private var specs: [String: CardSpec] = [:]
    private func cardSpecs() -> [CardSpec] {
        var out: [CardSpec] = []
        for (pair, bus, sub) in [(1, MixBus.mix12, "speakers"), (2, MixBus.mix34, "wedges \u{00B7} mono outs")] {
            let m = mix(bus), c0 = 2 * pair - 1
            if m.mono {
                out.append(CardSpec(id: "l\(c0)", title: "Line \(c0)", sub: m.name, buses: [bus, .aux], channels: [c0], pair: pair, phones: nil))
                out.append(CardSpec(id: "l\(c0 + 1)", title: "Line \(c0 + 1)", sub: m.rightName, buses: [bus, .aux], channels: [c0 + 1], pair: pair, phones: nil))
            } else {
                out.append(CardSpec(id: "l\(c0)\(c0 + 1)", title: "Line \(c0)/\(c0 + 1)", sub: sub, buses: [bus, .aux], channels: [c0, c0 + 1], pair: pair, phones: nil))
            }
        }
        out.append(CardSpec(id: "hp1", title: "Phones 1", sub: "", buses: MixBus.all, channels: [1, 2], pair: nil, phones: 1))
        out.append(CardSpec(id: "hp2", title: "Phones 2", sub: "", buses: MixBus.all, channels: [3, 4], pair: nil, phones: 2))
        return out
    }
    private func rebuildCards() {
        cards.forEach { $0.card.removeFromSuperview() }
        cards = []; specs = [:]
        for sp in cardSpecs() {
            // a mono line's card offers its own line (by name) or the Aux; the Aux choice is per output pair in the hardware
            let names = sp.buses.map { b -> String in
                if sp.channels.count == 1 && b != .aux { let m = mix(b); return sp.channels[0] % 2 == 1 ? m.name : m.rightName }
                return AdvancedMonitorView.cardName(mixes, b)
            }
            let card = MonitorOutputCard(title: sp.title, sub: sp.sub, buses: sp.buses, names: names, law: sp.phones == nil ? .console : .monitor)
            specs[sp.id] = sp
            wire(card, sp.id)
            cards.append((card, sp.id)); addSubview(card)
        }
        attachAB()
        needsLayout = true
    }

    // MARK: ＋ New mix: three hardware buses (Mix 1/2, Mix 3/4, Aux); a stereo one becomes two mono lines (user, 2026-10-28)

    private func newMixMenu() {
        let menu = NSMenu(); menu.autoenablesItems = false
        let n = mixes.reduce(0) { $0 + $1.lines.count }
        let info = NSMenuItem(title: "\(n) of 5 mixes: Mix 1/2 and Mix 3/4 are stereo or two mono lines each, plus the Aux", action: nil, keyEquivalent: "")
        info.isEnabled = false
        menu.addItem(info); menu.addItem(.separator())
        for b in [MixBus.mix12, .mix34] {
            let mm = mix(b), outs = b == .mix12 ? "Out 1 / Out 2" : "Out 3 / Out 4"
            let a = MenuAction { [weak self] in self?.toggleMono(b) }
            let i = NSMenuItem(title: mm.mono ? "Make \(b.label) one stereo mix" : "Make \(b.label) two mono lines (\(outs))",
                               action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
            i.target = a; i.representedObject = a
            menu.addItem(i)
        }
        menu.popUp(positioning: nil, at: NSPoint(x: 0, y: newMix.bounds.height + 4), in: newMix)
    }

    // MARK: the front A/B switch (Phones 1), as the standard panel's Hardware tab: its function and its A / B sources

    private func attachAB() {
        guard ctx.profile.hardware.abSwitch, let card = cards.first(where: { $0.id == "hp1" })?.card else { return }
        let b = FlatButton("A/B \u{25BE}")
        b.toolTip = "The front A/B switch: phones A/B sources or direct monitoring"
        b.onAction = { [weak self, weak b] in if let me = self, let v = b { me.abMenu(from: v) } }
        card.accessory = b
    }
    private func abMenu(from v: NSView) {
        let m = ctx.model
        let dm = (m.number(MAKeys.momentarySwitchSource) ?? 0) != 0
        let menu = NSMenu()
        func item(_ t: String, on: Bool, _ act: @escaping () -> Void) {
            let i = NSMenuItem(title: t, action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
            let a = MenuAction(act); i.target = a; i.representedObject = a; i.state = on ? .on : .off
            menu.addItem(i)
        }
        item("A/B switch: Phones 1 A / B sources", on: !dm) { [weak self] in m.setSwitchFunction(directMonitoring: false); self?.refresh() }
        item("A/B switch: direct monitoring", on: dm) { [weak self] in m.setSwitchFunction(directMonitoring: true); self?.refresh() }
        menu.addItem(.separator())
        item("A and B sources\u{2026}", on: false) { [weak self] in self?.abSheet() }
        menu.items.last?.isEnabled = !dm
        menu.autoenablesItems = false
        menu.popUp(positioning: nil, at: NSPoint(x: 0, y: v.bounds.height + 4), in: v)
    }
    private func abSheet() {
        guard let w = window else { return }
        let routes = ctx.model.ints(MAKeys.momentarySwitchRoutings), all = ctx.profile.phonesSources
        func source(_ i: Int) -> UInt32? { return routes.count > i ? all.first { Int($0.mask) == routes[i] }?.mask : nil }
        let s = HPABSheet(a: source(0), b: source(1), sources: all, name: ctx.profile.hardware.abSheetName)
        abSheetRef = s
        s.onSet = { [weak self] a, b in self?.ctx.model.setABSources(a: a, b: b); self?.refresh() }
        w.beginSheet(s.panel) { [weak self] _ in self?.abSheetRef = nil }
    }
    private var abSheetRef: HPABSheet?
    /// "A/B: A = Mix 1/2, B = Mix 3/4" or "A/B: direct monitoring" (Phones 1 card)
    private var abNote: String {
        let m = ctx.model
        if (m.number(MAKeys.momentarySwitchSource) ?? 0) != 0 { return "front A/B switch: direct monitoring" }
        let r = m.ints(MAKeys.momentarySwitchRoutings)
        func name(_ i: Int) -> String { return r.count > i ? ctx.profile.phonesSource(mask: r[i])?.short ?? "-" : "-" }
        return "front A/B: A = \(name(0)), B = \(name(1))"
    }

    private func showNote(_ n: String) {
        contextLabel.stringValue = n; contextLabel.textColor = Theme.solo
        noteTimer?.invalidate()
        noteTimer = Timer.scheduledTimer(withTimeInterval: 7, repeats: false) { [weak self] _ in self?.contextLabel.textColor = Theme.text2; self?.refresh() }
    }

    // MARK: layout

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutAll() }
    private func layoutAll() {
        let p = Theme.pad, w = bounds.width, h = bounds.height
        let outW: CGFloat = 248, mixW: CGFloat = 178, wireGap: CGFloat = 34
        let outX = w - p - outW, mixX = outX - wireGap - mixW, srcW = mixX - p - 16
        context.frame = NSRect(x: p, y: p, width: srcW, height: 32)
        contextName.frame = NSRect(x: 26, y: 8, width: 150, height: 16)
        contextLabel.frame = NSRect(x: 26 + Theme.textWidth(contextName.stringValue, font: Theme.font(12.5, .semibold)) + 10, y: 8.5, width: srcW - 200, height: 16)
        scroll.frame = NSRect(x: p, y: p + 32 + 10, width: srcW, height: h - p - 42 - p)
        placeStrips(width: srcW, height: scroll.frame.height)
        tilesLabel.frame = NSRect(x: mixX, y: p + 8, width: mixW, height: 14)
        let pitch: CGFloat = tiles.count > 3 ? 70 : 80, tilesEnd = p + 28 + CGFloat(tiles.count) * pitch
        for (i, t) in tiles.enumerated() { t.frame = NSRect(x: mixX, y: p + 28 + CGFloat(i) * pitch, width: mixW, height: pitch - 8) }
        newMix.frame = NSRect(x: mixX, y: tilesEnd, width: mixW, height: 26)
        hint.frame = NSRect(x: mixX, y: tilesEnd + 36, width: mixW, height: max(60, h - p - tilesEnd - 36))
        outputsLabel.frame = NSRect(x: outX, y: p + 8, width: outW, height: 14)
        let cardPitch = min(120, ((h - p - 28 - p) / CGFloat(max(1, cards.count))).rounded(.down))
        for (i, c) in cards.enumerated() { c.card.frame = NSRect(x: outX, y: p + 28 + CGFloat(i) * cardPitch, width: outW, height: cardPitch - 8) }
        wires.frame = bounds
        updateWires()
    }

    /// Lays the source groups out across `width`: mono strips 30-76 px, stereo ones 16 px wider, so every source fits the row (no scroll bar:
    /// 8 analog + 8 ADAT + 4 DAW returns are about 60 px each at full HD, 34 px at the 1280 pt minimum).
    private func placeStrips(width: CGFloat, height: CGFloat) {
        strips.frame = NSRect(x: 0, y: 0, width: width, height: height)
        let gap: CGFloat = 4, groupGap: CGFloat = 14
        // a stereo strip (a linked input pair, a joined DAW return pair) is 16 pt wider than a mono one
        let all = groupList.flatMap { $0.views ?? [] }
        let stereos = all.filter { $0.source.channels.count == 2 }.count, monos = all.count - stereos
        let fixed = CGFloat(max(0, groupList.count - 1)) * groupGap + CGFloat(max(0, monos + stereos - groupList.count)) * gap
            + CGFloat(stereos) * 16
        let w = max(30, min(76, ((width - fixed) / CGFloat(max(1, monos + stereos))).rounded(.down)))
        var x: CGFloat = 0, li = 0
        for g in groupList {
            guard let vs = g.views else { continue }
            groupLabels[li].0.frame = NSRect(x: x, y: 0, width: max(110, CGFloat(vs.count) * (w + gap)), height: 16); li += 1
            for v in vs { let sw = v.source.channels.count == 2 ? w + 16 : w; v.frame = NSRect(x: x, y: 20, width: sw, height: height - 20); x += sw + gap }
            x += groupGap - gap
        }
    }

    private func updateWires() {
        let m = ctx.model
        wires.wires = cards.compactMap { c in
            guard let sp = specs[c.id] else { return nil }
            let b: MixBus
            if let pair = sp.pair { b = m.monitorAux(pair: pair) ? .aux : (pair == 1 ? .mix12 : .mix34) }
            else if sp.phones == 1 { b = MixBus.all.first(where: { $0.phonesMask == Int(m.number(MAKeys.headphoneSourceV2) ?? 1) }) ?? .mix12 }
            else { b = MixBus.all.first(where: { $0.phonesMask == Int(m.number(MAKeys.headphoneSecondSource) ?? 2) }) ?? .mix34 }
            // a mono output's wire starts at its own line's tile (odd output = the left line)
            let side: MixSide? = sp.channels.count == 1 && b != .aux ? (sp.channels[0] % 2 == 1 ? .left : .right) : nil
            guard let t = tiles.first(where: { $0.bus == b && (side == nil || $0.line.side == side) }) else { return nil }
            return (t.frame, c.card.frame, b.color, b == selected && (side == nil || line.side == side))
        }
    }

    // MARK: model -> view

    func refresh() {
        let m = ctx.model
        if layoutKey != m.layout || builtLinks != linkedPairs || builtReturns != store.stereoReturns { rebuild() }
        let mx = MonitorMixer(m.cache, parked: store.parkedLevels), sel = mix(selected), l = line
        let stereoBus = l.side == nil && selected != .aux
        let other = selected.other
        for v in stripViews {
            let s = v.source
            v.member = mx.member(s, l)
            v.mute.toolTip = selected == .aux ? "Mute in the Aux: this source\u{2019}s Aux send to \u{2212}\u{221E} (Mix 1/2 and Mix 3/4 do not change)" : "Mute (the interface mixer\u{2019}s channel mute)"
            v.solo.toolTip = selected == .aux ? "Solo in the Aux: only soloed sources are heard in the Aux (Mix 1/2 and Mix 3/4 do not change)" : "Solo (the interface mixer\u{2019}s solo: only soloed sources are heard on the mixes)"
            let reach = mx.reachable(s, l)
            v.inButton.isEnabled = reach
            // a partner the pair routing brought in (it is in the other bus at a level, the pair is not linked): amber, with the reason
            let forced = s.kind == .input && mx.forcedPartner(s, selected)
            v.inButton.isOn = v.member; v.inButton.title = v.member ? "IN" : "OFF"
            v.inButton.onColor = forced ? Theme.solo : selected.color
            v.inButton.toolTip = forced ? "In with its pair: the interface routes to Mix 1/2 and Mix 3/4 in pairs and \(s.label) is in \(other?.label ?? "the other mix") (one level for both). Link the pair, or take it out of \(other?.label ?? "there"), to choose freely." : nil
            if !reach {
                v.inButton.toolTip = "\(s.label) always lands on the \(s.fixedSide == .left ? "left (Out 1 / Out 3)" : "right (Out 2 / Out 4)"): the interface has no pan for the DAW returns. To reach \(l.output), play it on \(l.side == .left ? "DAW 1 or DAW 3" : "DAW 2 or DAW 4") in the DAW."
            }
            v.fader.value = mx.send(s, selected); v.showValue(v.fader.value)
            v.shared = selected != .aux && mx.sharedLevel(s)
            if s.kind == .input {
                if stereoBus && !s.isStereoInput {
                    // stereo mix: the pan control; amber when the other bus's mono lines set this pan (shared)
                    let fromLines = other.map { mix($0).mono && mx.member(s, $0) } ?? false
                    v.showPan(mx.pan(s.channels[0]), enabled: true, conflict: fromLines)
                } else if l.side != nil {
                    // mono line: no pan control, where the source goes instead
                    let onL = mx.sideMember(s, selected, .left), onR = mx.sideMember(s, selected, .right)
                    let outs = [MixLine(bus: selected, side: .left), MixLine(bus: selected, side: .right)]
                    let t = onL && onR ? outs[0].output + " + " + String(outs[1].output.split(separator: " ").last ?? "") : (onL ? outs[0].output : (onR ? outs[1].output : ""))
                    v.showRoute(t)
                } else {
                    v.showRoute(selected == .aux ? auxRoute(s) : "stereo \u{00B7} L / R")
                }
                v.mute.isOn = selected == .aux ? mx.auxMuted(s) : m.element(MAKeys.inputMute, s.channels[0]) != 0
                v.solo.isOn = selected == .aux ? mx.auxSoloed(s) : m.element(MAKeys.inputSolo, s.channels[0]) != 0
            } else {
                if let f = s.fixedSide {
                    // a mono return: where it lands (its side, the line on a mono bus)
                    let t: String
                    if selected == .aux { t = auxRoute(s) }
                    else if l.side != nil { t = reach ? (v.member ? l.output : "\u{2192} " + l.output) : "not here" }
                    else { t = f == .left ? "left only" : "right only" }
                    v.showRoute(t)
                }
                v.mute.isOn = selected == .aux ? mx.auxMuted(s) : m.element(MAKeys.swReturnMute, s.channels[0]) != 0
                v.solo.isOn = selected == .aux ? mx.auxSoloed(s) : m.element(MAKeys.swReturnSolo, s.channels[0]) != 0
            }
        }
        // the context bar (unless a note is showing)
        contextName.stringValue = sel.name(l)
        if noteTimer?.isValid != true {
            let n = stripViews.filter { $0.member }.count
            if let side = l.side {
                let partner = MixLine(bus: selected, side: side.other)
                contextLabel.stringValue = "MONO \u{00B7} \(l.output): a source IN here only is panned to it; IN on \(l.output) and \(partner.output) = centre \u{00B7} one level per source for both lines \u{00B7} \(n) in"
            } else if selected == .aux {
                contextLabel.stringValue = "STEREO \u{00B7} the Aux: its own send per channel, no pan: odd channels on Aux L, even on Aux R \u{00B7} faders = each source\u{2019}s send \u{00B7} M / S act in the Aux only\(mx.auxSoloActive ? " (solo on: only soloed sources heard)" : "") \u{00B7} \(n) in"
            } else {
                contextLabel.stringValue = "STEREO \u{00B7} \(selected.label): pan each source \u{00B7} faders = each source\u{2019}s level (shared by Mix 1/2 and Mix 3/4) \u{00B7} \(n) in"
            }
        }
        (context.subviews.first(where: { $0 is ContextDot }) ?? { let d = ContextDot(); context.addSubview(d); return d }()).frame = NSRect(x: 12, y: 12, width: 8, height: 8)
        (context.subviews.first(where: { $0 is ContextDot }) as? ContextDot)?.color = selected.color
        // tiles
        for t in tiles {
            let mm = mix(t.bus)
            t.name = mm.name(t.line); t.mono = mm.mono && t.bus != .aux; t.selected = t.line == l
            var dests: [String] = []
            let lineOut: (Int) -> String = { pair in
                guard let s = t.line.side else { return pair == 1 ? "Line 1/2" : "Line 3/4" }
                return "Line \((pair == 1 ? 1 : 3) + (s == .right ? 1 : 0))"
            }
            if (t.bus == .mix12 && !m.monitorAux(pair: 1)) || (t.bus == .aux && m.monitorAux(pair: 1)) { dests.append(lineOut(1)) }
            if (t.bus == .mix34 && !m.monitorAux(pair: 2)) || (t.bus == .aux && m.monitorAux(pair: 2)) { dests.append(lineOut(2)) }
            let ear = t.line.side.map { $0 == .left ? " L" : " R" } ?? ""
            if Int(m.number(MAKeys.headphoneSourceV2) ?? 0) == t.bus.phonesMask { dests.append("Phones 1" + ear) }
            if Int(m.number(MAKeys.headphoneSecondSource) ?? 0) == t.bus.phonesMask { dests.append("Phones 2" + ear) }
            t.detail = (t.line.side == nil ? t.bus.label : t.bus.label + " " + (t.line.side == .left ? "L" : "R")) + " \u{2192} " + (dests.isEmpty ? "no output" : dests.joined(separator: ", "))
        }
        // output cards
        for c in cards {
            guard let sp = specs[c.id] else { continue }
            let card = c.card
            let busNow: MixBus
            if let pair = sp.pair { busNow = m.monitorAux(pair: pair) ? .aux : (pair == 1 ? .mix12 : .mix34) }
            else if sp.phones == 1 { busNow = MixBus.all.first(where: { $0.phonesMask == Int(m.number(MAKeys.headphoneSourceV2) ?? 1) }) ?? .mix12 }
            else { busNow = MixBus.all.first(where: { $0.phonesMask == Int(m.number(MAKeys.headphoneSecondSource) ?? 2) }) ?? .mix34 }
            card.source.selected = card.buses.index(of: busNow) ?? 0
            let key = sp.phones != nil ? MAKeys.headphoneLevels : MAKeys.outputLevels
            let ch = sp.channels[0]
            card.fader.value = m.faderLevel(key, channel: ch); card.fader.placeCap(); card.showValue(card.fader.value)
            card.mute.isOn = m.element(sp.phones != nil ? MAKeys.headphoneMute : MAKeys.outputMute, ch) != 0
            var notes: [String] = []
            let bm = mix(busNow)
            if sp.phones != nil && busNow != .aux && bm.mono { notes.append("\(bm.name) left \u{00B7} \(bm.rightName) right") }
            if sp.channels.count == 1 { notes.append("mono out \u{00B7} Aux: Line \(2 * (sp.pair ?? 1) - 1) + \(2 * (sp.pair ?? 1)) together") }
            if c.id == "hp1" && ctx.profile.hardware.abSwitch { notes.append(abNote) }
            card.note = notes.joined(separator: " \u{00B7} ")
            let p = ctx.peaks
            if sp.phones != nil { card.slots = (p.phones(ch), p.phones(ch + 1)) }
            else { card.slots = sp.channels.count == 1 ? (p.output(ch), p.output(ch)) : (p.output(ch), p.output(ch + 1)) }
        }
        contextLabel.toolTip = contextLabel.stringValue            // the whole text when the bar is too narrow for it
        updateWires()
    }

    func updateMeters() {
        let db = { [ctx] (s: Int) -> Int in ctx.db(s) }
        stripViews.forEach { $0.updateMeters(db) }
        let p = ctx.peaks
        for t in tiles {
            var s: (Int, Int) = t.bus == .mix12 ? (p.output(1), p.output(2)) : (t.bus == .mix34 ? (p.output(3), p.output(4)) : (p.auxOut(1), p.auxOut(2)))
            if let side = t.line.side { s = side == .left ? (s.0, s.0) : (s.1, s.1) }    // a mono line meters its own output
            t.bars[0].db = ctx.db(s.0); t.bars[1].db = ctx.db(s.1)
        }
        for c in cards { c.card.fader.bars[0].db = ctx.db(c.card.slots.0); c.card.fader.bars[1].db = ctx.db(c.card.slots.1) }
    }
}

/// The selected mix's colour dot in the context bar.
final class ContextDot: NSView {
    var color = Theme.accent { didSet { needsDisplay = true } }
    override func draw(_ dirtyRect: NSRect) { color.setFill(); NSBezierPath(ovalIn: bounds).fill() }
}

/// A closure as a menu item's target (kept alive by the item's representedObject).
final class MenuAction: NSObject {
    private let f: () -> Void
    init(_ f: @escaping () -> Void) { self.f = f }
    @objc func fire(_ sender: Any?) { f() }
}
