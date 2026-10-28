// Advanced window, Console mode (PANEL_REDESIGN_PLAN_v3 / v2 §4, phase M3): the interface as a converter for an analog console. A channel table
// per direction with the user's labels (the same per-unit names as DAW mode), linear dBFS meters with a 0 VU mark at the chosen reference,
// peak readouts and clip latches; a unity lock that watches the interface levels between the DAW and the console (it warns and offers
// "Restore 0 dB"; it does not rewrite on its own, so it never fights the front knob); clock and digital-port status; a phones row.
import AppKit

/// Horizontal meter on a linear dBFS scale (-60 ... 0) with a 0 VU mark: green below the reference, amber from it, red from -3 dBFS.
/// Layer-backed like MeterBar: the segment images are rendered once per size / reference, a level change only resizes the lit layer.
final class LinearMeter: NSView {
    var reference = -18 { didSet { if reference != oldValue { renderedFor = nil; needsLayout = true } } }
    var db = -64 { didSet { apply() } }
    private let lit = CALayer(), ref = CALayer()          // the unlit pattern is the layer's own contents (one sublayer fewer)
    private var renderedFor: (NSSize, CGFloat, Int)?
    private var shown = -1
    static let floor = -60
    static func fraction(_ db: Int) -> CGFloat { return CGFloat(min(0, max(LinearMeter.floor, db)) - LinearMeter.floor) / CGFloat(-LinearMeter.floor) }

    override init(frame: NSRect) {
        super.init(frame: frame)
        wantsLayer = true
        layerContentsRedrawPolicy = .never
        let still: [String: CAAction] = ["position": NSNull(), "bounds": NSNull(), "frame": NSNull(), "contents": NSNull(), "backgroundColor": NSNull()]
        for l in [lit, ref] { l.actions = still; l.anchorPoint = .zero }
        lit.contentsGravity = kCAGravityBottomLeft
        lit.masksToBounds = true
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override var wantsUpdateLayer: Bool { return true }
    override func updateLayer() {
        guard let l = layer else { return }
        l.backgroundColor = Theme.well.cgColor
        if lit.superlayer !== l { [lit, ref].forEach { l.addSublayer($0) } }
    }
    override func layout() { super.layout(); updateLayer(); render(); shown = -1; apply() }

    private func render() {
        let scale = window?.backingScaleFactor ?? 2
        if let r = renderedFor, r.0 == bounds.size, r.1 == scale, r.2 == reference { return }
        renderedFor = (bounds.size, scale, reference)
        let seg: CGFloat = 3, gap: CGFloat = 1
        let n = max(1, Int((bounds.width + gap) / (seg + gap)))
        let refF = LinearMeter.fraction(reference), hotF = LinearMeter.fraction(-3)
        func image(_ color: (CGFloat) -> NSColor) -> CGImage? {
            let w = Int((bounds.width * scale).rounded()), h = Int((bounds.height * scale).rounded())
            guard w > 0, h > 0, let ctx = CGContext(data: nil, width: w, height: h, bitsPerComponent: 8, bytesPerRow: 0, space: CGColorSpaceCreateDeviceRGB(),
                                                    bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue) else { return nil }
            ctx.scaleBy(x: scale, y: scale)
            for i in 0..<n {
                let x = CGFloat(i) * (seg + gap)
                ctx.setFillColor(color(x / bounds.width).cgColor)
                ctx.fill(CGRect(x: x, y: 0, width: seg, height: bounds.height))
            }
            return ctx.makeImage()
        }
        layer?.contentsScale = scale; lit.contentsScale = scale
        layer?.contents = image { _ in Theme.ledOff }
        lit.contents = image { f in f >= hotF ? Theme.meterHigh : (f >= refF ? Theme.meterMid : Theme.meterLow) }
        ref.backgroundColor = NSColor(white: 1, alpha: 0.8).cgColor
        ref.frame = CGRect(x: (refF * bounds.width).rounded() - 1, y: 0, width: 2, height: bounds.height)
    }
    private func apply() {
        let w = Int((db <= -64 ? 0 : LinearMeter.fraction(db)) * bounds.width)
        if w == shown { return }
        shown = w
        lit.frame = CGRect(x: 0, y: 0, width: CGFloat(w), height: bounds.height)
    }
}

/// Tick labels over a column of LinearMeters (-48, -36, -24, the reference as "0 VU", -6, 0).
final class LinearScale: FlippedView {
    var reference = -18 { didSet { needsDisplay = true } }
    override func draw(_ dirtyRect: NSRect) {
        var marks = [-48, -36, -24, -6, 0].filter { abs($0 - reference) > 3 }
        marks.append(reference)
        for d in marks {
            let x = LinearMeter.fraction(d) * bounds.width
            let t = d == reference ? "0 VU" : "\(d)"
            // the end marks stay inside the view (0 dBFS sits on the right edge)
            let r = d == 0 ? NSRect(x: bounds.width - 40, y: 0, width: 40, height: bounds.height) : NSRect(x: x - 20, y: 0, width: 40, height: bounds.height)
            Theme.draw(t, in: r, font: Theme.mono(8.5, d == reference ? .semibold : .regular), color: d == reference ? Theme.text : Theme.text3,
                       align: d == 0 ? .right : .center)
        }
    }
}

/// One table row: editable label, jack, Core Audio channel, meter, peak, clip latch.
final class ConsoleRow: FlippedView {
    let channel: AdvancedChannel
    let field = NSTextField(string: "")
    private let jack = makeLabel("", size: 10.5, color: Theme.text3)
    private let ca = makeLabel("", size: 10.5, color: Theme.text2)
    let meter = LinearMeter(frame: .zero)
    private let peak = makeLabel("", size: 10, color: Theme.text2, align: .right)
    private let latch = ClipLatch()
    private var hold = -64, holdStamp: CFAbsoluteTime = 0, shownPeak = Int.min
    var onRename: ((String) -> Void)?
    var contextMenu: (() -> NSMenu?)?
    override func menu(for event: NSEvent) -> NSMenu? { return contextMenu?() }
    /// PFL (user, 2026-10-28): the channel to the phones through the Aux, the main outputs untouched; nil = not through the mixer (ADAT out)
    let cue = PillButton("CUE")
    var cueElement: Int? { didSet { cue.isHidden = cueElement == nil } }
    let input: Bool
    /// Column widths: label, jack, Core Audio channel, peak; `compact` for the Dual window's three tables
    struct Widths { let label: CGFloat, jack: CGFloat, ca: CGFloat, peak: CGFloat }
    static let regular = Widths(label: 124, jack: 78, ca: 76, peak: 36), compact = Widths(label: 92, jack: 66, ca: 74, peak: 30)
    var widths = ConsoleRow.regular { didSet { setFrameSize(frame.size) } }

    init(_ c: AdvancedChannel, name: String, input: Bool) {
        channel = c; self.input = input
        super.init(frame: .zero)
        wantsLayer = true                       // one layer for the column: its labels and latch draw into it, only its meter bars have their own
        canDrawSubviewsIntoLayer = true         // (GPU-first rule; a peak change redraws the column, never the card)
        field.stringValue = name
        field.placeholderString = c.name
        field.font = Theme.font(11.5)
        field.textColor = Theme.text
        field.isBordered = false; field.drawsBackground = false; field.focusRingType = .none
        field.backgroundColor = Theme.well
        field.target = self; field.action = #selector(renamed)        // fires on Return and when the field loses focus
        field.toolTip = "Your label for this channel (kept for this interface)"
        jack.stringValue = c.jack
        ca.stringValue = input ? "\u{2192} DAW \(c.ca)" : "DAW \(c.ca) \u{2192}"
        ca.font = Theme.mono(10.5); peak.font = Theme.mono(10)
        cue.solid = true; cue.onColor = Theme.solo; cue.fontSize = 8.5; cue.isHidden = true
        cue.toolTip = "Cue (PFL): hear this channel alone on Phones 1, through the Aux; the outputs to the console do not change"
        [field, jack, ca, meter, peak, latch, cue].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    @objc private func renamed() { onRename?(field.stringValue) }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        let h = newSize.height, cy = (h - 16) / 2
        var x: CGFloat = 0
        // inputs: label, jack, -> DAW; outputs: label, DAW ->, jack (the signal reads left to right)
        let order: [(NSView, CGFloat)] = input ? [(field, widths.label), (jack, widths.jack), (ca, widths.ca)]
                                               : [(field, widths.label), (ca, widths.ca), (jack, widths.jack)]
        for (v, w) in order { v.frame = NSRect(x: x, y: v === field ? cy - 2 : cy, width: w - 6, height: v === field ? 20 : 16); x += w }
        let cw: CGFloat = 36
        cue.frame = NSRect(x: x, y: ((h - 16) / 2).rounded(), width: cw - 6, height: 16)
        x += cw
        let mw = newSize.width - x - widths.peak - 18
        let mh = max(9, min(20, (h * 0.42).rounded()))               // the meter thickens with the row (user, 2026-10-28)
        meter.frame = NSRect(x: x, y: ((h - mh) / 2).rounded(), width: max(20, mw), height: mh)
        peak.frame = NSRect(x: x + mw + 2, y: cy, width: widths.peak, height: 16)
        latch.frame = NSRect(x: newSize.width - 9, y: ((h - mh) / 2).rounded(), width: 9, height: mh)
    }

    func update(_ db: Int) {
        meter.db = db
        let now = CFAbsoluteTimeGetCurrent()
        if db >= hold || now - holdStamp > 1.5 { hold = db; holdStamp = now }
        if db >= 0 { latch.lit = true }
        if hold != shownPeak { shownPeak = hold; peak.stringValue = hold <= -64 ? "\u{2212}\u{221E}" : "\(hold)" }
    }
}

/// One direction's table: caption, header row with the scale, rows.
final class ConsoleTable: FlippedView {
    private let caption: String, note: String, input: Bool
    var widths = ConsoleRow.regular
    let scale = LinearScale(frame: .zero)
    private(set) var rows: [ConsoleRow] = []
    var extra: NSView? { didSet { oldValue?.removeFromSuperview(); if let e = extra { addSubview(e) }; needsLayout = true; layoutRows() } }
    var extraHeight: CGFloat = 36 { didSet { layoutRows(); needsDisplay = true } }

    init(caption: String, note: String, input: Bool) { self.caption = caption; self.note = note; self.input = input; super.init(frame: .zero); addSubview(scale) }
    required init?(coder: NSCoder) { fatalError("unused") }

    func setRows(_ r: [ConsoleRow]) { rows.forEach { $0.removeFromSuperview() }; rows = r; r.forEach { $0.widths = widths; addSubview($0) }; layoutRows() }
    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutRows() }
    private var rowH: CGFloat { return min(48, max(19, (bounds.height - 60 - (extra == nil ? 0 : extraHeight + 8)) / CGFloat(max(1, rows.count)))) }
    private func layoutRows() {
        let x: CGFloat = 14, w = bounds.width - 28
        let mx = x + widths.label + widths.jack + widths.ca + 36       // after the CUE column
        scale.frame = NSRect(x: mx, y: 34, width: max(20, w - (mx - x) - widths.peak - 18), height: 12)
        for (i, r) in rows.enumerated() { r.frame = NSRect(x: x, y: 50 + CGFloat(i) * rowH, width: w, height: rowH) }
        extra?.frame = NSRect(x: x, y: bounds.height - extraHeight - 8, width: w, height: extraHeight)
        needsDisplay = true
    }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 10, Theme.surface)
        Theme.caption(caption, in: NSRect(x: 14, y: 10, width: 300, height: 14), align: .left)
        Theme.draw(note, in: NSRect(x: bounds.width - 474, y: 10, width: 460, height: 14), font: Theme.font(10.5), color: Theme.text3, align: .right)
        let heads = input ? ["Label", "Jack", "Core Audio"] : ["Label", "Core Audio", "Jack"]
        let ws = input ? [widths.label, widths.jack, widths.ca] : [widths.label, widths.ca, widths.jack]
        var x: CGFloat = 14
        for (t, w) in zip(heads, ws) { Theme.caption(t, in: NSRect(x: x + 2, y: 34, width: w, height: 12), color: Theme.text3, align: .left, size: 8.5); x += w }
        Theme.caption("Peak", in: NSRect(x: bounds.width - 14 - widths.peak - 18, y: 34, width: widths.peak + 2, height: 12), color: Theme.text3, align: .right, size: 8.5)
        if extra != nil { Theme.hairline.setFill(); NSRect(x: 14, y: bounds.height - extraHeight - 14, width: bounds.width - 28, height: 1).fill() }
    }
}

/// A rounded chip with text in the top bar.
final class ChipView: FlippedView {
    var text = "" { didSet { needsDisplay = true } }
    var warn = false { didSet { needsDisplay = true } }
    var dot: NSColor? { didSet { needsDisplay = true } }
    /// room after the text for a control placed by the owner (the reference pop-up)
    var extraWidth: CGFloat = 0
    var preferredWidth: CGFloat { return Theme.textWidth(text, font: Theme.font(11.5)) + (dot == nil ? 24 : 38) + extraWidth }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 8, Theme.surface)
        if warn {
            Theme.solo.withAlphaComponent(0.6).setStroke()
            let p = NSBezierPath(roundedRect: bounds.insetBy(dx: 0.5, dy: 0.5), xRadius: 8, yRadius: 8); p.lineWidth = 1; p.stroke()
        }
        var x: CGFloat = 12
        if let d = dot { d.setFill(); NSBezierPath(ovalIn: NSRect(x: x, y: bounds.midY - 3.5, width: 7, height: 7)).fill(); x += 14 }
        Theme.draw(text, in: NSRect(x: x, y: 0, width: bounds.width - x - 8, height: bounds.height), font: Theme.font(11.5), color: warn ? Theme.solo : Theme.text2, align: .left)
    }
}

/// The phones rows under DAW -> console (Console mode, single and Dual): name, source, level fader with its meter, value. Phones 1 and 2 of
/// each unit (the interface has two phones outputs; the console's own monitoring is elsewhere, these are for checks and cue).
final class ConsolePhonesBar: FlippedView {
    struct Unit { let label: String; let ctx: PanelContext }          // label "" for the single window, "A" / "B" in Dual
    private struct Row { let ctx: PanelContext; let n: Int; let name: NSTextField; let note: NSTextField; let fader: HMeterFader; let value: NSTextField }
    private var rows: [Row] = []
    private var cache: [ObjectIdentifier: (ChannelLayout, [AdvancedPhones])] = [:]
    static let rowHeight: CGFloat = 32
    var preferredHeight: CGFloat { return CGFloat(rows.count) * ConsolePhonesBar.rowHeight }

    init(units: [Unit]) {
        super.init(frame: .zero)
        for u in units {
            for n in 1...2 {
                let name = makeLabel(u.label.isEmpty ? "Phones \(n)" : "\(u.label)\(n)", size: 11.5, weight: .semibold, color: Theme.text)
                let note = makeLabel("", size: 10.5, color: Theme.text3)
                let value = makeLabel("", size: 11, color: Theme.text2, align: .right); value.font = Theme.mono(11)
                let f = HMeterFader(frame: .zero)
                let ctx = u.ctx
                f.onChange = { [weak value] v in ctx.model.setLevel(MAKeys.headphoneLevels, channel: 2 * n - 1, value: v, linked: true); value?.stringValue = Theme.levelText(v, law: .monitor) }
                f.onCommit = { ctx.model.commit() }
                [name, note, f, value].forEach { addSubview($0) }
                rows.append(Row(ctx: ctx, n: n, name: name, note: note, fader: f, value: value))
            }
        }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        let w = s.width, noteW: CGFloat = w > 520 ? 150 : 90, fx = 60 + noteW + 6
        for (i, r) in rows.enumerated() {
            let y = CGFloat(i) * ConsolePhonesBar.rowHeight + 4
            r.name.frame = NSRect(x: 0, y: y + 3, width: 58, height: 16)
            r.note.frame = NSRect(x: 60, y: y + 3, width: noteW, height: 16)
            r.fader.frame = NSRect(x: fx, y: y, width: max(40, w - fx - 50), height: 22)
            r.value.frame = NSRect(x: w - 46, y: y + 3, width: 44, height: 16)
            r.fader.placeCap()
        }
    }

    /// Phones n's source as the mixer has it (mask 1 = mix 1/2 -> Line 1/2, 2 = mix 3/4 -> Line 3/4, 4 = aux).
    private static func source(_ m: DeviceModel, _ n: Int) -> String {
        switch m.number(n == 1 ? MAKeys.headphoneSourceV2 : MAKeys.headphoneSecondSource) ?? 0 {
        case 1: return "Line 1/2"
        case 2: return "Line 3/4"
        case 4: return "Aux"
        default: return "\u{2014}"
        }
    }
    func refresh() {
        for r in rows {
            let m = r.ctx.model
            r.fader.value = m.faderLevel(MAKeys.headphoneLevels, channel: 2 * r.n - 1); r.fader.placeCap()
            r.value.stringValue = Theme.levelText(r.fader.value, law: .monitor)
            r.note.stringValue = ConsolePhonesBar.source(m, r.n) + (r.n == 1 ? " \u{00B7} check / talkback" : " \u{00B7} cue")
        }
    }
    func updateMeters() {
        for r in rows {
            let key = ObjectIdentifier(r.ctx), l = r.ctx.model.layout
            if cache[key]?.0 != l { cache[key] = (l, r.ctx.profile.advancedChannels(l).phones) }
            guard let ph = cache[key]?.1, r.n - 1 < ph.count else { continue }
            r.fader.bars[0].db = r.ctx.db(ph[r.n - 1].slots.0); r.fader.bars[1].db = r.ctx.db(ph[r.n - 1].slots.1)
        }
    }
}

/// Console CUE (PFL, user 2026-10-28): the cued channels go to the Aux at 0 dB and Phones 1 listens to the Aux while any cue is on; the
/// Line outs (the console's returns) do not change. Aux send elements: inputs 1-18, then DAW returns 1-4 (= Line 1-4 in the passthrough).
enum ConsoleCue {
    /// The Aux send element of a console row: an input's mixer channel; Line out n (DAW return n) -> 18 + n; nil for outputs that skip the
    /// mixer (ADAT / S/PDIF out are direct)
    static func element(_ c: AdvancedChannel, input: Bool, inputCount: Int) -> Int? {
        if input { return c.slot + 1 }
        return c.group == "Line" && c.ca >= 1 && c.ca <= 4 ? inputCount + c.ca : nil
    }
    static func isOn(_ m: DeviceModel, _ el: Int) -> Bool { return m.element(MAKeys.auxSendLevels, el) > MARange.levelMin }
    static func anyOn(_ m: DeviceModel) -> Bool { return m.ints(MAKeys.auxSendLevels).contains { $0 > MARange.levelMin } }
    /// Cue on / off; the phones' source before the first cue is kept in `store.cuePhonesSource` and given back after the last
    static func set(_ m: DeviceModel, _ el: Int, _ on: Bool, store: AdvancedStore?) {
        let wasAny = anyOn(m)
        m.setElement(MAKeys.auxSendLevels, el, on ? MARange.levelMax : MARange.levelMin)
        if on && !wasAny {
            store?.cuePhonesSource = Int(m.number(MAKeys.headphoneSourceV2) ?? 1)
            for ch in [1, 2] { m.setElement(MAKeys.auxOutputLevels, ch, MARange.levelMax); m.setElement(MAKeys.auxOutputMute, ch, 0) }
            m.setNumber(MAKeys.headphoneSourceV2, UInt64(MixBus.aux.phonesMask))
        }
        if !on && !anyOn(m) {
            m.setNumber(MAKeys.headphoneSourceV2, UInt64(store?.cuePhonesSource ?? 1))
            store?.cuePhonesSource = nil
        }
        m.commit()
    }
}

final class AdvancedConsoleView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let ctx: PanelContext
    private let store: AdvancedStore
    private let lock = ChipView(), refChip = ChipView(), clock = ChipView(), ports = ChipView()
    private let restore = FlatButton("Restore 0 dB")
    private let refMenu = NSPopUpButton(frame: .zero, pullsDown: false)
    private let ins = ConsoleTable(caption: "Console \u{2192} DAW", note: "your console's labels \u{00B7} click a label to edit", input: true)
    private let outs = ConsoleTable(caption: "DAW \u{2192} console", note: "Line 1\u{2013}4 through the mixer at 0 dB, digital out direct", input: false)
    private let phones: ConsolePhonesBar
    private var layoutKey: ChannelLayout?
    private var channels = AdvancedChannels.none

    init(context: PanelContext, store: AdvancedStore) {
        ctx = context; self.store = store
        phones = ConsolePhonesBar(units: [ConsolePhonesBar.Unit(label: "", ctx: context)])
        super.init(frame: .zero)
        [lock, restore, refChip, refMenu, clock, ports, ins, outs].forEach { addSubview($0) }
        refChip.text = "Meters: 0 VU in dBFS"; refChip.extraWidth = 66
        refMenu.addItems(withTitles: AdvancedStore.references.map { "\($0)" })
        refMenu.controlSize = .small
        refMenu.font = Theme.font(11)
        refMenu.target = self; refMenu.action = #selector(refChanged)
        restore.onAction = { [weak self] in
            guard let me = self else { return }
            me.ctx.model.restore(AdvancedScene.unity(from: me.ctx.model.cache))
        }
        outs.extraHeight = phones.preferredHeight
        outs.extra = phones
        rebuild()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    @objc private func refChanged() {
        let r = AdvancedStore.references[max(0, refMenu.indexOfSelectedItem)]
        store.meterReference = r
        applyReference()
    }
    private func applyReference() {
        let r = store.meterReference
        ins.scale.reference = r; outs.scale.reference = r
        (ins.rows + outs.rows).forEach { $0.meter.reference = r }
        refMenu.selectItem(at: AdvancedStore.references.index(of: r) ?? 1)
    }

    private func rebuild() {
        let l = ctx.model.layout
        layoutKey = l
        channels = ctx.profile.advancedChannels(l)
        let key = store.viewKey, hidden = store.hiddenIDs(key)
        func row(_ c: AdvancedChannel, _ input: Bool) -> ConsoleRow {
            let r = ConsoleRow(c, name: store.name(c), input: input)
            r.onRename = { [weak self] t in self?.store.setName(t, for: c.id) }
            r.cueElement = ConsoleCue.element(c, input: input, inputCount: ctx.model.ints(MAKeys.inputLevels).count)
            r.cue.onClick = { [weak self, weak r] b in
                guard let me = self, let el = r?.cueElement else { return }
                ConsoleCue.set(me.ctx.model, el, b.isOn, store: me.store); me.refresh()
            }
            r.contextMenu = { [weak self] in
                guard let me = self else { return nil }
                return hideChannelMenu(name: me.store.name(c), hiddenCount: hidden.count,
                                       hide: { me.store.setHidden(c.id, true, in: key); me.rebuild(); me.refresh() },
                                       showAll: { me.store.showAll(in: key); me.rebuild(); me.refresh() })
            }
            return r
        }
        ins.setRows(channels.inputs.filter { !hidden.contains($0.id) }.map { row($0, true) })
        outs.setRows(channels.outputs.filter { !hidden.contains($0.id) }.map { row($0, false) })
        applyReference()
        layoutAll()
    }

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutAll() }
    private func layoutAll() {
        let p = Theme.pad, w = bounds.width, h = bounds.height
        var x = p
        func place(_ v: NSView, _ width: CGFloat) { v.frame = NSRect(x: x, y: p, width: width, height: 32); x += width + 10 }
        place(lock, lock.preferredWidth)
        if !restore.isHidden { restore.frame = NSRect(x: x - 4, y: p + 3, width: restore.intrinsicContentSize.width, height: 26); x += restore.frame.width + 10 }
        place(refChip, refChip.preferredWidth)
        refMenu.frame = NSRect(x: refChip.frame.maxX - 70, y: p + 5, width: 62, height: 22)
        place(clock, clock.preferredWidth)
        place(ports, ports.preferredWidth)
        let top = p + 32 + 10, tw = (w - 2 * p - 14) / 2
        ins.frame = NSRect(x: p, y: top, width: tw, height: h - top - p)
        outs.frame = NSRect(x: p + tw + 14, y: top, width: tw, height: h - top - p)
    }

    func refresh() {
        let m = ctx.model
        if layoutKey != m.layout { rebuild() }
        let off = AdvancedScene.offUnity(m.cache)
        lock.dot = off.isEmpty ? Theme.green : Theme.solo
        lock.warn = !off.isEmpty
        lock.text = off.isEmpty ? "Unity: every interface level at 0 dB" : "Not at unity: " + off.joined(separator: ", ")
        restore.isHidden = off.isEmpty
        let syncTitle = ctx.profile.hardware.sync.first(where: { $0.sync == m.clockSource.sync })?.title ?? ""
        let external = m.clockSource.sync == .externalDigital || m.clockSource.sync == .wordClock
        clock.text = "Clock: " + syncTitle + (m.rateLocked ? " \u{00B7} locked" : " \u{00B7} not locked")
        clock.dot = m.rateLocked ? Theme.green : Theme.red
        clock.warn = !external && m.clockSource.adatIn
        if clock.warn { clock.text += " \u{00B7} with an ADAT converter as master, use External" }
        let cs = m.clockSource
        ports.text = "Optical: " + (cs.adatIn ? "ADAT" : "S/PDIF") + " in \u{00B7} " + (cs.adatOut ? "ADAT" : "S/PDIF") + " out"
        ports.dot = nil
        phones.refresh()
        for r in ins.rows + outs.rows { if let el = r.cueElement { r.cue.isOn = ConsoleCue.isOn(m, el) } }
        layoutAll()
    }

    func updateMeters() {
        for r in ins.rows { r.update(ctx.db(r.channel.slot)) }
        for r in outs.rows { r.update(ctx.db(r.channel.slot)) }
        phones.updateMeters()
    }
}
