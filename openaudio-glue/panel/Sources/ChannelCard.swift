// OpenAudio Glue app - one channel's card in the matrix: number, label (the device's channel name; double-click to rename) and its
// routing, the ON pill, the compressor (Source menu, threshold slider, gain-reduction bar with the Drums hit lift) and the EQ graph.
// Static parts are drawn once into the card's layer; the meter bars move as layers (GPU-first, the main panel's rule).
import AppKit

/// A horizontal value slider in 0.5 dB steps. Threshold: -40 ... 0 dB, filled from the knob to the right, 0 = off (the resting value).
/// Drive: 0 ... +18 dB, filled from the left, 0 = the resting value. Drag across; double-click = the resting value.
final class ThresholdSlider: FlippedView {
    var value = 0.0 { didSet { if value != oldValue { needsDisplay = true } } }
    var isEnabled = true { didSet { needsDisplay = true } }
    var onChange: ((Double) -> Void)?
    var range: ClosedRange<Double> = GlueOptions.thresholdRange
    var rest = 0.0                                  // the double-click value, drawn as "off"
    var fillFromLeft = false
    private var t: CGFloat { return CGFloat((value - range.lowerBound) / (range.upperBound - range.lowerBound)) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let track = NSRect(x: 0, y: bounds.midY - 2, width: bounds.width, height: 4)
            Theme.fill(track, radius: 2, Theme.track)
            let off = value == rest
            if !off {
                let fill = fillFromLeft ? NSRect(x: 0, y: track.minY, width: bounds.width * t, height: 4)
                                        : NSRect(x: bounds.width * t, y: track.minY, width: bounds.width * (1 - t), height: 4)
                Theme.fill(fill, radius: 2, Theme.accent.withAlphaComponent(0.55))
            }
            let knob = NSRect(x: max(0, min(bounds.width - 10, bounds.width * t - 5)), y: bounds.midY - 6, width: 10, height: 12)
            Theme.fill(knob, radius: 3, off ? Theme.text3 : Theme.capFace)
        }
    }

    private func set(_ p: NSPoint) {
        let r = range
        let v = r.lowerBound + Double(min(max(p.x / bounds.width, 0), 1)) * (r.upperBound - r.lowerBound)
        let stepped = (v * 2).rounded() / 2
        if stepped != value { value = stepped; onChange?(stepped) }
    }
    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        if event.clickCount == 2 { value = rest; onChange?(rest); return }
        set(convert(event.locationInWindow, from: nil))
    }
    override func mouseDragged(with event: NSEvent) { guard isEnabled else { return }; set(convert(event.locationInWindow, from: nil)) }
}

/// The card's name: lets clicks through to the card (double-click to rename) except while it is being edited.
final class LabelField: NSTextField {
    var passesClicks = true
    override func hitTest(_ point: NSPoint) -> NSView? { return passesClicks ? nil : super.hitTest(point) }
}

final class ChannelCard: FlippedView {
    static let width: CGFloat = 196, height: CGFloat = 262
    static let maxGrDb: Float = 20

    let index: Int
    var label: String { didSet { labelField.stringValue = label; needsDisplay = true } }
    var routing: String = "" { didSet { if routing != oldValue { needsDisplay = true } } }
    var channel = GlueChannel() { didSet { apply() } }
    var rate = 48000.0 { didSet { eq.rate = rate } }
    var onChange: ((GlueChannel) -> Void)?
    var onRename: ((String) -> Void)?
    var onResetName: (() -> Void)?
    /// Set on the second channel of a linked pair: it shows its leader's settings and its controls are locked.
    var follows: Int? { didSet { if follows != oldValue { apply(); needsDisplay = true } } }
    var isLeaderSlot: Bool { return index % 2 == 0 }

    private let on = PillButton("ON")
    private let link = PillButton("LINK")
    private let source = FlatButton("General")
    private let threshold = ThresholdSlider()
    private let eq = EQGraphView(frame: .zero)
    private let labelField = LabelField(string: "")
    private var renaming = false
    private let thrValue = makeLabel("Off", size: 10, color: Theme.text2, align: .right)
    private let grValue = makeLabel("", size: 10, color: Theme.text3, align: .right)
    private let grWell = CALayer(), grBar = CALayer(), hitBar = CALayer()

    init(index: Int, label: String) {
        self.index = index
        self.label = label
        super.init(frame: NSRect(x: 0, y: 0, width: ChannelCard.width, height: ChannelCard.height))
        wantsLayer = true
        canDrawSubviewsIntoLayer = false
        on.onColor = Theme.green
        on.solid = true
        on.onClick = { [weak self] b in guard let s = self else { return }; s.channel.on = b.isOn; s.onChange?(s.channel) }
        link.onColor = Theme.accent
        link.solid = true
        link.fontSize = 9
        link.toolTip = "Stereo link: channel \(index + 2) follows this one (EQ, compressor, on/off; one gain reduction for both)"
        link.onClick = { [weak self] b in guard let s = self else { return }; s.channel.link = b.isOn; s.onChange?(s.channel) }
        source.onAction = { [weak self] in self?.showSourceMenu() }
        threshold.onChange = { [weak self] v in guard let s = self else { return }; s.channel.threshold = v; s.onChange?(s.channel) }
        eq.onChange = { [weak self] b in guard let s = self else { return }; s.channel.bands = b; s.onChange?(s.channel) }
        labelField.isBordered = false; labelField.drawsBackground = false; labelField.isEditable = false; labelField.isSelectable = false
        labelField.font = Theme.font(12, .semibold); labelField.textColor = Theme.text; labelField.stringValue = label
        labelField.lineBreakMode = .byTruncatingTail
        labelField.focusRingType = .none
        labelField.target = self; labelField.action = #selector(renamed)
        for v in [on, link, source, threshold, eq, labelField, thrValue, grValue] as [NSView] { addSubview(v) }
        link.isHidden = !isLeaderSlot
        for l in [grWell, grBar, hitBar] { l.actions = ["bounds": NSNull(), "position": NSNull(), "frame": NSNull()]; layer?.addSublayer(l) }
        grWell.backgroundColor = Theme.well.cgColor; grWell.cornerRadius = 2
        grBar.backgroundColor = Theme.solo.cgColor; grBar.cornerRadius = 2
        hitBar.backgroundColor = Theme.green.cgColor; hitBar.cornerRadius = 2
        layoutParts()
        apply()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func layoutParts() {
        let w = bounds.width, m: CGFloat = 12
        labelField.frame = NSRect(x: m + 22, y: 9, width: w - m * 2 - 22 - 44, height: 17)
        on.frame = NSRect(x: w - m - 38, y: 9, width: 38, height: 20)
        link.frame = NSRect(x: w - m - 38, y: 30, width: 38, height: 15)        // on the routing line, under ON
        source.frame = NSRect(x: m, y: 52, width: 96, height: 24)
        grValue.frame = NSRect(x: m + 98, y: 57, width: w - m * 2 - 98, height: 14)
        thrValue.frame = NSRect(x: w - m - 70, y: 84, width: 70, height: 14)
        threshold.frame = NSRect(x: m, y: 100, width: w - m * 2, height: 16)
        grWell.frame = CGRect(x: m, y: 122, width: w - m * 2, height: 5)
        eq.frame = NSRect(x: m, y: 138, width: w - m * 2, height: ChannelCard.height - 138 - m)
    }

    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: Theme.radius, Theme.surface)
        let m: CGFloat = 12
        Theme.fill(NSRect(x: m, y: 9, width: 18, height: 18), radius: 4, Theme.raised)
        Theme.draw("\(index + 1)", in: NSRect(x: m, y: 9, width: 18, height: 18), font: Theme.mono(10, .semibold), color: Theme.text2)
        let route = follows.map { routing + "  \u{00B7}  LINKED TO \($0 + 1)" } ?? routing
        Theme.caption(route, in: NSRect(x: m, y: 31, width: bounds.width - m * 2 - (isLeaderSlot ? 44 : 0), height: 12), color: follows == nil ? Theme.text3 : Theme.accent,
                      align: .left, size: 9)
        if follows != nil { Theme.fill(NSRect(x: 0, y: 14, width: 3, height: 10), radius: 1.5, Theme.accent) }   // the link, at the left edge
        Theme.caption("THRESHOLD", in: NSRect(x: m, y: 86, width: 80, height: 12), color: Theme.text3, align: .left, size: 9)
    }

    private func apply() {
        let c = channel
        on.isOn = c.on
        on.title = c.on ? "ON" : "OFF"
        source.title = GlueOptions.sourceNames[min(max(c.source, 0), GlueOptions.sourceNames.count - 1)]
        threshold.value = c.threshold
        thrValue.stringValue = c.threshold >= 0 ? "Off" : String(format: "%.1f dB", c.threshold)
        eq.bands = c.bands
        link.isOn = c.link
        let locked = follows != nil
        on.isEnabled = !locked
        let enabled = c.on && !locked
        source.isEnabled = enabled; threshold.isEnabled = enabled; eq.isEnabled = enabled
        thrValue.textColor = enabled ? Theme.text2 : Theme.text4
        labelField.textColor = c.on ? Theme.text : Theme.text3
        if !enabled || c.threshold >= 0 { setMeters(gr: 0, hit: 0) }
    }

    /// Gain reduction grows from the right (<= 0 dB); the Drums hit lift shows as a green bar from the left.
    func setMeters(gr: Float, hit: Float) {
        let well = grWell.frame
        let f = CGFloat(min(max(-gr, 0), ChannelCard.maxGrDb) / ChannelCard.maxGrDb)
        grBar.frame = CGRect(x: well.maxX - well.width * f, y: well.minY, width: well.width * f, height: well.height)
        let h = CGFloat(min(max(hit, 0), 6) / 6) * well.width * 0.3
        hitBar.frame = CGRect(x: well.minX, y: well.minY, width: h, height: well.height)
        let text = (channel.on && channel.threshold < 0) ? String(format: "GR %.1f", gr) + (channel.source == 1 ? String(format: "  \u{25B2}%.1f", hit) : "") : ""
        if grValue.stringValue != text { grValue.stringValue = text }
    }

    private func showSourceMenu() {
        let menu = NSMenu()
        for (i, name) in GlueOptions.sourceNames.enumerated() {
            let item = NSMenuItem(title: name, action: #selector(pickSource(_:)), keyEquivalent: "")
            item.target = self; item.tag = i; item.state = i == channel.source ? .on : .off
            menu.addItem(item)
        }
        menu.popUp(positioning: nil, at: NSPoint(x: source.frame.minX, y: source.frame.maxY + 2), in: self)
    }
    @objc private func pickSource(_ item: NSMenuItem) { channel.source = item.tag; onChange?(channel) }

    override func mouseDown(with event: NSEvent) {
        let p = convert(event.locationInWindow, from: nil)
        if event.clickCount == 2 && labelField.frame.insetBy(dx: -4, dy: -4).contains(p) { beginRename() }   // rename in place
    }

    /// Rename in place. selectText alone makes the field first responder and selects the name; calling makeFirstResponder first
    /// would start an edit that selectText then ends (NSWindow endEditingFor), firing renamed() at once (found 2026-10-28).
    @objc func beginRename() {
        renaming = true
        labelField.passesClicks = false
        labelField.isEditable = true; labelField.isSelectable = true; labelField.drawsBackground = true
        labelField.backgroundColor = Theme.raised
        labelField.selectText(nil)
    }

    // Right-click: rename, or go back to the device's name.
    override func menu(for event: NSEvent) -> NSMenu? {
        let m = NSMenu()
        let r = NSMenuItem(title: "Rename\u{2026}", action: #selector(beginRename), keyEquivalent: ""); r.target = self; m.addItem(r)
        let d = NSMenuItem(title: "Use the Device's Name", action: #selector(resetName), keyEquivalent: ""); d.target = self; m.addItem(d)
        return m
    }
    @objc private func resetName() { onResetName?() }
    @objc private func renamed() {
        guard renaming else { return }                       // only the end of a rename this card started
        renaming = false
        labelField.passesClicks = true
        labelField.isEditable = false; labelField.isSelectable = false; labelField.drawsBackground = false
        let t = labelField.stringValue.trimmingCharacters(in: .whitespaces)
        onRename?(t)
    }
}
