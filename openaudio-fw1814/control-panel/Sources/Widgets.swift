// PN.3 -- the panel's controls, all drawn in code with the Theme tokens: fader with the reference's non-linear dB scale, value knob with an
// LED-style ring, the endless level-controller encoder with its LED ring, slim LED meters, pill toggles, flat push buttons, radio buttons,
// check boxes, a stepper, segmented bars with text or icons, cards and captions. The fader scale positions are an approximation read off
// the v1.9 screenshots (0, 2, 5, 10, 20, 30, 50 dB), not a decoded table: **guess**, check against the reference CMAFader if pixel parity
// is wanted. Values are the property domain of A2-P13 §3: levels -129...0 in 0.5 dB steps (dB = -value/2), pan -255...255.
import AppKit

/// Static text label in the panel's type colours.
func makeLabel(_ text: String, size: CGFloat = 11, weight: NSFont.Weight = .regular, color: NSColor = Theme.text2,
               align: NSTextAlignment = .left) -> NSTextField {
    let t = NSTextField(labelWithString: text)
    t.font = Theme.font(size, weight)
    t.textColor = color
    t.alignment = align
    t.lineBreakMode = .byClipping
    t.drawsBackground = false
    return t
}

// MARK: - toggles and buttons

/// Small toggle (ctrl, solo, link, mute, routing). On = a solid semantic fill, or a tinted fill for the calm states.
final class PillButton: NSView {
    var title: String { didSet { needsDisplay = true } }
    var isOn = false { didSet { if isOn != oldValue { needsDisplay = true } } }
    var isEnabled = true { didSet { if isEnabled != oldValue { needsDisplay = true } } }
    var momentary = false
    var onClick: ((PillButton) -> Void)?
    var onColor = Theme.accent { didSet { needsDisplay = true } }
    /// Solid fill when on (mute, solo, dim); otherwise a tinted fill with coloured text.
    var solid = false { didSet { needsDisplay = true } }
    /// When set, the off state is drawn tinted in this colour too (two-state selectors such as main/aux).
    var offColor: NSColor? { didSet { needsDisplay = true } }
    var fontSize: CGFloat = 10
    private var pressed = false

    init(_ title: String) {
        self.title = title
        super.init(frame: NSRect(x: 0, y: 0, width: 34, height: 20))
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    override var intrinsicContentSize: NSSize { return NSSize(width: max(34, Theme.textWidth(title, font: Theme.font(fontSize, .medium)) + 16), height: 20) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let r = bounds
            let fill: NSColor, fg: NSColor
            if isOn {
                fill = solid ? onColor : onColor.withAlphaComponent(pressed ? 0.32 : 0.20)
                fg = solid ? Theme.textOn(onColor) : onColor
            } else if let off = offColor {
                fill = off.withAlphaComponent(pressed ? 0.32 : 0.20); fg = off
            } else {
                fill = pressed ? Theme.controlHi : Theme.control; fg = Theme.text2
            }
            Theme.fill(r, radius: 5, fill)
            Theme.draw(title, in: r, font: Theme.font(fontSize, .medium), color: fg)
        }
    }

    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        pressed = true; needsDisplay = true
        if !momentary { isOn = !isOn }
        onClick?(self)
    }
    override func mouseUp(with event: NSEvent) { pressed = false; needsDisplay = true }
}

/// Flat push button (settings, web links, sheet buttons). Fires on mouse-up inside.
final class FlatButton: NSView {
    var title: String { didSet { needsDisplay = true } }
    var isEnabled = true { didSet { if isEnabled != oldValue { needsDisplay = true } } }
    var primary = false { didSet { needsDisplay = true } }
    var onAction: (() -> Void)?
    /// Plain key that fires the button ("\r" default button, escape for cancel).
    var keyEquivalent: String?
    private var pressed = false

    override func performKeyEquivalent(with event: NSEvent) -> Bool {
        if isEnabled, let k = keyEquivalent, event.charactersIgnoringModifiers == k { onAction?(); return true }
        return super.performKeyEquivalent(with: event)
    }

    init(_ title: String) {
        self.title = title
        super.init(frame: NSRect(x: 0, y: 0, width: 80, height: 26))
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    override var intrinsicContentSize: NSSize { return NSSize(width: Theme.textWidth(title, font: Theme.font(11, .medium)) + 28, height: 26) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let fill = primary ? (pressed ? Theme.accent.withAlphaComponent(0.8) : Theme.accent) : (pressed ? Theme.controlHi : Theme.control)
            Theme.fill(bounds, radius: 6, fill)
            Theme.draw(title, in: bounds, font: Theme.font(11, .medium), color: primary ? Theme.textOn(Theme.accent) : Theme.text)
        }
    }

    override func mouseDown(with event: NSEvent) { guard isEnabled else { return }; pressed = true; needsDisplay = true }
    override func mouseDragged(with event: NSEvent) {
        guard isEnabled else { return }
        let inside = bounds.contains(convert(event.locationInWindow, from: nil))
        if inside != pressed { pressed = inside; needsDisplay = true }
    }
    override func mouseUp(with event: NSEvent) {
        guard isEnabled else { return }
        let inside = bounds.contains(convert(event.locationInWindow, from: nil))
        pressed = false; needsDisplay = true
        if inside { onAction?() }
    }
}

/// Radio button or check box with its label.
final class ChoiceButton: NSView {
    enum Style { case radio, check }
    let style: Style
    var title: String { didSet { needsDisplay = true } }
    var on = false { didSet { if on != oldValue { needsDisplay = true } } }
    var isEnabled = true { didSet { if isEnabled != oldValue { needsDisplay = true } } }
    var onAction: (() -> Void)?
    static let font = Theme.font(12)

    init(radio title: String) { style = .radio; self.title = title; super.init(frame: .zero); setFrameSize(intrinsicContentSize) }
    init(check title: String) { style = .check; self.title = title; super.init(frame: .zero); setFrameSize(intrinsicContentSize) }
    required init?(coder: NSCoder) { fatalError("unused") }

    override var intrinsicContentSize: NSSize { return NSSize(width: 24 + Theme.textWidth(title, font: ChoiceButton.font), height: 20) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let box = NSRect(x: 1, y: bounds.midY - 7, width: 14, height: 14)
            switch style {
            case .radio:
                if on {
                    Theme.accent.setFill(); NSBezierPath(ovalIn: box).fill()
                    Theme.chrome.setFill(); NSBezierPath(ovalIn: box.insetBy(dx: 4.5, dy: 4.5)).fill()
                } else {
                    Theme.control.setFill(); NSBezierPath(ovalIn: box).fill()
                    let ring = NSBezierPath(ovalIn: box.insetBy(dx: 0.75, dy: 0.75)); ring.lineWidth = 1.5
                    Theme.text4.setStroke(); ring.stroke()
                }
            case .check:
                let p = NSBezierPath(roundedRect: box, xRadius: 3.5, yRadius: 3.5)
                if on {
                    Theme.accent.setFill(); p.fill()
                    let m = NSBezierPath()                    // tick, in unflipped coordinates (y up)
                    m.move(to: NSPoint(x: box.minX + 3.5, y: box.maxY - 7.2))
                    m.line(to: NSPoint(x: box.minX + 6, y: box.maxY - 9.8))
                    m.line(to: NSPoint(x: box.minX + 10.5, y: box.maxY - 4.4))
                    m.lineWidth = 1.8; m.lineCapStyle = .roundLineCapStyle; m.lineJoinStyle = .roundLineJoinStyle
                    Theme.chrome.setStroke(); m.stroke()
                } else {
                    Theme.control.setFill(); p.fill()
                    let ring = NSBezierPath(roundedRect: box.insetBy(dx: 0.75, dy: 0.75), xRadius: 3, yRadius: 3); ring.lineWidth = 1.5
                    Theme.text4.setStroke(); ring.stroke()
                }
            }
            Theme.draw(title, in: NSRect(x: 24, y: 0, width: bounds.width - 24, height: bounds.height), font: ChoiceButton.font,
                       color: on ? Theme.text : Theme.text2, align: .left)
        }
    }

    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        if style == .radio { on = true } else { on = !on }
        onAction?()
    }
}

/// Up/down stepper (sample rate).
final class StepperControl: FlippedView {
    var isEnabled = true { didSet { needsDisplay = true } }
    var canUp = true { didSet { needsDisplay = true } }
    var canDown = true { didSet { needsDisplay = true } }
    var onStep: ((Int) -> Void)?
    private var pressedHalf = 0

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let half = bounds.height / 2
            for (i, dir) in [(0, 1), (1, -1)] {
                let r = NSRect(x: 0, y: CGFloat(i) * half, width: bounds.width, height: half).insetBy(dx: 0, dy: 1)
                Theme.fill(r, radius: 6, pressedHalf == dir ? Theme.controlHi : Theme.control)
                let ok = dir > 0 ? canUp : canDown
                let c = NSPoint(x: r.midX, y: r.midY), s: CGFloat = 4
                let p = NSBezierPath()
                let dy: CGFloat = dir > 0 ? -1 : 1               // flipped: up chevron points to smaller y
                p.move(to: NSPoint(x: c.x - s, y: c.y - dy * s / 2)); p.line(to: NSPoint(x: c.x, y: c.y + dy * s / 2)); p.line(to: NSPoint(x: c.x + s, y: c.y - dy * s / 2))
                p.lineWidth = 1.6; p.lineCapStyle = .roundLineCapStyle; p.lineJoinStyle = .roundLineJoinStyle
                (ok ? Theme.text : Theme.text4).setStroke(); p.stroke()
            }
        }
    }

    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        let p = convert(event.locationInWindow, from: nil)
        let dir = p.y < bounds.midY ? 1 : -1
        pressedHalf = dir; needsDisplay = true
        if (dir > 0 && canUp) || (dir < 0 && canDown) { onStep?(dir) }
    }
    override func mouseUp(with event: NSEvent) { pressedHalf = 0; needsDisplay = true }
}

// MARK: - segmented bars (tabs, view switch)

/// Glyphs for the view-mode switch: the full window, the wide (vertical bars) and the narrow (horizontal bars) meter windows.
enum ModeIcon { case full, wide, narrow }

final class SegmentedBar: FlippedView {
    enum Item { case text(String), icon(ModeIcon) }
    private final class Segment: FlippedView {
        let item: Item
        var selected = false { didSet { needsDisplay = true } }
        var onClick: (() -> Void)?
        init(_ item: Item) { self.item = item; super.init(frame: .zero) }
        required init?(coder: NSCoder) { fatalError("unused") }
        override func draw(_ dirtyRect: NSRect) {
            if selected { Theme.fill(bounds.insetBy(dx: 2, dy: 2), radius: 6, Theme.controlHi) }
            let fg = selected ? Theme.text : Theme.text3
            switch item {
            case .text(let s): Theme.draw(s, in: bounds, font: Theme.font(12, selected ? .semibold : .medium), color: fg)
            case .icon(let i): SegmentedBar.drawIcon(i, center: NSPoint(x: bounds.midX, y: bounds.midY), color: fg)
            }
        }
        override func mouseDown(with event: NSEvent) { onClick?() }
    }

    private var segments: [Segment] = []
    var selected = 0 { didSet { for (i, s) in segments.enumerated() { s.selected = (i == selected) } } }
    var onSelect: ((Int) -> Void)?
    let segmentWidth: CGFloat

    init(items: [Item], segmentWidth: CGFloat, tooltips: [String] = []) {
        self.segmentWidth = segmentWidth
        super.init(frame: NSRect(x: 0, y: 0, width: segmentWidth * CGFloat(items.count) + 4, height: 30))
        for (i, it) in items.enumerated() {
            let s = Segment(it)
            s.frame = NSRect(x: 2 + CGFloat(i) * segmentWidth, y: 0, width: segmentWidth, height: 30)
            if i < tooltips.count { s.toolTip = tooltips[i] }
            s.onClick = { [weak self] in
                guard let me = self else { return }
                me.selected = i
                me.onSelect?(i)
            }
            segments.append(s); addSubview(s)
        }
        segments.first?.selected = true                     // property observers do not run in init, so mark segment 0 here
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    override func draw(_ dirtyRect: NSRect) { Theme.fill(bounds, radius: 8, Theme.raised) }

    static func drawIcon(_ i: ModeIcon, center c: NSPoint, color: NSColor) {
        color.setFill(); color.setStroke()
        switch i {
        case .full:                                     // window with a header band and a sidebar
            let r = NSRect(x: c.x - 8, y: c.y - 6, width: 16, height: 12)
            let p = NSBezierPath(roundedRect: r.insetBy(dx: 0.6, dy: 0.6), xRadius: 2, yRadius: 2); p.lineWidth = 1.2; p.stroke()
            NSRect(x: r.minX + 1, y: r.minY + 1, width: r.width - 2, height: 2.4).fill()
            NSRect(x: r.maxX - 5, y: r.minY + 3, width: 1.2, height: r.height - 4).fill()
        case .wide:                                     // vertical bars
            let hs: [CGFloat] = [8, 12, 6, 10, 7]
            for (k, h) in hs.enumerated() {
                NSBezierPath(roundedRect: NSRect(x: c.x - 8 + CGFloat(k) * 3.5, y: c.y + 6 - h, width: 2, height: h), xRadius: 1, yRadius: 1).fill()
            }
        case .narrow:                                   // horizontal bars
            let ws: [CGFloat] = [12, 16, 8, 13]
            for (k, w) in ws.enumerated() {
                NSBezierPath(roundedRect: NSRect(x: c.x - 8, y: c.y - 6 + CGFloat(k) * 3.4, width: w, height: 2), xRadius: 1, yRadius: 1).fill()
            }
        }
    }
}

// MARK: - fader

/// One channel fader: thin track, value fill from the bottom, flat light cap with a grip line. Clicking the cap drags it relatively,
/// clicking the track jumps there.
final class Fader: NSView {
    var value = 0 { didSet { if value != oldValue { needsDisplay = true } } }
    var isEnabled = true { didSet { if isEnabled != oldValue { needsDisplay = true } } }
    var onChange: ((Int) -> Void)?
    var onCommit: (() -> Void)?

    // (dB, fraction of the track from the top); positions approximated from the screenshots, 64.5 dB stands for the bottom (-129 steps).
    static let curve: [(db: Double, frac: CGFloat)] = [(0, 0), (2, 0.26), (5, 0.42), (10, 0.55), (20, 0.69), (30, 0.76), (50, 0.90), (64.5, 1)]
    static let labels = [0, 2, 5, 10, 20, 30, 50]
    static let trackPad: CGFloat = 7                  // half the cap height plus 1
    static let capSize = NSSize(width: 20, height: 12)

    override var isFlipped: Bool { return true }

    static func fraction(forValue v: Int) -> CGFloat {
        let db = min(64.5, max(0, -Double(v) / 2))
        for i in 1..<curve.count where db <= curve[i].db {
            let a = curve[i - 1], b = curve[i]
            return a.frac + (b.frac - a.frac) * CGFloat((db - a.db) / (b.db - a.db))
        }
        return 1
    }
    static func value(forFraction f: CGFloat) -> Int {
        let fr = min(1, max(0, f))
        for i in 1..<curve.count where fr <= curve[i].frac {
            let a = curve[i - 1], b = curve[i]
            let db = a.db + (b.db - a.db) * Double((fr - a.frac) / (b.frac - a.frac))
            return Int((-db * 2).rounded())
        }
        return MARange.levelMin
    }

    private var trackHeight: CGFloat { return bounds.height - 2 * Fader.trackPad }
    private func y(forFraction f: CGFloat) -> CGFloat { return Fader.trackPad + f * trackHeight }
    private var dragOffset: CGFloat = 0

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let cx = bounds.midX
            let top = Fader.trackPad, bottom = bounds.height - Fader.trackPad
            Theme.fill(NSRect(x: cx - 1, y: top, width: 2, height: bottom - top), radius: 1, Theme.track)
            let hy = y(forFraction: Fader.fraction(forValue: value))
            Theme.fill(NSRect(x: cx - 1, y: hy, width: 2, height: bottom - hy), radius: 1, Theme.text3)
            let cap = NSRect(x: cx - Fader.capSize.width / 2, y: hy - Fader.capSize.height / 2, width: Fader.capSize.width, height: Fader.capSize.height)
            NSGraphicsContext.saveGraphicsState()
            let sh = NSShadow(); sh.shadowColor = NSColor(white: 0, alpha: 0.7); sh.shadowBlurRadius = 3; sh.shadowOffset = NSSize(width: 0, height: -1); sh.set()
            Theme.fill(cap, radius: 3, isEnabled ? Theme.capFace : Theme.text3)
            NSGraphicsContext.restoreGraphicsState()
            Theme.capLine.setFill(); NSRect(x: cap.minX + 4, y: hy - 0.5, width: cap.width - 8, height: 1).fill()
        }
    }

    private func update(_ yy: CGFloat) {
        let v = Fader.value(forFraction: (yy - Fader.trackPad) / trackHeight)
        if v != value { value = v; onChange?(v) }
    }
    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        let p = convert(event.locationInWindow, from: nil)
        let hy = y(forFraction: Fader.fraction(forValue: value))
        if abs(p.y - hy) <= Fader.capSize.height / 2 + 2 { dragOffset = p.y - hy } else { dragOffset = 0; update(p.y) }
    }
    override func mouseDragged(with event: NSEvent) {
        guard isEnabled else { return }
        update(convert(event.locationInWindow, from: nil).y - dragOffset)
    }
    override func mouseUp(with event: NSEvent) { if isEnabled { onCommit?() } }
}

/// The dB numbers between a fader pair, on the same track geometry as Fader.
final class FaderScale: FlippedView {
    var isEnabled = true { didSet { needsDisplay = true } }
    override func draw(_ dirtyRect: NSRect) {
        let h = bounds.height - 2 * Fader.trackPad
        for db in Fader.labels {
            let y = Fader.trackPad + Fader.fraction(forValue: -db * 2) * h
            Theme.draw("\(db)", in: NSRect(x: -4, y: y - 6, width: bounds.width + 8, height: 12), font: Theme.mono(8),
                       color: isEnabled ? Theme.text3 : Theme.text4)
        }
    }
}

// MARK: - knobs

/// Value knob (aux send, pan, balance): flat body, a pointer, and a thin LED-style ring showing the value (from the minimum, or from
/// the centre for bipolar controls). Vertical drag, double-click = default.
final class Knob: NSView {
    var minValue = -255, maxValue = 255, defaultValue = 0
    var bipolar = false
    var value = 0 { didSet { if value != oldValue { needsDisplay = true } } }
    var isEnabled = true { didSet { if isEnabled != oldValue { needsDisplay = true } } }
    var accent = Theme.accent
    var onChange: ((Int) -> Void)?
    var onCommit: (() -> Void)?
    private var dragStartY: CGFloat = 0
    private var dragStartValue = 0

    override var intrinsicContentSize: NSSize { return NSSize(width: 32, height: 32) }

    override func draw(_ dirtyRect: NSRect) {
        Theme.dimmed(isEnabled ? 1 : Theme.disabledAlpha) {
            let c = NSPoint(x: bounds.midX, y: bounds.midY)
            let ringR = min(bounds.width, bounds.height) / 2 - 2
            let bodyR = ringR - 4.5
            // The kext reports an aux send that is off as -255 ("row 16, default -255"), below the knob's -129 (-inf) stop: draw it at the stop.
            let shown = min(maxValue, max(minValue, value))
            let f = CGFloat(shown - minValue) / CGFloat(max(1, maxValue - minValue))
            let a = 225 - 270 * f                              // degrees, unflipped: 225 = lower left, -45 = lower right
            let track = NSBezierPath()
            track.appendArc(withCenter: c, radius: ringR, startAngle: 225, endAngle: -45, clockwise: true)
            track.lineWidth = 2; track.lineCapStyle = .roundLineCapStyle
            Theme.track.setStroke(); track.stroke()
            let start: CGFloat = bipolar ? 90 : 225
            if abs(start - a) > 0.5 {
                let arc = NSBezierPath()
                arc.appendArc(withCenter: c, radius: ringR, startAngle: start, endAngle: a, clockwise: a < start)
                arc.lineWidth = 2; arc.lineCapStyle = .roundLineCapStyle
                accent.setStroke(); arc.stroke()
            }
            Theme.control.setFill(); NSBezierPath(ovalIn: NSRect(x: c.x - bodyR, y: c.y - bodyR, width: 2 * bodyR, height: 2 * bodyR)).fill()
            let rad = a * CGFloat.pi / 180
            let p = NSBezierPath()
            p.move(to: NSPoint(x: c.x + cos(rad) * bodyR * 0.25, y: c.y + sin(rad) * bodyR * 0.25))
            p.line(to: NSPoint(x: c.x + cos(rad) * (bodyR - 2), y: c.y + sin(rad) * (bodyR - 2)))
            p.lineWidth = 1.8; p.lineCapStyle = .roundLineCapStyle
            Theme.text.setStroke(); p.stroke()
        }
    }

    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        if event.clickCount == 2 { value = defaultValue; onChange?(value); onCommit?(); return }
        dragStartY = event.locationInWindow.y; dragStartValue = value
    }
    override func mouseDragged(with event: NSEvent) {
        guard isEnabled else { return }
        let steps = Int((event.locationInWindow.y - dragStartY) * CGFloat(maxValue - minValue) / 150)
        let v = min(maxValue, max(minValue, dragStartValue + steps))
        if v != value { value = v; onChange?(v) }
    }
    override func mouseUp(with event: NSEvent) { if isEnabled { onCommit?() } }
}

/// The on-screen level-controller knob (sidebar). The front-panel knob is an endless encoder, but this one is a bounded control that follows the
/// ring of LED dots: its pointer sits on the dot of the loudest selected level (`value`, raw 0.5 dB steps, -129 = -inf ... 0) and a drag, the
/// scroll wheel or `turn` moves it only inside that working range. The owner still sends signed detents (the kext steps every selected pair by
/// the same amount, as for the hardware encoder); the knob clamps them so nothing is sent past either end.
final class EncoderKnob: NSView {
    var onTurn: ((Int) -> Void)?
    /// Loudest selected level, raw (-129 = -inf ... 0); nil = no channel selected. The ring and the pointer are derived from it.
    var value: Int? { didSet { if value != oldValue { needsDisplay = true } } }
    var ringLevel: CGFloat? { return value.map { 1 - Fader.fraction(forValue: $0) } }
    private var lastDragY: CGFloat = 0
    private var dragAcc: CGFloat = 0
    private var scrollAcc: CGFloat = 0
    private var lastTick: CFAbsoluteTime = 0
    private var active = false { didSet { if active != oldValue { needsDisplay = true } } }
    static let pixelsPerDetent: CGFloat = 4
    static let dots = 29

    /// Applies n detents inside the working range: the pointer moves and the owner is told how many were actually applied.
    func turn(_ n: Int) {
        guard n != 0, let v = value else { return }
        if v < MARange.levelMin && n < 0 { return }                // already off: turning down does nothing
        let base = max(v, MARange.levelMin)                        // the kext reports "off" as -255, below the -inf stop: it rises from the stop
        let target = min(MARange.levelMax, max(MARange.levelMin, base + n))
        guard target != base || v < MARange.levelMin else { return }       // at an end stop: nothing to send
        value = target
        onTurn?(target - v)
    }

    override func draw(_ dirtyRect: NSRect) {
        let c = NSPoint(x: bounds.midX, y: bounds.midY)
        let ringR = min(bounds.width, bounds.height) / 2 - 4
        let bodyR = ringR - 13
        // LED ring over 270 degrees, lit up to ringLevel
        let level = ringLevel ?? 0
        let lit = Int((level * CGFloat(EncoderKnob.dots)).rounded())
        for i in 0..<EncoderKnob.dots {
            let a = (225 - 270 * CGFloat(i) / CGFloat(EncoderKnob.dots - 1)) * CGFloat.pi / 180
            let p = NSPoint(x: c.x + cos(a) * ringR, y: c.y + sin(a) * ringR)
            (i < lit ? Theme.text : Theme.ledOff).setFill()
            NSBezierPath(ovalIn: NSRect(x: p.x - 2.1, y: p.y - 2.1, width: 4.2, height: 4.2)).fill()
        }
        // body: flat disc with a faint edge
        let body = NSRect(x: c.x - bodyR, y: c.y - bodyR, width: 2 * bodyR, height: 2 * bodyR)
        NSGraphicsContext.saveGraphicsState()
        let sh = NSShadow(); sh.shadowColor = NSColor(white: 0, alpha: 0.6); sh.shadowBlurRadius = 8; sh.shadowOffset = NSSize(width: 0, height: -2); sh.set()
        Theme.control.setFill(); NSBezierPath(ovalIn: body).fill()
        NSGraphicsContext.restoreGraphicsState()
        let edge = NSBezierPath(ovalIn: body.insetBy(dx: 0.5, dy: 0.5)); edge.lineWidth = 1
        Theme.hairline.setStroke(); edge.stroke()
        // pointer dot on the same angle as the last lit LED (225 degrees = the first dot, -45 = the last)
        let a = (225 - 270 * level) * CGFloat.pi / 180
        let pr = bodyR - 9
        let dot = NSPoint(x: c.x + cos(a) * pr, y: c.y + sin(a) * pr)
        ((active ? Theme.accent : Theme.text).withAlphaComponent(ringLevel == nil ? 0.35 : 1)).setFill()
        NSBezierPath(ovalIn: NSRect(x: dot.x - 3, y: dot.y - 3, width: 6, height: 6)).fill()
    }

    override func mouseDown(with event: NSEvent) { lastDragY = event.locationInWindow.y; dragAcc = 0; active = true }
    override func mouseDragged(with event: NSEvent) {
        let y = event.locationInWindow.y
        dragAcc += y - lastDragY                         // up = clockwise
        lastDragY = y
        let n = Int(dragAcc / EncoderKnob.pixelsPerDetent)
        if n != 0 { dragAcc -= CGFloat(n) * EncoderKnob.pixelsPerDetent; turn(n) }
    }
    override func mouseUp(with event: NSEvent) { active = false }

    override func scrollWheel(with event: NSEvent) {
        var dy = event.scrollingDeltaY
        if event.isDirectionInvertedFromDevice { dy = -dy }   // wheel or fingers up = clockwise, whatever the scroll setting
        if event.hasPreciseScrollingDeltas {
            scrollAcc += dy
            let n = Int(scrollAcc / 6)
            if n != 0 { scrollAcc -= CGFloat(n) * 6; flash(); turn(n) }
        } else if dy != 0 {
            // One wheel notch = one count; quick successive notches accelerate: count x (13 - interval/6) under 78 ms.
            let count = dy > 0 ? 1 : -1
            let now = CFAbsoluteTimeGetCurrent()
            let ms = Int((now - lastTick) * 1000)
            lastTick = now
            flash()
            turn(ms < 78 ? count * max(1, 13 - ms / 6) : count)
        }
    }

    private func flash() {
        active = true
        NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(endFlash), object: nil)
        perform(#selector(endFlash), with: nil, afterDelay: 0.35)
    }
    @objc private func endFlash() { active = false }
}

// MARK: - meters

/// Slim LED meter, -64...0 dB (floor of the reference conversion, A2-P9 §7.5): segmented bar with green/yellow/red zones, a peak-hold
/// segment and a clip LED at the end. Opaque: it fills its bounds with `surface` first.
final class MeterBar: NSView {
    var db = -64 { didSet { advance() } }
    var isEnabled = true { didSet { if isEnabled != oldValue { needsDisplay = true } } }
    var horizontal = false
    var showsClip = true
    var surface = Theme.surface
    private(set) var hold = -64
    private var holdStamp: CFAbsoluteTime = 0
    private var clipStamp: CFAbsoluteTime = -100
    private var drawn: (Int, Int, Bool) = (-1, -1, false)

    // Display law: dB -> fraction of the bar (not the fader law; the meter only shows level).
    static let law: [(db: Double, f: CGFloat)] = [(-64, 0), (-48, 0.08), (-36, 0.18), (-24, 0.36), (-18, 0.48), (-12, 0.62), (-6, 0.80), (-3, 0.90), (0, 1)]
    static let scaleMarks = [0, -6, -12, -24, -48]
    static let clipLength: CGFloat = 4, clipGap: CGFloat = 3
    static let segment: CGFloat = 2, segmentGap: CGFloat = 1

    static func fraction(_ db: Int) -> CGFloat {
        let d = min(0, max(-64, Double(db)))
        for i in 1..<law.count where d <= law[i].db {
            let a = law[i - 1], b = law[i]
            return a.f + (b.f - a.f) * CGFloat((d - a.db) / (b.db - a.db))
        }
        return 1
    }

    override var isOpaque: Bool { return true }
    override var isFlipped: Bool { return false }

    private var length: CGFloat { return horizontal ? bounds.width : bounds.height }
    private var barLength: CGFloat { return length - (showsClip ? MeterBar.clipLength + MeterBar.clipGap : 0) }
    private var segmentCount: Int { return max(1, Int((barLength + MeterBar.segmentGap) / (MeterBar.segment + MeterBar.segmentGap))) }
    private func litCount(_ v: Int) -> Int { return v <= -64 ? 0 : Int((MeterBar.fraction(v) * CGFloat(segmentCount)).rounded(.up)) }

    private func advance() {
        let now = CFAbsoluteTimeGetCurrent()
        if db >= hold { hold = db; holdStamp = now }
        else if now - holdStamp > 1.2 { hold = max(db, hold - 2) }
        if db >= 0 { clipStamp = now }
        let state = (litCount(db), litCount(hold), now - clipStamp < 1.5)
        if state != drawn { needsDisplay = true }
    }

    override func draw(_ dirtyRect: NSRect) {
        surface.setFill(); bounds.fill()
        let n = segmentCount
        let lit = isEnabled ? litCount(db) : 0
        let holdSeg = isEnabled ? litCount(hold) : 0
        let clipped = isEnabled && CFAbsoluteTimeGetCurrent() - clipStamp < 1.5
        drawn = (lit, holdSeg, clipped)
        let pitch = MeterBar.segment + MeterBar.segmentGap
        let yellowFrom = Int(MeterBar.fraction(-12) * CGFloat(n)), redFrom = Int(MeterBar.fraction(-3) * CGFloat(n))
        let off = isEnabled ? Theme.ledOff : Theme.ledOff.withAlphaComponent(0.45)
        func rect(_ i: Int) -> NSRect {
            let o = CGFloat(i) * pitch
            return horizontal ? NSRect(x: o, y: 0, width: MeterBar.segment, height: bounds.height)
                              : NSRect(x: 0, y: o, width: bounds.width, height: MeterBar.segment)
        }
        func zone(_ i: Int) -> NSColor { return i >= redFrom ? Theme.meterHigh : (i >= yellowFrom ? Theme.meterMid : Theme.meterLow) }
        // Group consecutive segments of one colour into one path fill.
        var current: NSColor? = nil
        let path = NSBezierPath()
        for i in 0..<n {
            let c = (i < lit || i == holdSeg - 1) ? zone(i) : off
            if current == nil || c != current! {
                if let cc = current { cc.setFill(); path.fill(); path.removeAllPoints() }
                current = c
            }
            path.appendRect(rect(i))
        }
        if let cc = current { cc.setFill(); path.fill() }
        if showsClip {
            let r = horizontal ? NSRect(x: bounds.width - MeterBar.clipLength, y: 0, width: MeterBar.clipLength, height: bounds.height)
                               : NSRect(x: 0, y: bounds.height - MeterBar.clipLength, width: bounds.width, height: MeterBar.clipLength)
            (clipped ? Theme.meterHigh : off).setFill(); r.fill()
        }
    }
}

/// dB marks for a vertical MeterBar of the same height (meter law, not the fader law).
final class MeterScale: NSView {
    var align: NSTextAlignment = .right
    override func draw(_ dirtyRect: NSRect) {
        let barLen = bounds.height - MeterBar.clipLength - MeterBar.clipGap
        for d in MeterBar.scaleMarks {
            let y = MeterBar.fraction(d) * barLen
            Theme.draw(d == 0 ? "0" : "\(-d)", in: NSRect(x: 0, y: y - 6, width: bounds.width, height: 12), font: Theme.mono(8), color: Theme.text4, align: align)
        }
    }
}

// MARK: - containers and text

/// Rounded surface with an optional small caption; content is placed by the owner (flipped coordinates).
final class Card: FlippedView {
    var title: String? { didSet { needsDisplay = true } }
    var color = Theme.surface
    static let contentTop: CGFloat = 40
    init(title: String?) { self.title = title; super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 12, color)
        if let t = title { Theme.caption(t, in: NSRect(x: 18, y: 12, width: bounds.width - 36, height: 16), align: .left) }
    }
}

/// Section caption above a group of strips.
final class SectionLabel: FlippedView {
    let text: String
    init(_ text: String) { self.text = text; super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func draw(_ dirtyRect: NSRect) { Theme.caption(text, in: NSRect(x: 2, y: 0, width: bounds.width - 2, height: 14), align: .left) }
}

/// Sample-rate display: big figure on a dark well, green when the device reports lock, red otherwise, with a padlock drawn in code.
final class RateDisplay: FlippedView {
    var text = "44.1" { didSet { needsDisplay = true } }
    var locked = true { didSet { needsDisplay = true } }
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 10, Theme.well)
        let c = locked ? Theme.green : Theme.red
        let big = Theme.attributed(text, font: Theme.mono(34, .light), color: c)
        let unit = Theme.attributed(" kHz", font: Theme.font(14, .medium), color: c.withAlphaComponent(0.75))
        let s = NSMutableAttributedString(attributedString: big); s.append(unit)
        let sz = s.size()
        s.draw(at: NSPoint(x: 20, y: bounds.midY - sz.height / 2))
        // padlock, top-right
        let lx = bounds.maxX - 30, ly = bounds.midY - 2
        let body = NSRect(x: lx, y: ly, width: 14, height: 11)
        Theme.fill(body, radius: 2, c)
        let sh = NSBezierPath()
        let left = lx + 3, right = lx + 11, topY = ly - 6
        sh.move(to: NSPoint(x: left, y: ly))
        sh.line(to: NSPoint(x: left, y: topY + 3))
        sh.appendArc(withCenter: NSPoint(x: lx + 7, y: topY + 3), radius: 4, startAngle: 180, endAngle: 0, clockwise: false)
        sh.line(to: NSPoint(x: right, y: locked ? ly : ly - 3))
        if !locked { sh.transform(using: AffineTransform(translationByX: 0, byY: -3)) }
        sh.lineWidth = 2; c.setStroke(); sh.stroke()
    }
}
