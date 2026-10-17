// PN.3 -- the right-hand sidebar present on every tab (wireframe PN1, screenshots): settings buttons, level controller (encoder assignment
// radios), main output mute/dim. The radios write MARotaryControlV2 presets (A2-P13 §2); mute/dim write MAGlobalMute/MAGlobalDim (rows 28/27).
// The on-screen knob is bounded (it follows the LED ring inside the working range; the front-panel knob is an endless encoder): each batch of
// detents from a drag or the scroll wheel is written at once as a signed 32-bit count to MAMetaVirtualRotateDataKnob (positive = clockwise = louder) and committed; the kext steps every pair
// selected in MARotaryControlV2 and publishes the new levels, which come back through the model refresh. Its LED ring shows the loudest
// level among the selected channels.
import AppKit

final class SidebarView: FlippedView, PanelTab {
    private let ctx: PanelContext
    private let presets: PresetStore
    private weak var windowRef: NSWindow?
    private let loadB = FlatButton("Load"), saveB = FlatButton("Save"), resetB = FlatButton("Reset"), deleteB = FlatButton("Delete")
    private var radios: [(ChoiceButton, MARotaryMask.Group)] = []
    private let knob = EncoderKnob()
    private let muteB = PillButton("mute"), dimB = PillButton("dim")
    private var readout = "-"

    // Layout (top-down): settings, level control, main output.
    private let inset: CGFloat = 20
    private let knobY: CGFloat = 154, knobSize: CGFloat = 132
    private var readoutRect: NSRect { return NSRect(x: 0, y: knobY + knobSize + 2, width: bounds.width, height: 16) }

    init(context: PanelContext, presets: PresetStore, window: NSWindow?) {
        ctx = context; self.presets = presets; windowRef = window
        super.init(frame: NSRect(x: 0, y: 0, width: Theme.sidebarWidth, height: 560))
        build()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    override var isOpaque: Bool { return true }

    private func build() {
        let m = ctx.model
        let w = Theme.sidebarWidth, bw = (w - 2 * inset - 8) / 2
        loadB.onAction = { [weak self] in self?.presets.load(from: self?.windowRef) }
        saveB.onAction = { [weak self] in self?.presets.save(from: self?.windowRef) }
        resetB.onAction = { m.resetToFactory() }
        deleteB.onAction = { [weak self] in self?.presets.deleteLoaded(from: self?.windowRef) }
        presets.onChange = { [weak self] in self?.refresh() }
        for (i, b) in [loadB, saveB, resetB, deleteB].enumerated() {
            b.frame = NSRect(x: inset + CGFloat(i % 2) * (bw + 8), y: 44 + CGFloat(i / 2) * 34, width: bw, height: 26)
            addSubview(b)
        }

        knob.frame = NSRect(x: ((w - knobSize) / 2).rounded(), y: knobY, width: knobSize, height: knobSize)
        knob.onTurn = { [weak self] n in self?.sendDetents(n) }
        addSubview(knob)
        let defs: [(String, MARotaryMask.Group)] = [("SW return bus", .swReturn), ("Output bus", .output), ("Input bus", .input), ("Aux send", .aux)]
        for (i, (t, g)) in defs.enumerated() {
            let b = ChoiceButton(radio: t)
            b.onAction = { m.setRotaryPreset(MARotaryMask.preset(for: g)) }
            b.frame = NSRect(x: inset + 4, y: knobY + knobSize + 30 + CGFloat(i) * 28, width: w - 2 * inset - 4, height: 20)
            radios.append((b, g)); addSubview(b)
        }

        let my = knobY + knobSize + 30 + 4 * 28 + 52
        muteB.onColor = Theme.mute; muteB.solid = true
        dimB.onColor = Theme.solo; dimB.solid = true
        for (i, b) in [muteB, dimB].enumerated() {
            b.fontSize = 11
            b.frame = NSRect(x: inset + CGFloat(i) * (bw + 8), y: my, width: bw, height: 28)
            addSubview(b)
        }
        muteB.onClick = { b in m.setNumber(MAKeys.globalMute, b.isOn ? 1 : 0); m.commit() }
        dimB.onClick = { b in m.setNumber(MAKeys.globalDim, b.isOn ? 1 : 0); m.commit() }
    }

    /// One batch of encoder detents, sent at once (signed 32-bit count in the u32 property).
    private func sendDetents(_ n: Int) {
        let m = ctx.model
        m.setNumber(MAKeys.virtualRotateKnob, UInt64(UInt32(bitPattern: Int32(clamping: n))))
        m.commit()
    }

    /// Debug/demo aid: turns the on-screen encoder as a user would (same path as a drag).
    func turnLevelKnob(_ n: Int) { knob.turn(n) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.sidebar.setFill(); dirtyRect.fill()
        let w = bounds.width
        Theme.caption("Settings", in: NSRect(x: inset, y: 20, width: w - 2 * inset, height: 14), align: .left)
        Theme.caption("Level Control", in: NSRect(x: inset, y: 130, width: w - 2 * inset, height: 14), align: .left)
        Theme.draw(readout, in: readoutRect, font: Theme.mono(12, .medium), color: Theme.text2)
        Theme.caption("Main Output", in: NSRect(x: inset, y: knobY + knobSize + 30 + 4 * 28 + 28, width: w - 2 * inset, height: 14), align: .left)
    }

    func rebuild() { refresh() }
    func updateMeters() {}
    var preferredSize: NSSize { return NSSize(width: Theme.sidebarWidth, height: 532) }

    func refresh() {
        let g = ctx.rotaryGroup
        for (b, grp) in radios { b.on = (g == grp) }
        muteB.isOn = (ctx.model.number(MAKeys.globalMute) ?? 0) != 0
        dimB.isOn = (ctx.model.number(MAKeys.globalDim) ?? 0) != 0
        deleteB.isEnabled = presets.canDelete
        // Ring + readout: the loudest selected channel (levels -129...0, 0.5 dB steps).
        var loudest: Int?
        for t in RotaryTargets.channels(mask: ctx.model.rotaryMask) {
            let a = ctx.model.ints(t.key)
            for c in t.channels where c >= 1 && c <= a.count { loudest = max(loudest ?? Int.min, a[c - 1]) }
        }
        knob.value = loudest                                      // the pointer and the ring follow the model, so the knob always shows the current level
        let text = loudest.map { $0 <= MARange.levelMin ? "-\u{221E} dB" : String(format: "%.1f dB", Double($0) / 2) } ?? "-"
        if text != readout { readout = text; setNeedsDisplay(readoutRect) }
    }
}
