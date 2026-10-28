// Advanced window, DAW mode (PANEL_REDESIGN_PLAN_v3 / v2 §4, phase M2): everything is monitored in the DAW, so there is no mixer on screen.
// Input meter bridge (what is recorded: taken before the interface mixer, special.rs:14-41), output meter bridge, and a control room:
// monitor level on Line 1/2 (also the front knob's target in this mode), speakers A (Line 1/2) / B (Line 3/4), the two phones with DAW 1/2
// or DAW 3/4 (a cue the DAW sends there), mute and dim in the header. Meters are layer-backed MeterBars (GPU-first, plan v3 §5); names are
// the user's, per unit (AdvancedStore), double-click to rename.
import AppKit

/// A small red latch under the peak readout: lit when the channel reached 0 dBFS, until clicked.
/// Clip latch. Inside a layer-backed column it is a coloured layer (no drawing on a meter tick, GPU-first rule); draw() is the fallback.
final class ClipLatch: NSView {
    var lit = false { didSet { if lit != oldValue { needsDisplay = true } } }
    override var wantsUpdateLayer: Bool { return true }
    override func updateLayer() { layer?.cornerRadius = 2; layer?.backgroundColor = (lit ? Theme.meterHigh : Theme.ledOff).cgColor }
    override func draw(_ dirtyRect: NSRect) { Theme.fill(bounds, radius: 2, lit ? Theme.meterHigh : Theme.ledOff) }
    override func mouseDown(with event: NSEvent) { lit = false }
}

/// Right-click on a channel (a view option per mode, plan v2 §3): hide it in this mode, or show the hidden ones again.
func hideChannelMenu(name: String, hiddenCount: Int, hide: (() -> Void)?, showAll: @escaping () -> Void) -> NSMenu {
    let menu = NSMenu(); menu.autoenablesItems = false
    func item(_ t: String, enabled: Bool, _ f: @escaping () -> Void) {
        let a = MenuAction(f)
        let i = NSMenuItem(title: t, action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
        i.target = a; i.representedObject = a; i.isEnabled = enabled
        menu.addItem(i)
    }
    if let h = hide { item("Hide \u{201C}\(name)\u{201D} in this mode", enabled: true, h) }
    item(hiddenCount == 0 ? "No hidden channels" : "Show the \(hiddenCount) hidden channel\(hiddenCount == 1 ? "" : "s")", enabled: hiddenCount > 0, showAll)
    return menu
}

/// One meter column: peak hold readout, clip latch, name, jack -> Core Audio channel; its bar(s) are drawn by the bridge's MeterField.
/// No subviews (8-bit rule, user 2026-10-28): the column draws its own texts and latch once, and on a tick redraws only the readout or
/// the latch rectangle when they change. One font-smoothing setting for every column (as text-field subviews in the root's one layer,
/// some came out with subpixel and some with grayscale smoothing).
final class AdvMeterColumn: FlippedView {
    let slots: [Int]
    private(set) var barRects: [NSRect] = []         // in this view's coordinates
    private(set) var values: [Int]                   // the last tick's dB per bar, for the MeterField
    private var held = -64, heldStamp: CFAbsoluteTime = 0, shownPeak = Int.min
    private var name: String, sub: String, peakText = ""
    private var latchLit = false, latchRect = NSRect.zero
    var onRename: (() -> Void)?
    var contextMenu: (() -> NSMenu?)?
    override func menu(for event: NSEvent) -> NSMenu? { return contextMenu?() }

    init(slots: [Int], name: String, sub s: String) {
        self.slots = slots
        values = slots.map { _ in -64 }
        self.name = name; sub = s
        super.init(frame: .zero)                // no layer of its own: under the MeterField every extra layer is composited on each tick
        toolTip = name + " \u{2014} double-click to rename"
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    func setName(_ n: String) { guard n != name else { return }; name = n; toolTip = n + " \u{2014} double-click to rename"; needsDisplay = true }

    override var isOpaque: Bool { return true }
    private var peakRect: NSRect { return NSRect(x: 0, y: 0, width: bounds.width, height: 13) }
    override func draw(_ dirtyRect: NSRect) {
        Theme.surface.setFill(); dirtyRect.fill()
        NSGraphicsContext.current?.cgContext.setShouldSmoothFonts(false)
        let w = bounds.width, h = bounds.height
        Theme.draw(peakText, in: peakRect, font: Theme.mono(9.5), color: Theme.text2)
        Theme.fill(latchRect, radius: 2, latchLit ? Theme.meterHigh : Theme.ledOff)
        Theme.draw(name, in: NSRect(x: 0, y: h - 24, width: w, height: 13), font: Theme.font(9.5, .semibold), color: Theme.text)
        Theme.draw(sub, in: NSRect(x: 0, y: h - 12, width: w, height: 12), font: Theme.mono(8.5), color: Theme.text3)
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        let w = newSize.width, h = newSize.height
        let top: CGFloat = 24, bottom = h - 26
        // broad bars (user, 2026-10-28): about half the column, one bar up to 40 px, a stereo pair up to 20 px each
        let k = slots.count
        let bw: CGFloat = max(8, min(k == 1 ? 40 : 20, (w * 0.55 / CGFloat(k)).rounded(.down))), gap: CGFloat = 2
        let total = CGFloat(k) * bw + CGFloat(k - 1) * gap
        latchRect = NSRect(x: ((w - total) / 2).rounded(), y: 16, width: total, height: 4)
        barRects = (0..<k).map { NSRect(x: ((w - total) / 2).rounded() + CGFloat($0) * (bw + gap), y: top, width: bw, height: max(10, bottom - top)) }
    }

    /// Reads this column's slots; the readout holds the peak 1.2 s, then follows (it redraws only when the held value changes).
    func update(_ db: (Int) -> Int) {
        values = slots.map(db)
        let top = values.max() ?? -64, now = CFAbsoluteTimeGetCurrent()
        if top >= 0 && !latchLit { latchLit = true; setNeedsDisplay(latchRect) }
        if top >= held || now - heldStamp > 1.2 { held = top; heldStamp = now }
        if held != shownPeak { shownPeak = held; peakText = held <= -64 ? "\u{2212}\u{221E}" : "\(held)"; setNeedsDisplay(peakRect) }
    }

    override func mouseDown(with event: NSEvent) {
        let p = convert(event.locationInWindow, from: nil)
        if event.clickCount == 2, p.y > bounds.height - 26 { onRename?() }
        else if latchLit, latchRect.insetBy(dx: -4, dy: -6).contains(p) { latchLit = false; setNeedsDisplay(latchRect) }   // click clears the latch
    }
}

/// ――― TEXT ――― centred over [a, b] at height y (user, 2026-10-28: the label spans every meter it names); used by AdvMeterBridge and the
/// Dual window's BigMeterRow.
func drawRuleLabel(_ text: String, _ a: CGFloat, _ b: CGFloat, y: CGFloat, font: NSFont, color: NSColor) {
    let t = text.uppercased(), tw = Theme.textWidth(t, font: font, kern: 0.8)
    let cx = (a + b) / 2
    Theme.draw(t, in: NSRect(x: cx - tw / 2 - 2, y: y - 7, width: tw + 4, height: 14), font: font, color: color, kern: 0.8)
    Theme.track.setFill()
    if cx - tw / 2 - 8 > a { NSRect(x: a, y: y, width: cx - tw / 2 - 8 - a, height: 1).fill() }
    if b > cx + tw / 2 + 8 { NSRect(x: cx + tw / 2 + 8, y: y, width: b - (cx + tw / 2 + 8), height: 1).fill() }
}

/// A card of meter columns in groups, each group with its dB scale. Two label lines above the meters, as in the Dual window (user,
/// 2026-10-28): the caption centred over all the columns, then each group's title centred over its columns; the note is the tooltip.
final class AdvMeterBridge: FlippedView {
    struct Group { let title: String; let columns: [AdvMeterColumn] }
    private let caption: String
    private(set) var groups: [Group] = []
    private var scales: [MeterScale] = []
    private var groupSpans: [(String, CGFloat, CGFloat)] = []      // title, first column's left, last column's right
    private let field = MeterField(frame: .zero)                    // every bar of the card, one image
    var columnWidth: CGFloat = 46

    init(caption: String, note: String) { self.caption = caption; super.init(frame: .zero); toolTip = note }
    required init?(coder: NSCoder) { fatalError("unused") }

    func setGroups(_ g: [Group]) {
        subviews.forEach { $0.removeFromSuperview() }
        groups = g
        scales = g.map { _ in MeterScale(frame: .zero) }
        scales.forEach { addSubview($0) }
        g.forEach { $0.columns.forEach { addSubview($0) } }
        addSubview(field)                       // on top of the (opaque) columns; it lets clicks through
        needsLayout = true
        layoutColumns()
    }

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutColumns() }
    private func layoutColumns() {
        var x: CGFloat = 14
        let top: CGFloat = 40, h = bounds.height - top - 10
        groupSpans = []
        for (gi, g) in groups.enumerated() {
            scales[gi].frame = NSRect(x: x, y: top + 24, width: 18, height: max(10, h - 24 - 26))
            x += 22
            let start = x
            for c in g.columns {
                c.frame = NSRect(x: x, y: top, width: columnWidth, height: h)
                x += columnWidth + 4
            }
            groupSpans.append((g.title, start, x - 4))
            x += 14
        }
        field.frame = bounds
        field.setBars(groups.flatMap { $0.columns }.flatMap { c in c.barRects.map { field.convert($0, from: c) } })
        needsDisplay = true
    }
    /// After the columns' update: their values to the field, in bar order.
    func flush() { field.update(groups.flatMap { $0.columns }.flatMap { $0.values }) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 10, Theme.surface)
        guard let first = groupSpans.first, let last = groupSpans.last else { return }
        drawRuleLabel(caption, first.1, last.2, y: 12, font: Theme.font(10, .bold), color: Theme.text)
        for (t, a, b) in groupSpans where !t.isEmpty { drawRuleLabel(t, a, b, y: 29, font: Theme.font(8.5, .semibold), color: Theme.text3) }
    }
}

/// The monitor knob of the control room: a big knob, vertical drag (0.5 dB per 2 px) or scroll; the console law's range (off at -129).
final class MonitorKnob: NSView {
    var value: Int = MARange.levelMin { didSet { if value != oldValue { needsDisplay = true } } }
    var onChange: ((Int) -> Void)?
    var onCommit: (() -> Void)?
    private var lastY: CGFloat = 0, acc: CGFloat = 0
    override var isFlipped: Bool { return true }

    /// 0 at off, 1 at 0 dB, on the fader law so the pointer matches the faders of the standard panel
    private var travel: CGFloat { return 1 - FaderLaw.console.fraction(forValue: value) }

    override func draw(_ dirtyRect: NSRect) {
        let d = min(bounds.width, bounds.height) - 24
        let r = NSRect(x: (bounds.width - d) / 2, y: (bounds.height - d) / 2, width: d, height: d)
        let c = NSPoint(x: r.midX, y: r.midY)
        // ring of dots, lit up to the value (-135 ... +135 degrees, 0 = up; flipped view: y grows downwards)
        let n = 31
        for i in 0..<n {
            let f = CGFloat(i) / CGFloat(n - 1), a = (-135 + 270 * f) * .pi / 180
            let p = NSPoint(x: c.x + sin(a) * (d / 2 + 9), y: c.y - cos(a) * (d / 2 + 9))
            (f <= travel + 0.001 && value > MARange.levelMin ? Theme.accent : Theme.track).setFill()
            NSBezierPath(ovalIn: NSRect(x: p.x - 1.6, y: p.y - 1.6, width: 3.2, height: 3.2)).fill()
        }
        NSGraphicsContext.saveGraphicsState()
        let sh = NSShadow(); sh.shadowColor = NSColor(white: 0, alpha: 0.6); sh.shadowBlurRadius = 10; sh.shadowOffset = NSSize(width: 0, height: -4); sh.set()
        Theme.control.setFill(); NSBezierPath(ovalIn: r).fill()
        NSGraphicsContext.restoreGraphicsState()
        NSColor(white: 1, alpha: 0.06).setStroke(); let ring = NSBezierPath(ovalIn: r.insetBy(dx: 0.5, dy: 0.5)); ring.lineWidth = 1; ring.stroke()
        let a = (-135 + 270 * travel) * .pi / 180
        let p1 = NSPoint(x: c.x + sin(a) * d * 0.18, y: c.y - cos(a) * d * 0.18), p2 = NSPoint(x: c.x + sin(a) * d * 0.40, y: c.y - cos(a) * d * 0.40)
        let ptr = NSBezierPath(); ptr.move(to: p1); ptr.line(to: p2); ptr.lineWidth = 3.5; ptr.lineCapStyle = .roundLineCapStyle
        Theme.text.setStroke(); ptr.stroke()
    }

    private func nudge(_ halfDB: Int) {
        guard halfDB != 0 else { return }
        let base = value <= MARange.levelMin ? MARange.levelMin : value
        var v = min(MARange.levelMax, base + halfDB)
        if v <= MARange.levelMin { v = MARange.levelMin }             // the console floor: off
        if v != value { value = v; onChange?(v) }
    }
    override func mouseDown(with event: NSEvent) { lastY = event.locationInWindow.y; acc = 0 }
    override func mouseDragged(with event: NSEvent) {
        acc += event.locationInWindow.y - lastY; lastY = event.locationInWindow.y
        let steps = Int(acc / 2); if steps != 0 { acc -= CGFloat(steps) * 2; nudge(steps) }
    }
    override func mouseUp(with event: NSEvent) { onCommit?() }
    override func scrollWheel(with event: NSEvent) { nudge(event.scrollingDeltaY > 0 ? 1 : (event.scrollingDeltaY < 0 ? -1 : 0)); onCommit?() }
}

/// Horizontal level fader with a stereo meter in its track (phones rows).
final class HMeterFader: FlippedView {
    var value = 0 { didSet { if value != oldValue { needsDisplay = true } } }
    var law = FaderLaw.monitor
    var onChange: ((Int) -> Void)?
    var onCommit: (() -> Void)?
    let bars = [MeterBar(frame: .zero), MeterBar(frame: .zero)]
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        bars.forEach { $0.horizontal = true; $0.showsClip = false; $0.surface = Theme.well; addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        bars[0].frame = NSRect(x: 8, y: newSize.height / 2 - 5, width: newSize.width - 16, height: 4)
        bars[1].frame = NSRect(x: 8, y: newSize.height / 2 + 1, width: newSize.width - 16, height: 4)
    }
    private var capX: CGFloat { return 8 + (1 - law.fraction(forValue: value)) * (bounds.width - 16) }
    override func draw(_ dirtyRect: NSRect) { Theme.fill(bounds, radius: 5, Theme.well) }
    override func viewDidMoveToWindow() { super.viewDidMoveToWindow(); addCap() }
    private let cap = NSView()
    private func addCap() {
        guard cap.superview == nil else { return }
        cap.wantsLayer = true; cap.layer?.backgroundColor = Theme.capFace.cgColor; cap.layer?.cornerRadius = 3
        cap.layer?.shadowOpacity = 0.6; cap.layer?.shadowRadius = 2; cap.layer?.shadowOffset = CGSize(width: 0, height: -1)
        addSubview(cap)
    }
    override func layout() { super.layout(); placeCap() }
    func placeCap() { cap.frame = NSRect(x: capX - 5.5, y: -1, width: 11, height: bounds.height + 2) }
    private func set(_ x: CGFloat) {
        let f = 1 - min(1, max(0, (x - 8) / (bounds.width - 16)))
        let v = law.value(forFraction: f)
        if v != value { value = v; placeCap(); onChange?(v) }
    }
    override func mouseDown(with event: NSEvent) { set(convert(event.locationInWindow, from: nil).x) }
    override func mouseDragged(with event: NSEvent) { set(convert(event.locationInWindow, from: nil).x) }
    override func mouseUp(with event: NSEvent) { onCommit?() }
}

final class AdvancedDAWView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let ctx: PanelContext
    private let store: AdvancedStore
    private let inputs = AdvMeterBridge(caption: "Inputs \u{2192} DAW", note: "recorded before the interface mixer: nothing here changes what is recorded")
    private let outputs = AdvMeterBridge(caption: "DAW \u{2192} outputs", note: "Core Audio channel under each meter")
    private let status = makeLabel("", size: 11, color: Theme.text3)
    // control room
    private let monitorCard = Card(title: "Monitor"), phonesCard = Card(title: "Phones")
    private let monitorNote = makeLabel("", size: 10.5, color: Theme.text3, align: .right)
    private let knob = MonitorKnob()
    private let knobValue = makeLabel("", size: 21, weight: .semibold, color: Theme.text, align: .center)
    private let knobHint = makeLabel("drag, scroll, or turn the knob on the 1814", size: 11, color: Theme.text3, align: .center)
    private let speakers = SegmentedBar(items: [.text("Speakers A \u{00B7} Line 1/2"), .text("B \u{00B7} Line 3/4")], segmentWidth: 133)
    private let speakersNote = makeLabel("", size: 10.5, color: Theme.text3)
    private var phoneRows: [(source: SegmentedBar, fader: HMeterFader, value: NSTextField, mute: PillButton, label: NSTextField)] = []
    private var layoutKey: ChannelLayout?
    private var channels = AdvancedChannels.none
    private var inCols: [AdvMeterColumn] = [], outCols: [AdvMeterColumn] = []

    init(context: PanelContext, store: AdvancedStore) {
        ctx = context; self.store = store
        super.init(frame: .zero)
        [inputs, outputs, status, monitorCard, phonesCard].forEach { addSubview($0) }
        status.lineBreakMode = .byTruncatingTail
        [monitorNote, knob, knobValue, knobHint, speakers, speakersNote].forEach { monitorCard.addSubview($0) }
        monitorNote.stringValue = "Line 1/2 \u{00B7} the front knob"
        speakersNote.maximumNumberOfLines = 3; speakersNote.lineBreakMode = .byWordWrapping; speakersNote.cell?.wraps = true
        knob.onChange = { [weak self] v in self?.setMonitor(v) }
        knob.onCommit = { [weak self] in self?.ctx.model.commit() }
        speakers.onSelect = { [weak self] i in self?.setSpeakers(b: i == 1) }
        for n in 1...2 {
            let src = SegmentedBar(items: [.text("DAW 1/2"), .text("DAW 3/4 \u{00B7} cue")], segmentWidth: 98)
            let f = HMeterFader(frame: .zero)
            let v = makeLabel("", size: 11, color: Theme.text2, align: .right); v.font = Theme.mono(11)
            let m = PillButton("M"); m.solid = true; m.onColor = Theme.mute
            let l = makeLabel("Phones \(n)", size: 11.5, weight: .semibold, color: Theme.text)
            src.onSelect = { [weak self] i in self?.ctx.model.setPhonesSource(phones: n, mask: i == 0 ? 1 : 2) }
            f.onChange = { [weak self] val in self?.ctx.model.setLevel(MAKeys.headphoneLevels, channel: 2 * n - 1, value: val, linked: true) }
            f.onCommit = { [weak self] in self?.ctx.model.commit() }
            m.onClick = { [weak self] b in
                guard let me = self else { return }
                me.ctx.model.setElement(MAKeys.headphoneMute, 2 * n - 1, b.isOn ? 1 : 0)
                me.ctx.model.setElement(MAKeys.headphoneMute, 2 * n, b.isOn ? 1 : 0)
                me.ctx.model.commit()
            }
            [l, src, f, v, m].forEach { phonesCard.addSubview($0) }
            phoneRows.append((src, f, v, m, l))
        }
        rebuildChannels()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    // MARK: channels (rebuilt when the layout changes: optical modes)

    private func rebuildChannels() {
        let l = ctx.model.layout
        layoutKey = l
        channels = ctx.profile.advancedChannels(l)
        let key = store.viewKey, hidden = store.hiddenIDs(key)
        func col(_ c: AdvancedChannel, sub: String) -> AdvMeterColumn {
            let v = AdvMeterColumn(slots: [c.slot], name: store.name(c), sub: sub)
            v.onRename = { [weak self, weak v] in self?.rename(c) { v?.setName($0) } }
            v.contextMenu = { [weak self] in
                guard let me = self else { return nil }
                return hideChannelMenu(name: me.store.name(c), hiddenCount: hidden.count,
                                       hide: { me.store.setHidden(c.id, true, in: key); me.rebuildChannels(); me.refresh() },
                                       showAll: { me.store.showAll(in: key); me.rebuildChannels(); me.refresh() })
            }
            return v
        }
        let ins = channels.inputs.filter { !hidden.contains($0.id) }, outs = channels.outputs.filter { !hidden.contains($0.id) }
        func short(_ j: String) -> String {
            return j.replacingOccurrences(of: "Mic/Line ", with: "In ").replacingOccurrences(of: "Line ", with: "In ").replacingOccurrences(of: "ADAT ", with: "A").replacingOccurrences(of: "S/PDIF ", with: "S")
        }
        inCols = ins.map { col($0, sub: "\(short($0.jack)) \u{2192} \($0.ca)") }
        outCols = outs.map { col($0, sub: "DAW \($0.ca)") }
        let phoneCols = channels.phones.map { p -> AdvMeterColumn in AdvMeterColumn(slots: [p.slots.0, p.slots.1], name: "Ph \(p.number)", sub: "") }
        func groups(_ cs: [AdvancedChannel], _ cols: [AdvMeterColumn]) -> [AdvMeterBridge.Group] {
            var out: [AdvMeterBridge.Group] = []
            for (c, v) in zip(cs, cols) {
                if let last = out.last, last.title == c.group { out[out.count - 1] = AdvMeterBridge.Group(title: last.title, columns: last.columns + [v]) }
                else { out.append(AdvMeterBridge.Group(title: c.group, columns: [v])) }
            }
            return out
        }
        var ig = groups(ins, inCols)
        if let first = ig.first, first.title == "Analog" { ig[0] = AdvMeterBridge.Group(title: "Analog \u{00B7} Mic/Line 1\u{2013}2 gain on the front", columns: first.columns) }
        inputs.setGroups(ig)
        outputs.setGroups(groups(outs, outCols) + [AdvMeterBridge.Group(title: "Phones", columns: phoneCols)])
        outCols += phoneCols
        layoutAll()
    }

    private func rename(_ c: AdvancedChannel, done: @escaping (String) -> Void) {
        guard let w = window else { return }
        let a = NSAlert()
        a.messageText = "Name for \(c.jack) (DAW \(c.ca))"
        a.informativeText = "Kept by the panel for this interface. Leave empty for \u{201C}\(c.name)\u{201D}."
        let f = NSTextField(frame: NSRect(x: 0, y: 0, width: 240, height: 24))
        f.stringValue = store.name(c)
        a.accessoryView = f
        a.addButton(withTitle: "Rename"); a.addButton(withTitle: "Cancel")
        a.window.initialFirstResponder = f
        a.beginSheetModal(for: w) { [weak self] r in
            guard r == .alertFirstButtonReturn, let me = self else { return }
            me.store.setName(f.stringValue, for: c.id)
            done(me.store.name(c))
        }
    }

    // MARK: layout

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutAll() }
    private func layoutAll() {
        let w = bounds.width, h = bounds.height, p = Theme.pad
        let crW: CGFloat = 300, left = w - 3 * p - crW
        let inH = ((h - 2 * p - 30) * 0.53).rounded(), outH = h - 2 * p - 30 - inH - 10
        inputs.columnWidth = min(120, max(34, (left - 28 - CGFloat(max(1, inputs.groups.count)) * 36 - CGFloat(inCols.count) * 4) / CGFloat(max(1, inCols.count))))
        outputs.columnWidth = min(120, max(32, (left - 28 - CGFloat(max(1, outputs.groups.count)) * 36 - CGFloat(outCols.count) * 4) / CGFloat(max(1, outCols.count))))
        inputs.frame = NSRect(x: p, y: p, width: left, height: inH)
        outputs.frame = NSRect(x: p, y: p + inH + 10, width: left, height: outH)
        status.frame = NSRect(x: p + 2, y: h - p - 16, width: left, height: 16)
        let x = w - p - crW
        monitorCard.frame = NSRect(x: x, y: p, width: crW, height: 318)
        monitorNote.frame = NSRect(x: 120, y: 11, width: crW - 138, height: 16)
        knob.frame = NSRect(x: (crW - 150) / 2, y: 34, width: 150, height: 150)
        knobValue.frame = NSRect(x: 10, y: 188, width: crW - 20, height: 26)
        knobHint.frame = NSRect(x: 10, y: 214, width: crW - 20, height: 16)
        speakers.setFrameOrigin(NSPoint(x: (crW - speakers.frame.width) / 2, y: 238))
        speakersNote.frame = NSRect(x: 18, y: 274, width: crW - 36, height: 40)
        phonesCard.frame = NSRect(x: x, y: p + 318 + 10, width: crW, height: 186)
        for (i, r) in phoneRows.enumerated() {
            let y = 40 + CGFloat(i) * 72
            r.label.frame = NSRect(x: 18, y: y, width: 62, height: 16)
            r.source.setFrameOrigin(NSPoint(x: crW - 18 - r.source.frame.width, y: y - 7))
            r.fader.frame = NSRect(x: 18, y: y + 30, width: crW - 18 - 18 - 46 - 26, height: 22)
            r.value.frame = NSRect(x: crW - 18 - 26 - 46, y: y + 33, width: 40, height: 16)
            r.mute.frame = NSRect(x: crW - 18 - 22, y: y + 31, width: 22, height: 20)
            r.fader.placeCap()
        }
    }

    // MARK: model -> view

    private var speakersB: Bool { return Speakers.isB(ctx.model) }
    private var monitorChannel: Int { return Speakers.monitorChannel(ctx.model) }

    func refresh() {
        let m = ctx.model
        if layoutKey != m.layout { rebuildChannels() }
        knob.value = m.faderLevel(MAKeys.outputLevels, channel: monitorChannel)
        knobValue.stringValue = Theme.levelText(knob.value) + " dB"
        speakers.selected = speakersB ? 1 : 0
        monitorNote.stringValue = (speakersB ? "Line 3/4" : "Line 1/2") + " \u{00B7} the front knob"
        speakersNote.stringValue = speakersB
            ? "B: DAW 1/2 plays on Line 3/4 and Line 1/2 is muted; DAW 3/4 is off Line 3/4 meanwhile."
            : "A: DAW 1/2 on Line 1/2. B moves DAW 1/2 to Line 3/4 (second pair of speakers)."
        for (i, r) in phoneRows.enumerated() {
            let n = i + 1
            let src = Int(m.number(n == 1 ? MAKeys.headphoneSourceV2 : MAKeys.headphoneSecondSource) ?? 1)
            r.source.selected = src == 2 ? 1 : 0
            r.fader.value = m.faderLevel(MAKeys.headphoneLevels, channel: 2 * n - 1)
            r.fader.placeCap()
            r.value.stringValue = Theme.levelText(r.fader.value, law: .monitor)
            r.mute.isOn = m.element(MAKeys.headphoneMute, 2 * n - 1) != 0
        }
        status.stringValue = "Interface mixer: passthrough \u{2014} monitor in the DAW   \u{00B7}   buffer size: set in the DAW   \u{00B7}   driver round trip \u{2248} 9 ms at 48 kHz / 64 frames (bench)"
    }

    func updateMeters() {
        let db = { [ctx] (s: Int) -> Int in ctx.db(s) }
        inCols.forEach { $0.update(db) }
        outCols.forEach { $0.update(db) }
        inputs.flush(); outputs.flush()
        for (i, r) in phoneRows.enumerated() where i < channels.phones.count {
            let s = channels.phones[i].slots
            r.fader.bars[0].db = ctx.db(s.0); r.fader.bars[1].db = ctx.db(s.1)
        }
    }

    // MARK: control room actions

    private func setMonitor(_ v: Int) {
        ctx.model.setLevel(MAKeys.outputLevels, channel: monitorChannel, value: v, linked: true)
        knobValue.stringValue = Theme.levelText(v) + " dB"
    }

    private func setSpeakers(b: Bool) { Speakers.set(ctx.model, b: b); refresh() }
}

/// Speakers A / B of the DAW mode (plan v2 §4), shared by the single and the Dual DAW views. B: DAW 1/2 also into Mix 3/4, DAW 3/4 out of
/// it, Line 1/2 muted, Line 3/4 at the monitor level, the front knob on Line 3/4. A undoes it (DAW returns to their own pairs, Line 1/2
/// unmuted at the level B had, the knob back on Line 1/2).
enum Speakers {
    static func isB(_ m: DeviceModel) -> Bool {
        return (m.element(MAKeys.swReturnRoutingV2, 1) & 2) != 0 && m.element(MAKeys.outputMute, 1) != 0
    }
    /// The output channel the monitor knob drives: Line 1 (A) or Line 3 (B)
    static func monitorChannel(_ m: DeviceModel) -> Int { return isB(m) ? 3 : 1 }

    static func set(_ m: DeviceModel, b: Bool) {
        guard b != isB(m) else { return }
        let level = m.faderLevel(MAKeys.outputLevels, channel: monitorChannel(m))
        if b {
            m.setElement(MAKeys.swReturnRoutingV2, 1, 3)
            m.setElement(MAKeys.swReturnRoutingV2, 2, 0)
            m.setLevel(MAKeys.outputLevels, channel: 3, value: level, linked: true)
            for ch in [3, 4] { m.setElement(MAKeys.outputMute, ch, 0) }
            for ch in [1, 2] { m.setElement(MAKeys.outputMute, ch, 1) }
            m.setNumber(MAKeys.rotaryControl, MARotaryMask.outputPairBit(2))
        } else {
            m.setElement(MAKeys.swReturnRoutingV2, 1, 1)
            m.setElement(MAKeys.swReturnRoutingV2, 2, 2)
            m.setLevel(MAKeys.outputLevels, channel: 1, value: level, linked: true)
            for ch in [1, 2] { m.setElement(MAKeys.outputMute, ch, 0) }
            m.setNumber(MAKeys.rotaryControl, MARotaryMask.outputPairBit(1))
        }
        m.commit()
    }
}
