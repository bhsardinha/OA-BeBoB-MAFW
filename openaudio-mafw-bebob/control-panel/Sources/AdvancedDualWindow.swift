// Advanced window, Dual (PANEL_REDESIGN_PLAN_v3 §4, phase M7; user 2026-10-27): two FW1814s as one macOS aggregate, an extra next to each
// unit's own Advanced window, DAW and Console only. The views show both units with the aggregate's channel numbers (unit A first: inputs
// 1-16 A, 17-32 B; outputs 1-12 A, 13-24 B in ADAT mode). A mode switch applies the mode's scene to both units (each through its own
// AdvancedWindowController and store) and asks once when an output of either unit would get louder. The Dual setup sheet checks the rate,
// A's and B's clocks, the aggregate and the buses (AdvancedDual.swift) and offers the fixes the user presses: B to word clock, create /
// recreate the aggregate, swap A and B. It never changes the rate.
import AppKit

/// One unit of the pair, with its aggregate channel offsets.
struct DualUnit {
    let label: String                 // "A" / "B"
    let controller: PanelWindowController
    var ctx: PanelContext { return controller.ctx }
    var advanced: AdvancedWindowController? { return controller.advancedController() }
    var tail: String { return String(controller.device.info.guid.suffix(6)) }
    let inOffset: Int, outOffset: Int
}

/// One tall meter of the Dual DAW rows: peak readout, clip latch, the bar(s), name and Core Audio channel number. Nothing else. No subviews:
/// it draws its texts and latch itself and on a tick redraws only the readout or latch rectangle (8-bit rule, as AdvMeterColumn).
final class BigMeterColumn: FlippedView {
    let slots: [Int]
    private(set) var barRects: [NSRect] = []         // in this view's coordinates; the row's MeterField draws them
    private(set) var values: [Int]                   // the last tick's dB per bar
    private var held = -64, heldStamp: CFAbsoluteTime = 0, shownPeak = Int.min
    private var name: String, number: String, peakText = ""
    private var latchLit = false, latchRect = NSRect.zero
    var onRename: (() -> Void)?
    var contextMenu: (() -> NSMenu?)?
    override func menu(for event: NSEvent) -> NSMenu? { return contextMenu?() }
    static let top: CGFloat = 20, bottom: CGFloat = 30            // room above and under the bars (peak / latch, name / number)

    init(slots: [Int], name: String, number n: String) {
        self.slots = slots
        values = slots.map { _ in -64 }
        self.name = name; number = n
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
        Theme.draw(peakText, in: peakRect, font: Theme.mono(10), color: Theme.text2)
        Theme.fill(latchRect, radius: 1.5, latchLit ? Theme.meterHigh : Theme.ledOff)
        Theme.draw(name, in: NSRect(x: 0, y: h - 27, width: w, height: 14), font: Theme.font(10.5, .semibold), color: Theme.text)
        Theme.draw(number, in: NSRect(x: 0, y: h - 13, width: w, height: 12), font: Theme.mono(9.5), color: Theme.text3)
    }

    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        let w = s.width
        latchRect = NSRect(x: ((w - min(w - 6, 26)) / 2).rounded(), y: 14, width: min(w - 6, 26), height: 3)
        let k = slots.count
        let bw = max(6, min(16, ((w - 10) / CGFloat(k) - 2).rounded(.down)))
        let total = CGFloat(k) * bw + CGFloat(k - 1) * 2
        barRects = (0..<k).map {
            NSRect(x: ((w - total) / 2).rounded() + CGFloat($0) * (bw + 2), y: BigMeterColumn.top, width: bw, height: max(20, s.height - BigMeterColumn.top - BigMeterColumn.bottom))
        }
    }
    /// Reads this column's slots; the readout holds the peak 1.2 s, then follows (it redraws only when the held value changes).
    func update(_ db: (Int) -> Int) {
        values = slots.map(db)
        let top = values.max() ?? -64, now = CFAbsoluteTimeGetCurrent()
        if top >= 0 && !latchLit { latchLit = true; setNeedsDisplay(latchRect) }
        if top >= held || now - heldStamp > 1.2 { held = top; heldStamp = now }
        if held != shownPeak { shownPeak = held; peakText = held <= -64 ? "" : "\(held)"; setNeedsDisplay(peakRect) }
    }
    override func mouseDown(with e: NSEvent) {
        let p = convert(e.locationInWindow, from: nil)
        if e.clickCount == 2, p.y > bounds.height - BigMeterColumn.bottom { onRename?() }
        else if latchLit, latchRect.insetBy(dx: -4, dy: -6).contains(p) { latchLit = false; setNeedsDisplay(latchRect) }   // click clears the latch
    }
}

/// A full-width row of big meters for both units (user, 2026-10-28): two label lines above the meters, both centred over what they name with
/// a rule each side (――― ADAT ―――): first the row and unit ("A · INPUTS"), then the groups (ANALOG, ADAT, LINE, PHONES). One dB scale.
final class BigMeterRow: FlippedView {
    struct Group { let title: String; let columns: [BigMeterColumn] }
    struct Block { let unit: String; let groups: [Group] }
    private let caption: String
    private(set) var blocks: [Block] = []
    private let scale = MeterScale(frame: .zero)
    private let field = MeterField(frame: .zero)                     // every bar of the row, both units, one image
    private var unitSpans: [(String, CGFloat, CGFloat)] = []     // unit, first column's left, last column's right
    private var groupSpans: [(String, CGFloat, CGFloat)] = []
    static let head: CGFloat = 40

    init(caption: String) { self.caption = caption; super.init(frame: .zero); addSubview(field); addSubview(scale) }
    required init?(coder: NSCoder) { fatalError("unused") }

    func setBlocks(_ b: [Block]) {
        blocks.forEach { $0.groups.forEach { $0.columns.forEach { $0.removeFromSuperview() } } }
        blocks = b
        b.forEach { $0.groups.forEach { $0.columns.forEach { addSubview($0) } } }
        addSubview(field)                       // back on top of the (opaque) columns; it lets clicks through
        layoutColumns()
    }
    override func setFrameSize(_ s: NSSize) { super.setFrameSize(s); layoutColumns() }

    private func layoutColumns() {
        let scaleW: CGFloat = 22, gap: CGFloat = 3, groupGap: CGFloat = 12, unitGap: CGFloat = 28
        let n = blocks.reduce(0) { $0 + $1.groups.reduce(0) { $0 + $1.columns.count } }
        let groups = blocks.reduce(0) { $0 + $1.groups.count }
        let fixed = 12 + scaleW + 6 + CGFloat(max(0, n - groups)) * gap + CGFloat(max(0, groups - blocks.count)) * groupGap + CGFloat(max(0, blocks.count - 1)) * unitGap + 12
        let cw = max(22, min(64, ((bounds.width - fixed) / CGFloat(max(1, n))).rounded(.down)))
        let colTop = BigMeterRow.head, colH = bounds.height - colTop - 8
        scale.frame = NSRect(x: 12, y: colTop + BigMeterColumn.top, width: scaleW, height: max(10, colH - BigMeterColumn.top - BigMeterColumn.bottom + MeterBar.clipLength + MeterBar.clipGap))
        var x = 12 + scaleW + 6
        unitSpans = []; groupSpans = []
        for (bi, b) in blocks.enumerated() {
            let ustart = x
            for (gi, g) in b.groups.enumerated() {
                let gstart = x
                for (ci, c) in g.columns.enumerated() { c.frame = NSRect(x: x, y: colTop, width: cw, height: colH); x += cw + (ci < g.columns.count - 1 ? gap : 0) }
                groupSpans.append((g.title, gstart, x))
                if gi < b.groups.count - 1 { x += groupGap }
            }
            unitSpans.append((b.unit, ustart, x))
            if bi < blocks.count - 1 { x += unitGap }
        }
        field.frame = bounds
        field.setBars(columns.flatMap { c in c.barRects.map { field.convert($0, from: c) } })
        needsDisplay = true
    }
    private var columns: [BigMeterColumn] { return blocks.flatMap { $0.groups.flatMap { $0.columns } } }
    /// After the columns' update: their values to the field, in bar order.
    func flush() { field.update(columns.flatMap { $0.values }) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 10, Theme.surface)
        for (u, a, b) in unitSpans { drawRuleLabel("\(u) \u{00B7} \(caption)", a, b, y: 12, font: Theme.font(10, .bold), color: Theme.text) }
        for (t, a, b) in groupSpans where !t.isEmpty { drawRuleLabel(t, a, b, y: 29, font: Theme.font(8.5, .semibold), color: Theme.text3) }
        if unitSpans.count > 1 {                                          // the split between the units
            Theme.hairline.setFill()
            for i in 0..<(unitSpans.count - 1) { NSRect(x: (unitSpans[i].2 + unitSpans[i + 1].1) / 2, y: 6, width: 1, height: bounds.height - 14).fill() }
        }
    }
}

/// The DAW view for both units, made for Full HD (user, 2026-10-28): one row of all inputs (A then B), one row of all outputs and phones,
/// the meters as tall as the window allows; a narrow column with unit A's monitor knob and the four phones. Labels: name and channel only.
final class DualDAWView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let units: [DualUnit]
    private let inRow = BigMeterRow(caption: "Inputs"), outRow = BigMeterRow(caption: "Outputs")
    private var cols: [(col: BigMeterColumn, ctx: PanelContext)] = []
    private let monitorCard = Card(title: "Monitor"), phonesCard = Card(title: "Phones")
    /// Per unit: its monitor knob (Line 1/2, or Line 3/4 with speakers B), value and speakers A / B (the single DAW's control room, twice)
    private var monitors: [(unit: DualUnit, name: NSTextField, knob: MonitorKnob, value: NSTextField, speakers: SegmentedBar)] = []
    private var phones: [(unit: DualUnit, n: Int, label: NSTextField, source: SegmentedBar, fader: HMeterFader, value: NSTextField, mute: PillButton)] = []
    /// Each unit's phones channels, kept until its layout changes: the meter tick must not rebuild the channel list (GPU-first rule)
    private var phoneCache: [String: (layout: ChannelLayout, phones: [AdvancedPhones])] = [:]
    private func phoneChannels(_ u: DualUnit) -> [AdvancedPhones] {
        let l = u.ctx.model.layout
        if let c = phoneCache[u.label], c.layout == l { return c.phones }
        let p = u.ctx.profile.advancedChannels(l).phones
        phoneCache[u.label] = (l, p)
        return p
    }

    init(units: [DualUnit]) {
        self.units = units
        super.init(frame: .zero)
        [inRow, outRow, monitorCard, phonesCard].forEach { addSubview($0) }
        for u in units {
            let name = makeLabel("Unit \(u.label)", size: 11, weight: .semibold, color: Theme.text2, align: .center)
            let k = MonitorKnob()
            let v = makeLabel("", size: 13, weight: .semibold, color: Theme.text, align: .center)
            let sp = SegmentedBar(items: [.text("A 1/2"), .text("B 3/4")], segmentWidth: 46,
                                  tooltips: ["Speakers A \u{00B7} DAW 1/2 on Line 1/2", "Speakers B \u{00B7} DAW 1/2 on Line 3/4, Line 1/2 muted"])
            k.onChange = { [weak v] val in u.ctx.model.setLevel(MAKeys.outputLevels, channel: Speakers.monitorChannel(u.ctx.model), value: val, linked: true); v?.stringValue = Theme.levelText(val) + " dB" }
            k.onCommit = { u.ctx.model.commit() }
            sp.onSelect = { [weak self] i in Speakers.set(u.ctx.model, b: i == 1); self?.refresh() }
            [name, k, v, sp].forEach { monitorCard.addSubview($0) }
            monitors.append((u, name, k, v, sp))
        }
        for u in units {
            for n in 1...2 {
                let o = u.outOffset
                let src = SegmentedBar(items: [.text("\(o + 1)/\(o + 2)"), .text("\(o + 3)/\(o + 4)")], segmentWidth: 58,
                                       tooltips: ["DAW \(o + 1)/\(o + 2)", "DAW \(o + 3)/\(o + 4) (cue)"])
                let f = HMeterFader(frame: .zero)
                let v = makeLabel("", size: 10.5, color: Theme.text2, align: .right); v.font = Theme.mono(10.5)
                let m = PillButton("M"); m.solid = true; m.onColor = Theme.mute
                let l = makeLabel("\(u.label)\(n)", size: 11.5, weight: .bold, color: Theme.text)
                src.onSelect = { i in u.ctx.model.setPhonesSource(phones: n, mask: i == 0 ? 1 : 2) }
                f.onChange = { [weak v] val in u.ctx.model.setLevel(MAKeys.headphoneLevels, channel: 2 * n - 1, value: val, linked: true); v?.stringValue = Theme.levelText(val, law: .monitor) }
                f.onCommit = { u.ctx.model.commit() }
                m.onClick = { b in
                    u.ctx.model.setElement(MAKeys.headphoneMute, 2 * n - 1, b.isOn ? 1 : 0); u.ctx.model.setElement(MAKeys.headphoneMute, 2 * n, b.isOn ? 1 : 0); u.ctx.model.commit()
                }
                [l, src, f, v, m].forEach { phonesCard.addSubview($0) }
                phones.append((u, n, l, src, f, v, m))
            }
        }
        rebuild()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func rename(_ u: DualUnit, _ c: AdvancedChannel, _ col: BigMeterColumn) {
        guard let w = window, let store = u.advanced?.store else { return }
        let a = NSAlert()
        a.messageText = "Name for unit \(u.label) \u{00B7} \(c.jack)"
        let f = NSTextField(frame: NSRect(x: 0, y: 0, width: 240, height: 24)); f.stringValue = store.name(c)
        a.accessoryView = f; a.window.initialFirstResponder = f
        a.addButton(withTitle: "Rename"); a.addButton(withTitle: "Cancel")
        a.beginSheetModal(for: w) { [weak col] r in if r == .alertFirstButtonReturn { store.setName(f.stringValue, for: c.id); col?.setName(store.name(c)) } }
    }

    /// Hidden channels of the Dual DAW view (a view option, per unit; plan v2 §3)
    static let viewKey = "dual-daw"
    private func rebuild() {
        cols = []
        let key = DualDAWView.viewKey
        let hiddenCount = units.reduce(0) { $0 + ($1.advanced?.store.hiddenIDs(key).count ?? 0) }
        func groups(_ u: DualUnit, _ all: [AdvancedChannel], offset: Int) -> [BigMeterRow.Group] {
            var out: [BigMeterRow.Group] = []
            let hidden = u.advanced?.store.hiddenIDs(key) ?? []
            for c in all where !hidden.contains(c.id) {
                let v = BigMeterColumn(slots: [c.slot], name: u.advanced?.store.name(c) ?? c.name, number: "\(c.ca + offset)")
                v.onRename = { [weak self, weak v] in if let me = self, let col = v { me.rename(u, c, col) } }
                v.contextMenu = { [weak self] in
                    guard let me = self else { return nil }
                    return hideChannelMenu(name: "\(u.label) \u{00B7} " + (u.advanced?.store.name(c) ?? c.name), hiddenCount: hiddenCount,
                                           hide: { u.advanced?.store.setHidden(c.id, true, in: key); me.rebuild(); me.refresh() },
                                           showAll: { me.units.forEach { $0.advanced?.store.showAll(in: key) }; me.rebuild(); me.refresh() })
                }
                cols.append((v, u.ctx))
                let title = c.group.replacingOccurrences(of: " out", with: "")
                if let last = out.last, last.title == title { out[out.count - 1] = BigMeterRow.Group(title: title, columns: last.columns + [v]) }
                else { out.append(BigMeterRow.Group(title: title, columns: [v])) }
            }
            return out
        }
        inRow.setBlocks(units.map { u in BigMeterRow.Block(unit: u.label, groups: groups(u, u.ctx.profile.advancedChannels(u.ctx.model.layout).inputs, offset: u.inOffset)) })
        outRow.setBlocks(units.map { u in
            let ch = u.ctx.profile.advancedChannels(u.ctx.model.layout)
            let ph = ch.phones.map { p -> BigMeterColumn in
                let v = BigMeterColumn(slots: [p.slots.0, p.slots.1], name: "Ph \(p.number)", number: "")
                cols.append((v, u.ctx)); return v
            }
            return BigMeterRow.Block(unit: u.label, groups: groups(u, ch.outputs, offset: u.outOffset) + [BigMeterRow.Group(title: "Phones", columns: ph)])
        })
        layoutAll()
    }

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutAll() }
    private func layoutAll() {
        let p: CGFloat = 12, w = bounds.width, h = bounds.height, crW: CGFloat = 236
        let left = w - 3 * p - crW, rowH = ((h - 2 * p - 8) / 2).rounded(.down)
        inRow.frame = NSRect(x: p, y: p, width: left, height: rowH)
        outRow.frame = NSRect(x: p, y: p + rowH + 8, width: left, height: rowH)
        let x = w - p - crW
        monitorCard.frame = NSRect(x: x, y: p, width: crW, height: 214)
        let half = crW / 2
        for (i, m) in monitors.enumerated() {
            let cx = half * CGFloat(i) + half / 2
            m.name.frame = NSRect(x: cx - 50, y: 34, width: 100, height: 15)
            m.knob.frame = NSRect(x: cx - 44, y: 52, width: 88, height: 88)
            m.value.frame = NSRect(x: cx - 55, y: 144, width: 110, height: 18)
            m.speakers.setFrameOrigin(NSPoint(x: (cx - m.speakers.frame.width / 2).rounded(), y: 172))
        }
        phonesCard.frame = NSRect(x: x, y: p + 214 + 8, width: crW, height: min(h - 2 * p - 222, 4 * 60 + 40))
        for (i, r) in phones.enumerated() {
            let y = 36 + CGFloat(i) * 60
            r.label.frame = NSRect(x: 14, y: y + 3, width: 30, height: 16)
            r.source.setFrameOrigin(NSPoint(x: crW - 14 - r.source.frame.width, y: y - 4))
            r.fader.frame = NSRect(x: 14, y: y + 30, width: crW - 14 - 14 - 40 - 26, height: 18)
            r.value.frame = NSRect(x: crW - 14 - 26 - 40, y: y + 31, width: 38, height: 16)
            r.mute.frame = NSRect(x: crW - 14 - 22, y: y + 29, width: 22, height: 20)
            r.fader.placeCap()
        }
    }

    func refresh() {
        for m in monitors {
            let model = m.unit.ctx.model
            m.knob.value = model.faderLevel(MAKeys.outputLevels, channel: Speakers.monitorChannel(model))
            m.value.stringValue = Theme.levelText(m.knob.value) + " dB"
            m.speakers.selected = Speakers.isB(model) ? 1 : 0
        }
        for r in phones {
            let m = r.unit.ctx.model
            r.source.selected = Int(m.number(r.n == 1 ? MAKeys.headphoneSourceV2 : MAKeys.headphoneSecondSource) ?? 1) == 2 ? 1 : 0
            r.fader.value = m.faderLevel(MAKeys.headphoneLevels, channel: 2 * r.n - 1); r.fader.placeCap()
            r.value.stringValue = Theme.levelText(r.fader.value, law: .monitor)
            r.mute.isOn = m.element(MAKeys.headphoneMute, 2 * r.n - 1) != 0
        }
    }

    func updateMeters() {
        for c in cols { let ctx = c.ctx; c.col.update { ctx.db($0) } }
        inRow.flush(); outRow.flush()
        for r in phones {
            let ch = phoneChannels(r.unit)
            guard r.n - 1 < ch.count else { continue }
            let s = ch[r.n - 1].slots
            r.fader.bars[0].db = r.unit.ctx.db(s.0); r.fader.bars[1].db = r.unit.ctx.db(s.1)
        }
    }
}

/// The Console view for both units: Console -> DAW 1-16 (A), 17-32 (B), DAW 1-24 -> console; unity watch over both; reference; clock.
final class DualConsoleView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let units: [DualUnit]
    private let lock = ChipView(), refChip = ChipView(), clock = ChipView()
    private let restore = FlatButton("Restore 0 dB")
    private let refMenu = NSPopUpButton(frame: .zero, pullsDown: false)
    private var tables: [ConsoleTable] = []
    private var rows: [(row: ConsoleRow, ctx: PanelContext)] = []
    private let phones: ConsolePhonesBar                              // A1, A2, B1, B2 under DAW -> console

    init(units: [DualUnit]) {
        self.units = units
        phones = ConsolePhonesBar(units: units.map { ConsolePhonesBar.Unit(label: $0.label, ctx: $0.ctx) })
        super.init(frame: .zero)
        [lock, restore, refChip, refMenu, clock].forEach { addSubview($0) }
        refChip.text = "Meters: 0 VU in dBFS"; refChip.extraWidth = 66
        refMenu.addItems(withTitles: AdvancedStore.references.map { "\($0)" })
        refMenu.controlSize = .small; refMenu.font = Theme.font(11)
        refMenu.target = self; refMenu.action = #selector(refChanged)
        restore.onAction = { [weak self] in self?.units.forEach { $0.ctx.model.restore(AdvancedScene.unity(from: $0.ctx.model.cache)) } }
        rebuild()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private var reference: Int { return units[0].advanced?.store.meterReference ?? -18 }
    @objc private func refChanged() {
        units[0].advanced?.store.meterReference = AdvancedStore.references[max(0, refMenu.indexOfSelectedItem)]
        applyReference()
    }
    private func applyReference() {
        tables.forEach { $0.scale.reference = reference }
        rows.forEach { $0.row.meter.reference = reference }
        refMenu.selectItem(at: AdvancedStore.references.index(of: reference) ?? 1)
    }

    static let viewKey = "dual-console"
    private func rebuild() {
        tables.forEach { $0.removeFromSuperview() }
        tables = []; rows = []
        let key = DualConsoleView.viewKey
        let hiddenCount = units.reduce(0) { $0 + ($1.advanced?.store.hiddenIDs(key).count ?? 0) }
        func shown(_ u: DualUnit, _ list: [AdvancedChannel]) -> [AdvancedChannel] {
            let h = u.advanced?.store.hiddenIDs(key) ?? []
            return list.filter { !h.contains($0.id) }
        }
        func row(_ u: DualUnit, _ c: AdvancedChannel, input: Bool) -> ConsoleRow {
            let store = u.advanced?.store
            let shifted = AdvancedChannel(id: c.id, name: c.name, jack: input ? c.jack : "\(u.label) \u{00B7} " + c.jack.replacingOccurrences(of: " out", with: ""),
                                          ca: c.ca + (input ? u.inOffset : u.outOffset), slot: c.slot, group: c.group)
            let r = ConsoleRow(shifted, name: store?.name(c) ?? c.name, input: input)
            r.onRename = { t in store?.setName(t, for: c.id) }
            r.cueElement = ConsoleCue.element(c, input: input, inputCount: u.ctx.model.ints(MAKeys.inputLevels).count)
            r.cue.onClick = { [weak self, weak r] b in
                guard let el = r?.cueElement else { return }
                ConsoleCue.set(u.ctx.model, el, b.isOn, store: store); self?.refresh()
            }
            r.contextMenu = { [weak self] in
                guard let me = self else { return nil }
                return hideChannelMenu(name: "\(u.label) \u{00B7} " + (store?.name(c) ?? c.name), hiddenCount: hiddenCount,
                                       hide: { store?.setHidden(c.id, true, in: key); me.rebuild(); me.refresh() },
                                       showAll: { me.units.forEach { $0.advanced?.store.showAll(in: key) }; me.rebuild(); me.refresh() })
            }
            rows.append((r, u.ctx))
            return r
        }
        for u in units {
            let ch = u.ctx.profile.advancedChannels(u.ctx.model.layout)
            let t = ConsoleTable(caption: "Console \u{2192} DAW \(u.inOffset + 1)\u{2013}\(u.inOffset + ch.inputs.count)", note: "unit \(u.label) \u{00B7} \(u.tail)", input: true)
            t.widths = ConsoleRow.compact
            t.setRows(shown(u, ch.inputs).map { row(u, $0, input: true) })
            tables.append(t); addSubview(t)
        }
        let outs = ConsoleTable(caption: "DAW \u{2192} console", note: "A first, then B", input: false)
        outs.widths = ConsoleRow.compact
        outs.setRows(units.flatMap { u in shown(u, u.ctx.profile.advancedChannels(u.ctx.model.layout).outputs).map { row(u, $0, input: false) } })
        outs.extraHeight = phones.preferredHeight
        outs.extra = phones
        tables.append(outs); addSubview(outs)
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
        place(clock, min(clock.preferredWidth, w - x - p))
        let top = p + 42, tw = (w - 2 * p - 2 * 12) / 3
        for (i, t) in tables.enumerated() { t.frame = NSRect(x: p + CGFloat(i) * (tw + 12), y: top, width: tw, height: h - top - p) }
    }

    func refresh() {
        var off: [String] = []
        for u in units { for o in AdvancedScene.offUnity(u.ctx.model.cache) { off.append(u.label + " " + o) } }
        lock.dot = off.isEmpty ? Theme.green : Theme.solo
        lock.warn = !off.isEmpty
        if off.isEmpty { lock.text = "Unity: both units at 0 dB" }
        else {
            let more: String = off.count > 4 ? "\u{2026}" : ""
            lock.text = "Not at unity: " + off.prefix(4).joined(separator: ", ") + more
        }
        restore.isHidden = off.isEmpty
        let a = units[0].ctx.model, b = units[1].ctx.model
        let bOK = b.clockSource.sync == .wordClock && b.rateLocked
        var aName = "internal"
        if a.clockSource.sync == .wordClock { aName = "word clock" } else if a.clockSource.sync == .externalDigital { aName = "external" }
        let bName: String = b.clockSource.sync == .wordClock ? "word clock" : "not on word clock"
        let lockedText: String = bOK ? " \u{00B7} locked" : ""
        clock.text = "Clock: A \(aName) \u{2192} B \(bName)\(lockedText)"
        clock.dot = bOK ? Theme.green : Theme.solo
        clock.warn = !bOK
        phones.refresh()
        for r in rows { if let el = r.row.cueElement { r.row.cue.isOn = ConsoleCue.isOn(r.ctx.model, el) } }
        layoutAll()
    }

    func updateMeters() { for r in rows { r.row.update(r.ctx.db(r.row.channel.slot)) }; phones.updateMeters() }
}

/// The Dual setup sheet: the checklist with its fixes.
final class DualSetupView: FlippedView {
    var items: [DualSetup.Item] = [] { didSet { rebuildButtons(); needsDisplay = true } }
    var onFix: ((DualSetup.Fix) -> Void)?
    var onSwap: (() -> Void)?, onRemove: (() -> Void)?, onDone: (() -> Void)?
    var title = "" { didSet { needsDisplay = true } }
    private var fixButtons: [FlatButton] = []
    private let swap = FlatButton("Swap A and B"), remove = FlatButton("Remove aggregate"), done = FlatButton("Done")

    override init(frame: NSRect) {
        super.init(frame: frame)
        done.primary = true; done.keyEquivalent = "\r"
        swap.onAction = { [weak self] in self?.onSwap?() }
        remove.onAction = { [weak self] in self?.onRemove?() }
        done.onAction = { [weak self] in self?.onDone?() }
        [swap, remove, done].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func rebuildButtons() {
        fixButtons.forEach { $0.removeFromSuperview() }
        fixButtons = items.enumerated().compactMap { (i, it) -> FlatButton? in
            guard let f = it.fix else { return nil }
            let b = FlatButton(f == .wordClockB ? "Set B to Word clock" : (f == .createAggregate ? "Create aggregate" : "Recreate aggregate"))
            b.onAction = { [weak self] in self?.onFix?(f) }
            b.frame = NSRect(x: bounds.width - 24 - b.intrinsicContentSize.width, y: 64 + CGFloat(i) * 52 + 8, width: b.intrinsicContentSize.width, height: 26)
            addSubview(b)
            return b
        }
        let y = bounds.height - 48
        done.frame = NSRect(x: bounds.width - 24 - 80, y: y, width: 80, height: 28)
        remove.frame = NSRect(x: done.frame.minX - 10 - remove.intrinsicContentSize.width, y: y, width: remove.intrinsicContentSize.width, height: 28)
        swap.frame = NSRect(x: 24, y: y, width: swap.intrinsicContentSize.width, height: 28)
    }

    override var isOpaque: Bool { return true }
    override func draw(_ dirtyRect: NSRect) {
        Theme.hex(0x1D1F23).setFill(); bounds.fill()
        Theme.draw(title, in: NSRect(x: 24, y: 18, width: bounds.width - 48, height: 22), font: Theme.font(15, .semibold), color: Theme.text, align: .left)
        for (i, it) in items.enumerated() {
            let y = 64 + CGFloat(i) * 52
            var mark = "i"
            var c: NSColor = Theme.text3
            switch it.status {
            case .ok: mark = "\u{2713}"; c = Theme.green
            case .fail: mark = "\u{2715}"; c = Theme.mute
            case .warn: mark = "!"; c = Theme.solo
            case .info: break
            }
            Theme.draw(mark, in: NSRect(x: 24, y: y, width: 16, height: 20), font: Theme.font(13, .bold), color: c, align: .left)
            let fixW: CGFloat = it.fix == nil ? 0 : 170
            let tw: CGFloat = bounds.width - 72 - fixW
            let a = Theme.attributed(it.text, font: Theme.font(12), color: Theme.text2, align: .left)
            let pa = NSMutableAttributedString(attributedString: a)
            let ps = NSMutableParagraphStyle(); ps.lineBreakMode = .byWordWrapping
            pa.addAttribute(.paragraphStyle, value: ps, range: NSRange(location: 0, length: pa.length))
            pa.draw(with: NSRect(x: 48, y: y + 1, width: tw, height: 44), options: [.usesLineFragmentOrigin])
        }
    }
}

final class AdvancedDualWindowController: NSObject, NSWindowDelegate {
    private(set) var units: [DualUnit]
    let window: NSWindow
    private let root = AdvancedRoot(modes: AdvancedMode.dual)
    private var mode: AdvancedMode
    private var bodyView: AdvancedView?
    private var bodyKey = ""
    private var timer: Timer?
    private var sheet: NSWindow?
    private let setupView = DualSetupView(frame: NSRect(x: 0, y: 0, width: 640, height: 420))
    /// demo / snapshot aids
    var demo = false
    var ignoresOcclusion = false
    private static let modeKey = "AdvancedDualMode", unitAKey = "AdvancedDualUnitA"

    init(pair: [PanelWindowController]) {
        // unit A: the one the user chose last ("Swap A and B"), else the lower GUID
        let saved = UserDefaults.standard.string(forKey: AdvancedDualWindowController.unitAKey)
        var p = pair.sorted { $0.device.info.guid < $1.device.info.guid }
        if let s = saved, p[1].device.info.guid == s { p.reverse() }
        units = AdvancedDualWindowController.make(p)
        mode = AdvancedMode(rawValue: UserDefaults.standard.string(forKey: AdvancedDualWindowController.modeKey) ?? "") .flatMap { AdvancedMode.dual.contains($0) ? $0 : nil } ?? .daw
        window = NSWindow(contentRect: NSRect(origin: .zero, size: AdvancedWindowController.contentSize), styleMask: [.titled, .closable, .miniaturizable, .resizable], backing: .buffered, defer: false)
        super.init()
        window.isReleasedWhenClosed = false
        window.delegate = self
        window.title = "Dual FireWire 1814 \u{2014} Advanced"
        styleWindow(window)
        // GPU-first rule, measured 2026-10-28 (Dual DAW demo on the iMac): one layer per view made Core Animation walk ~1000 layers on every
        // meter commit. The static views draw into the root's one layer (once, then only where they change); only the moving parts (meter
        // bars, columns with a peak readout) keep layers of their own, by asking for one explicitly.
        root.wantsLayer = true
        root.canDrawSubviewsIntoLayer = true
        window.contentView = root
        AdvancedWindowController.fit(window)
        root.header.modes.onSelect = { [weak self] i in self?.request(AdvancedMode.dual[i]) }
        root.header.dim.onClick = { [weak self] b in self?.units.forEach { $0.ctx.model.setNumber(MAKeys.globalDim, b.isOn ? 1 : 0); $0.ctx.model.commit() } }
        root.header.mute.onClick = { [weak self] b in self?.units.forEach { $0.ctx.model.setNumber(MAKeys.globalMute, b.isOn ? 1 : 0); $0.ctx.model.commit() } }
        root.bar.onChipClick = { [weak self] in self?.showSetup() }
        setupView.onFix = { [weak self] f in self?.fix(f) }
        setupView.onSwap = { [weak self] in self?.swapUnits() }
        setupView.onRemove = { [weak self] in self?.removeAggregate() }
        setupView.onDone = { [weak self] in self?.closeSetup() }
        hook()
        let t = Timer(timeInterval: 1.0 / 30.0, repeats: true) { [weak self] _ in self?.tick() }
        RunLoop.main.add(t, forMode: .commonModes)
        timer = t
        refresh()
    }

    private static func make(_ p: [PanelWindowController]) -> [DualUnit] {
        let a = p[0], b = p[1]
        let ch = a.device.profile.advancedChannels(a.ctx.model.layout)
        return [DualUnit(label: "A", controller: a, inOffset: 0, outOffset: 0),
                DualUnit(label: "B", controller: b, inOffset: DualSetup.offset(unitB: true, countA: ch.inputs.count), outOffset: DualSetup.offset(unitB: true, countA: ch.outputs.count))]
    }
    private func hook() {
        units.forEach { $0.controller.onRefresh = { [weak self] in self?.refresh() } }
        NotificationCenter.default.removeObserver(self, name: DeviceModel.writeFailed, object: nil)
        units.forEach { NotificationCenter.default.addObserver(self, selector: #selector(writeFailed), name: DeviceModel.writeFailed, object: $0.ctx.model) }
    }
    @objc private func writeFailed() { if window.isVisible { refresh() } }

    /// One panel window at a time (user, 2026-10-28): told before the Dual window shows, so the app hides the units' windows
    var onExclusive: (() -> Void)?
    func hide() { window.orderOut(nil) }

    func show() {
        onExclusive?()
        if !window.isVisible { placeWindow(window, center: screenCenter()) }
        if available { syncUnitsToMode() }
        refresh()
        window.makeKeyAndOrderFront(nil)
    }
    func close() { timer?.invalidate(); window.orderOut(nil) }

    private func state(_ u: DualUnit) -> DualUnitState {
        let m = u.ctx.model
        return DualUnitState(guid: u.controller.device.info.guid, rate: m.sampleRate, sync: m.clockSource.sync, locked: m.rateLocked, bus: u.controller.device.busID)
    }
    var available: Bool { return DualSetup.available(state(units[0]), state(units[1])) }
    private func uid(_ u: DualUnit) -> String? { return demo ? "demo-\(u.controller.device.info.guid)" : DualAggregate.unitUID(guid: u.controller.device.info.guid) }
    private func checklist() -> [DualSetup.Item] {
        return DualSetup.check(state(units[0]), state(units[1]), uidA: uid(units[0]), uidB: uid(units[1]), aggregate: demo ? .missing : DualAggregate.state())
    }

    // MARK: modes

    /// On opening: each unit's first Advanced opening (its mixer kept as its Monitor scene), then both in the Dual mode.
    private func syncUnitsToMode() {
        for u in units { u.advanced?.firstOpenIfNeeded() }
        let pending = units.compactMap { u -> (DualUnit, AdvancedSwitch)? in
            guard let a = u.advanced, a.mode != mode else { return nil }
            return (u, a.plan(mode))
        }
        if !pending.isEmpty { apply(pending, ask: true) }
    }

    func request(_ m: AdvancedMode, ask: Bool = true) {
        guard available else { refresh(); return }
        mode = m
        UserDefaults.standard.set(m.rawValue, forKey: AdvancedDualWindowController.modeKey)
        let pending = units.compactMap { u -> (DualUnit, AdvancedSwitch)? in
            guard let a = u.advanced, a.mode != m else { return nil }
            return (u, a.plan(m))
        }
        apply(pending, ask: ask)
    }

    private func apply(_ pending: [(DualUnit, AdvancedSwitch)], ask: Bool) {
        let rises = pending.flatMap { p in p.1.rises.map { (p.0.label, $0) } }
        let go = { [weak self] in for p in pending { p.0.advanced?.apply(p.1) }; self?.refresh() }
        guard ask, !rises.isEmpty else { go(); return }
        let alert = NSAlert()
        alert.messageText = "Switch both units to \(mode.name)?"
        let law: (String) -> FaderLaw = { $0 == MAKeys.headphoneLevels.name ? .monitor : .console }
        var lines: [String] = []
        for (u, r) in rises {
            var n = "Line out \(r.channel)"
            if r.key == MAKeys.headphoneLevels.name { let side: String = r.channel % 2 == 1 ? "L" : "R"; n = "Phones \((r.channel + 1) / 2) \(side)" }
            let from = Theme.levelText(r.from, law: law(r.key)), to = Theme.levelText(r.to, law: law(r.key))
            lines.append("Unit \(u) \u{00B7} \(n): \(from) \u{2192} \(to) dB")
        }
        alert.informativeText = "These outputs get louder:\n" + lines.joined(separator: "\n") + "\n\nRate and clock are not changed."
        alert.addButton(withTitle: "Switch"); alert.addButton(withTitle: "Cancel")
        alert.beginSheetModal(for: window) { [weak self] r in if r == .alertFirstButtonReturn { go() } else { self?.refresh() } }
    }

    // MARK: setup sheet

    func showSetup() {
        setupView.title = "Dual setup \u{2014} A \(units[0].tail), B \(units[1].tail)"
        setupView.items = checklist()
        let w = NSWindow(contentRect: setupView.frame, styleMask: [.titled], backing: .buffered, defer: false)
        styleWindow(w)
        w.contentView = setupView
        sheet = w
        window.beginSheet(w, completionHandler: nil)
    }
    private func closeSetup() { if let s = sheet { window.endSheet(s); sheet = nil }; refresh() }
    private func refreshSetup() { setupView.items = checklist(); refresh() }

    private func fix(_ f: DualSetup.Fix) {
        switch f {
        case .wordClockB:
            units[1].ctx.model.updateClock { $0.sync = .wordClock }
            DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in self?.refreshSetup() }
        case .createAggregate, .recreateAggregate:
            guard !demo, let a = uid(units[0]), let b = uid(units[1]) else { NSSound.beep(); return }
            if f == .recreateAggregate { DualAggregate.destroy() }
            let st = DualAggregate.create(uidA: a, uidB: b)
            if st != noErr { NSSound.beep() }
            DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in self?.refreshSetup() }
        }
    }
    private func removeAggregate() {
        guard !demo else { NSSound.beep(); return }
        DualAggregate.destroy()
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in self?.refreshSetup() }
    }
    private func swapUnits() {
        units.forEach { $0.controller.onRefresh = nil }
        units = AdvancedDualWindowController.make([units[1].controller, units[0].controller])
        UserDefaults.standard.set(units[0].controller.device.info.guid, forKey: AdvancedDualWindowController.unitAKey)
        hook()
        bodyKey = ""                                     // rebuild the view with the new order
        setupView.title = "Dual setup \u{2014} A \(units[0].tail), B \(units[1].tail)"
        refreshSetup()
    }

    // MARK: view

    func refresh() {
        let h = root.header, bar = root.bar
        let a = units[0].ctx.model, b = units[1].ctx.model
        let chA = units[0].ctx.profile.advancedChannels(a.layout), chB = units[1].ctx.profile.advancedChannels(b.layout)
        h.name = "Dual FireWire 1814"
        h.subtitle = "Advanced \u{00B7} A \(units[0].tail) + B \(units[1].tail) \u{00B7} \(chA.inputs.count + chB.inputs.count) in / \(chA.outputs.count + chB.outputs.count) out"
        let bClock: String = b.clockSource.sync == .wordClock ? "B word clock" : "B not on word clock"
        let lockText: String = a.rateLocked && b.rateLocked ? "both locked" : "not locked"
        let rateA = Theme.rateText(a.sampleRate), rateB = Theme.rateText(b.sampleRate)
        h.rate = "A \(rateA) kHz \u{00B7} B \(rateB) kHz  \u{00B7}  \(bClock)  \u{00B7}  \(lockText)"
        h.locked = a.rateLocked && b.rateLocked && b.clockSource.sync == .wordClock
        h.dim.isOn = (a.number(MAKeys.globalDim) ?? 0) != 0
        h.mute.isOn = (a.number(MAKeys.globalMute) ?? 0) != 0
        h.modesEnabled = available
        h.modes.selected = AdvancedMode.dual.index(of: mode) ?? 0
        let items = checklist(), fails = items.filter { $0.status == .fail }.count
        if available {
            bar.title = mode.name + " \u{00B7} Dual"; bar.summary = ""                // less text (user, 2026-10-28)
        } else {
            bar.title = "Dual"; bar.summary = "both FW1814s at the same rate, 44.1 or 48 kHz"
        }
        bar.state = "Dual setup"
        bar.statusOverride = fails == 0 ? ("\u{2713} ready", Theme.green) : ("\(fails) to fix \u{2014} open", Theme.solo)
        if let u = units.first(where: { $0.ctx.model.writeFailure != nil }), let f = u.ctx.model.writeFailure {
            bar.statusOverride = (String(format: "\u{26A0} unit %@: the driver refused %d setting%@ (0x%x)", u.label, f.keys.count, f.keys.count == 1 ? "" : "s", f.code), Theme.red)
        }
        setBody(available ? mode.rawValue : "lock")
        bodyView?.refresh()
    }

    private func setBody(_ key: String) {
        guard key != bodyKey else { return }
        bodyKey = key
        root.body.subviews.forEach { $0.removeFromSuperview() }
        let v: AdvancedView
        switch key {
        case AdvancedMode.daw.rawValue: v = DualDAWView(units: units)
        case AdvancedMode.console.rawValue: v = DualConsoleView(units: units)
        default:
            let a = units[0].ctx.model, b = units[1].ctx.model
            v = AdvancedMessageView(heading: "Dual works at 44.1 and 48 kHz, both units the same",
                                    text: "Unit A runs at \(Theme.rateText(a.sampleRate)) kHz and unit B at \(Theme.rateText(b.sampleRate)) kHz. Set both to 44.1 or 48 kHz (Audio MIDI Setup, or each unit\u{2019}s standard panel); with B on A\u{2019}s word clock, B follows A. Nothing here changes the rate by itself.")
        }
        bodyView = v
        v.view.frame = root.body.bounds
        root.body.addSubview(v.view)
    }

    private func tick() {
        guard window.isVisible, ignoresOcclusion || benchIgnoresOcclusion || window.occlusionState.contains(.visible) else { return }
        units.forEach { $0.ctx.pollMeters() }
        bodyView?.updateMeters()
    }

    func windowShouldClose(_ sender: NSWindow) -> Bool { window.orderOut(nil); return false }

    // MARK: snapshot aid

    func writeSnapshots(dir: String, prefix: String, png: (NSView, String) -> Void) {
        ignoresOcclusion = true
        show()
        for _ in 0..<16 { RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05)) }
        for m in AdvancedMode.dual {
            request(m, ask: false)
            for _ in 0..<12 { RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05)) }
            if let v = window.contentView { png(v, "\(dir)/\(prefix)-dual-\(m.rawValue).png") }
        }
        setupView.title = "Dual setup \u{2014} A \(units[0].tail), B \(units[1].tail)"
        setupView.items = checklist()
        setupView.layoutSubtreeIfNeeded()
        png(setupView, "\(dir)/\(prefix)-dual-setup.png")
    }
}
