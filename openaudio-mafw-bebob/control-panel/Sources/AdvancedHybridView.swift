// Advanced window, Hybrid mode (PANEL_REDESIGN_PLAN_v3 / v2 §4, phase M5): outboard inserts and reamp from the DAW. The interface mixer is a
// passthrough (the Hybrid scene); the user lists loops (DAW -> send output -> device -> return input -> DAW) and each card shows the send
// and return meters and their difference, for gain matching. A Line-out send must sit at 0 dB to be a calibrated send: the card says
// when it is not and sets it (ADAT / S/PDIF outs are direct, no level). Returns are heard through the DAW, never doubled by the
// interface (the passthrough monitors no input). The DAW measures the loop latency itself (ping).
import AppKit

/// Horizontal meter + held peak readout in one row of a loop card.
final class LoopMeterRow: FlippedView {
    let bars: [MeterBar]
    let slots: [Int]
    private let label = makeLabel("", size: 9.5, weight: .semibold, color: Theme.text3)
    private let value = makeLabel("", size: 10.5, color: Theme.text2, align: .right)
    private(set) var hold = -64
    private var holdStamp: CFAbsoluteTime = 0
    init(_ title: String, slots: [Int]) {
        self.slots = slots
        bars = slots.map { _ in let b = MeterBar(frame: .zero); b.horizontal = true; b.showsClip = false; b.surface = Theme.well; return b }
        super.init(frame: .zero)
        wantsLayer = true                       // one layer for the column: its labels and latch draw into it, only its meter bars have their own
        canDrawSubviewsIntoLayer = true         // (GPU-first rule; a peak change redraws the column, never the card)
        label.stringValue = title.uppercased(); value.font = Theme.mono(10.5)
        [label, value].forEach { addSubview($0) }; bars.forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        label.frame = NSRect(x: 0, y: (s.height - 13) / 2, width: 56, height: 13)
        let w = s.width - 56 - 50, bh: CGFloat = bars.count > 1 ? 4 : 7
        for (i, b) in bars.enumerated() { b.frame = NSRect(x: 56, y: (s.height - CGFloat(bars.count) * (bh + 2)) / 2 + CGFloat(i) * (bh + 2), width: w, height: bh) }
        value.frame = NSRect(x: s.width - 46, y: (s.height - 14) / 2, width: 44, height: 14)
    }
    func update(_ db: (Int) -> Int) {
        var top = -64
        for (i, b) in bars.enumerated() { let v = db(slots[i]); b.db = v; top = max(top, v) }
        let now = CFAbsoluteTimeGetCurrent()
        if top >= hold || now - holdStamp > 1.5 { hold = top; holdStamp = now; value.stringValue = top <= -64 ? "\u{2212}\u{221E}" : "\(top)" }
    }
}

/// One loop: name and device, the path, send and return meters, their difference, the send's unity state, remove.
final class LoopCard: FlippedView {
    let loop: HybridLoop
    let send: LoopMeterRow, ret: LoopMeterRow
    private let delta = makeLabel("\u{2014}", size: 19, weight: .semibold, color: Theme.text, align: .center)
    private let deltaNote = makeLabel("return vs send", size: 10, color: Theme.text3, align: .center)
    let unity = FlatButton("Set send to 0 dB")
    private let unityNote = makeLabel("", size: 10.5, color: Theme.text3)
    let remove = FlatButton("Remove"), edit = FlatButton("Edit")
    private let path: [String]

    init(_ loop: HybridLoop, path: [String], sendSlots: [Int], returnSlots: [Int]) {
        self.loop = loop; self.path = path
        send = LoopMeterRow("Send", slots: sendSlots); ret = LoopMeterRow("Return", slots: returnSlots)
        super.init(frame: .zero)
        delta.font = Theme.mono(19, .semibold)
        delta.wantsLayer = true                 // changes with the meters: its own layer, the card stays a static image
        [send, ret, delta, deltaNote, unity, unityNote, remove, edit].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    /// nil = calibrated (or a direct digital send); else the off-unity text, e.g. "Line 3 at -6.0 dB"
    func setUnity(_ off: String?) {
        unity.isHidden = off == nil
        unityNote.stringValue = off ?? "send at 0 dB"
        unityNote.textColor = off == nil ? Theme.green : Theme.solo
    }
    func updateMeters(_ db: (Int) -> Int) {
        send.update(db); ret.update(db)
        if let d = HybridLoop.difference(send: send.hold, ret: ret.hold) {
            delta.stringValue = (d > 0 ? "+" : (d < 0 ? "\u{2212}" : "")) + "\(abs(d)) dB"
            delta.textColor = abs(d) <= 1 ? Theme.green : Theme.text
        } else { delta.stringValue = "\u{2014}"; delta.textColor = Theme.text3 }
    }
    override func setFrameSize(_ s: NSSize) {
        super.setFrameSize(s)
        let mw = s.width - 24 - 120
        send.frame = NSRect(x: 12, y: 64, width: mw, height: 18)
        ret.frame = NSRect(x: 12, y: 86, width: mw, height: 18)
        delta.frame = NSRect(x: s.width - 120, y: 66, width: 110, height: 24)
        deltaNote.frame = NSRect(x: s.width - 120, y: 90, width: 110, height: 14)
        remove.frame = NSRect(x: s.width - 12 - remove.intrinsicContentSize.width, y: 8, width: remove.intrinsicContentSize.width, height: 24)
        edit.frame = NSRect(x: remove.frame.minX - 8 - edit.intrinsicContentSize.width, y: 8, width: edit.intrinsicContentSize.width, height: 24)
        unity.frame = NSRect(x: edit.frame.minX - 8 - unity.intrinsicContentSize.width, y: 8, width: unity.intrinsicContentSize.width, height: 24)
        unityNote.frame = NSRect(x: unity.frame.minX - 190, y: 12, width: 180, height: 16)
        unityNote.alignment = .right
    }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 10, Theme.surface)
        let nf = Theme.font(13, .semibold)
        let nw = Theme.textWidth(loop.name, font: nf)
        Theme.draw(loop.name, in: NSRect(x: 12, y: 10, width: nw + 4, height: 18), font: nf, color: Theme.text, align: .left)
        Theme.draw(loop.sends.count > 1 ? "stereo" : "mono", in: NSRect(x: 20 + nw, y: 10, width: 60, height: 18), font: Theme.font(11), color: Theme.text3, align: .left)
        // path chips: DAW out -> send jack -> device -> return jack -> DAW in
        var x: CGFloat = 12
        for (i, t) in path.enumerated() {
            let f = Theme.font(11, i == 2 ? .semibold : .regular)
            let w = Theme.textWidth(t, font: f) + 16
            let fill = i == 2 ? Theme.hex(0xFF9F5A).blended(withFraction: 0.8, of: Theme.raised) ?? Theme.raised
                     : (i == 0 || i == 4 ? Theme.accent.blended(withFraction: 0.84, of: Theme.raised) ?? Theme.raised : Theme.raised)
            let r = NSRect(x: x, y: 36, width: w, height: 20)
            Theme.fill(r, radius: 6, fill)
            Theme.draw(t, in: r, font: f, color: i == 2 || i == 0 || i == 4 ? Theme.text : Theme.text2)
            x += w
            if i < path.count - 1 { Theme.draw("\u{2192}", in: NSRect(x: x, y: 36, width: 16, height: 20), font: Theme.font(11), color: Theme.text4); x += 16 }
        }
        Theme.hairline.setFill(); NSRect(x: bounds.width - 124, y: 64, width: 1, height: 40).fill()
    }
}

final class AdvancedHybridView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let ctx: PanelContext
    private let store: AdvancedStore
    private let loopsLabel = SectionLabel("Loops")
    private let add = FlatButton("\u{FF0B} New loop")
    private let empty = makeLabel("No loops yet. \u{FF0B} New loop: choose the output that feeds the device and the input it comes back on. The DAW plays into the send and records the return; the card shows both levels and their difference.", size: 12, color: Theme.text3)
    private var cards: [LoopCard] = []
    private let others = AdvMeterBridge(caption: "Other inputs \u{00B7} tracking at the same time", note: "loop returns are not monitored by the interface")
    private var otherCols: [AdvMeterColumn] = []
    private let monitorCard = Card(title: "Monitor"), infoCard = Card(title: "Sends and latency")
    private let knob = MonitorKnob()
    private let knobValue = makeLabel("", size: 19, weight: .semibold, color: Theme.text, align: .center)
    private let info = makeLabel("", size: 11, color: Theme.text3)
    private var layoutKey: ChannelLayout?
    private var channels = AdvancedChannels.none

    init(context: PanelContext, store: AdvancedStore) {
        ctx = context; self.store = store
        super.init(frame: .zero)
        empty.maximumNumberOfLines = 4; empty.lineBreakMode = .byWordWrapping; empty.cell?.wraps = true
        info.maximumNumberOfLines = 8; info.lineBreakMode = .byWordWrapping; info.cell?.wraps = true
        [loopsLabel, add, empty, others, monitorCard, infoCard].forEach { addSubview($0) }
        [knob, knobValue].forEach { monitorCard.addSubview($0) }
        infoCard.addSubview(info)
        add.onAction = { [weak self] in self?.loopSheet(editing: nil) }
        knob.onChange = { [weak self] v in self?.ctx.model.setLevel(MAKeys.outputLevels, channel: 1, value: v, linked: true); self?.knobValue.stringValue = Theme.levelText(v) + " dB" }
        knob.onCommit = { [weak self] in self?.ctx.model.commit() }
        rebuild()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func channel(_ id: String) -> AdvancedChannel? { return (channels.inputs + channels.outputs).first(where: { $0.id == id }) }

    private func rebuild() {
        let l = ctx.model.layout
        layoutKey = l
        channels = ctx.profile.advancedChannels(l)
        cards.forEach { $0.removeFromSuperview() }
        cards = store.loops.compactMap { lp in
            let s = lp.sends.compactMap(channel), r = lp.returns.compactMap(channel)
            guard s.count == lp.sends.count, r.count == lp.returns.count else { return nil }      // a channel this optical mode does not have
            func span(_ cs: [AdvancedChannel], _ f: (AdvancedChannel) -> String) -> String { return cs.count > 1 ? f(cs[0]) + "\u{2013}" + "\(cs[1].ca)" : f(cs[0]) }
            let path = [span(s) { "DAW out \($0.ca)" }, s.map { $0.jack }.joined(separator: " + "), lp.device.isEmpty ? "device" : lp.device,
                        r.map { $0.jack }.joined(separator: " + "), span(r) { "DAW in \($0.ca)" }]
            let c = LoopCard(lp, path: path, sendSlots: s.map { $0.slot }, returnSlots: r.map { $0.slot })
            c.remove.onAction = { [weak self] in self?.removeLoop(lp) }
            c.edit.onAction = { [weak self] in self?.loopSheet(editing: lp) }
            c.unity.onAction = { [weak self] in self?.setUnity(lp) }
            addSubview(c)
            return c
        }
        let used = Set(store.loops.flatMap { $0.returns })
        let key = store.viewKey, hidden = store.hiddenIDs(key)
        otherCols = channels.inputs.filter { !used.contains($0.id) && !hidden.contains($0.id) }.map { c in
            let v = AdvMeterColumn(slots: [c.slot], name: store.name(c), sub: c.jack.replacingOccurrences(of: "Mic/Line ", with: "In ").replacingOccurrences(of: "Line ", with: "In ").replacingOccurrences(of: "ADAT ", with: "A"))
            v.contextMenu = { [weak self] in
                guard let me = self else { return nil }
                return hideChannelMenu(name: me.store.name(c), hiddenCount: hidden.count,
                                       hide: { me.store.setHidden(c.id, true, in: key); me.rebuild(); me.refresh() },
                                       showAll: { me.store.showAll(in: key); me.rebuild(); me.refresh() })
            }
            return v
        }
        others.setGroups([AdvMeterBridge.Group(title: "", columns: otherCols)])
        empty.isHidden = !cards.isEmpty
        layoutAll()
    }

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); layoutAll() }
    private func layoutAll() {
        let p = Theme.pad, w = bounds.width, h = bounds.height, crW: CGFloat = 300
        let left = w - 3 * p - crW
        loopsLabel.frame = NSRect(x: p, y: p + 6, width: 200, height: 14)
        add.frame = NSRect(x: p + left - add.intrinsicContentSize.width, y: p, width: add.intrinsicContentSize.width, height: 26)
        let cardH: CGFloat = 114
        for (i, c) in cards.enumerated() { c.frame = NSRect(x: p, y: p + 34 + CGFloat(i) * (cardH + 8), width: left, height: cardH) }
        empty.frame = NSRect(x: p + 4, y: p + 40, width: left - 8, height: 60)
        let oy = p + 34 + max(CGFloat(cards.count) * (cardH + 8), 70)
        others.columnWidth = min(120, max(32, (left - 60 - CGFloat(otherCols.count) * 4) / CGFloat(max(1, otherCols.count))))
        others.frame = NSRect(x: p, y: oy, width: left, height: max(150, h - oy - p))
        let x = w - p - crW
        monitorCard.frame = NSRect(x: x, y: p, width: crW, height: 212)
        knob.frame = NSRect(x: (crW - 130) / 2, y: 34, width: 130, height: 130)
        knobValue.frame = NSRect(x: 10, y: 170, width: crW - 20, height: 24)
        infoCard.frame = NSRect(x: x, y: p + 222, width: crW, height: 170)
        info.frame = NSRect(x: 18, y: 40, width: crW - 36, height: 124)
    }

    func refresh() {
        let m = ctx.model
        if layoutKey != m.layout { rebuild() }
        knob.value = m.faderLevel(MAKeys.outputLevels, channel: 1)
        knobValue.stringValue = Theme.levelText(knob.value) + " dB"
        for c in cards {
            let off = HybridLoop.lineChannels(c.loop.sends).filter { m.element(MAKeys.outputLevels, $0) != MALevel.unity || m.element(MAKeys.outputMute, $0) != 0 }
            c.setUnity(off.isEmpty ? nil : off.map { "Line \($0) at \(m.element(MAKeys.outputMute, $0) != 0 ? "mute" : Theme.levelText(m.element(MAKeys.outputLevels, $0)) + " dB")" }.joined(separator: ", "))
        }
        let free = HybridLoop.freeSends(channels.outputs, loops: store.loops).map { $0.name }
        info.stringValue = "Free sends: " + (free.isEmpty ? "none" : free.joined(separator: ", ")) + ". Line 1/2 are the monitor speakers.\n\nThe DAW measures each loop\u{2019}s latency (ping / insert delay compensation). Driver round trip on the bench: \u{2248} 9 ms at 48 kHz / 64 frames."
    }

    func updateMeters() {
        let db = { [ctx] (s: Int) -> Int in ctx.db(s) }
        cards.forEach { $0.updateMeters(db) }
        otherCols.forEach { $0.update(db) }
        others.flush()
    }

    // MARK: actions

    private func setUnity(_ lp: HybridLoop) {
        let m = ctx.model
        for ch in HybridLoop.lineChannels(lp.sends) { m.setElement(MAKeys.outputLevels, ch, MALevel.unity); m.setElement(MAKeys.outputMute, ch, 0) }
        m.commit()
    }
    private func removeLoop(_ lp: HybridLoop) {
        store.loops = store.loops.filter { $0 != lp }
        rebuild(); refresh()
    }

    /// New loop, or edit one (`editing`): name, device, send output, return input, mono or stereo (stereo takes the next channel of each
    /// list). The lists offer what the other loops leave free; an edited loop keeps its place in the list.
    private func loopSheet(editing old: HybridLoop?) {
        guard let w = window else { return }
        let others = store.loops.filter { $0 != old }
        let sends = HybridLoop.freeSends(channels.outputs, loops: others)
        let usedReturns = Set(others.flatMap { $0.returns })
        let returns = channels.inputs.filter { !usedReturns.contains($0.id) }
        guard !sends.isEmpty, !returns.isEmpty else { NSSound.beep(); return }
        let a = NSAlert()
        a.messageText = old == nil ? "New loop" : "Edit loop"
        a.informativeText = "The DAW plays into the send output; the device comes back on the return input. Stereo uses the next channel of each."
        let box = NSView(frame: NSRect(x: 0, y: 0, width: 320, height: 150))
        func row(_ y: CGFloat, _ t: String, _ v: NSView) {
            let l = NSTextField(labelWithString: t); l.frame = NSRect(x: 0, y: y + 3, width: 70, height: 18); box.addSubview(l)
            v.frame = NSRect(x: 76, y: y, width: 240, height: 24); box.addSubview(v)
        }
        let name = NSTextField(string: old?.name ?? "Loop \(store.loops.count + 1)"), dev = NSTextField(string: old?.device ?? "")
        dev.placeholderString = "e.g. 1176, reamp box \u{2192} amp"
        let sp = NSPopUpButton(frame: .zero, pullsDown: false); sp.addItems(withTitles: sends.map { "\($0.jack)  (DAW out \($0.ca))" })
        let rp = NSPopUpButton(frame: .zero, pullsDown: false); rp.addItems(withTitles: returns.map { "\($0.jack)  (DAW in \($0.ca))" })
        let st = NSPopUpButton(frame: .zero, pullsDown: false); st.addItems(withTitles: ["Mono", "Stereo"])
        row(124, "Name", name); row(94, "Device", dev); row(62, "Send", sp); row(30, "Return", rp); row(0, "Format", st)
        if let o = old {
            if let i = sends.index(where: { $0.id == o.sends[0] }) { sp.selectItem(at: i) }
            if let i = returns.index(where: { $0.id == o.returns[0] }) { rp.selectItem(at: i) }
            st.selectItem(at: o.sends.count > 1 ? 1 : 0)
        }
        a.accessoryView = box
        a.addButton(withTitle: old == nil ? "Add" : "Save"); a.addButton(withTitle: "Cancel")
        a.beginSheetModal(for: w) { [weak self] r in
            guard r == .alertFirstButtonReturn, let me = self else { return }
            let si = max(0, sp.indexOfSelectedItem), ri = max(0, rp.indexOfSelectedItem)
            var s = [sends[si].id], rt = [returns[ri].id]
            if st.indexOfSelectedItem == 1 {
                guard si + 1 < sends.count, ri + 1 < returns.count else { NSSound.beep(); return }
                s.append(sends[si + 1].id); rt.append(returns[ri + 1].id)
            }
            let n = name.stringValue.trimmingCharacters(in: .whitespacesAndNewlines)
            let lp = HybridLoop(name: n.isEmpty ? "Loop" : String(n.prefix(24)), device: String(dev.stringValue.prefix(32)), sends: s, returns: rt)
            if let o = old, let i = me.store.loops.index(of: o) { var ls = me.store.loops; ls[i] = lp; me.store.loops = ls }
            else { me.store.loops = me.store.loops + [lp] }
            me.rebuild(); me.refresh()
        }
    }
}
