// Plan v8 AP-5.6 (user, 2026-10-21; a clone feature, the reference panel has no such view) -- the big horizontal meter: a window about 80 % of
// the screen width with two horizontal peak bars (L / R) of the selected unit. Source selectable (SW 1/2 default, Out 1/2, Phones, In 1/2);
// peak only, from the driver's meter page (the same page the panel's meters read, ~30 Hz); dBFS -60 ... 0 on a dB-linear scale (StatusLogic
// BigMeterScale). Options (per Mac, UserDefaults): always on top, pin to the top / bottom of the screen or free, transparency, click-through.
import AppKit

enum BigMeterSource: String {
    case sw12, out12, phones, in12
    static let all: [BigMeterSource] = [.sw12, .out12, .phones, .in12]
    var title: String {
        switch self { case .sw12: return "SW 1/2"; case .out12: return "Out 1/2"; case .phones: return "Phones"; case .in12: return "In 1/2" }
    }
    func slots(_ p: PeakMap) -> (Int, Int) {
        switch self {
        case .sw12: return (p.swReturn(1), p.swReturn(2))
        case .out12: return (p.output(1), p.output(2))
        case .phones: return (p.phones(1), p.phones(2))
        case .in12: return (p.input(1), p.input(2))
        }
    }
}

enum BigMeterPin: String { case free, top, bottom }

/// Per-Mac options of the big meter.
enum BigMeterPrefs {
    private static let d = UserDefaults.standard
    static var source: BigMeterSource {
        get { return BigMeterSource(rawValue: d.string(forKey: "BigMeterSource") ?? "") ?? .sw12 }
        set { d.set(newValue.rawValue, forKey: "BigMeterSource") }
    }
    static var onTop: Bool { get { return d.object(forKey: "BigMeterOnTop") as? Bool ?? true } set { d.set(newValue, forKey: "BigMeterOnTop") } }
    static var pin: BigMeterPin {
        get { return BigMeterPin(rawValue: d.string(forKey: "BigMeterPin") ?? "") ?? .bottom }
        set { d.set(newValue.rawValue, forKey: "BigMeterPin") }
    }
    static var transparent: Bool { get { return d.bool(forKey: "BigMeterTransparent") } set { d.set(newValue, forKey: "BigMeterTransparent") } }
    static var clickThrough: Bool { get { return d.bool(forKey: "BigMeterClickThrough") } set { d.set(newValue, forKey: "BigMeterClickThrough") } }
    static var origin: NSPoint? {
        get { return d.string(forKey: "BigMeterOrigin").map { NSPointFromString($0) } }
        set { d.set(newValue.map { NSStringFromPoint($0) }, forKey: "BigMeterOrigin") }
    }
}

/// LED segments like an old stereo deck (user, 2026-10-21): BigMeterScale.segments blocks over -60 ... 0 dBFS, green / amber / red zones,
/// unlit blocks faintly in their own colour, the peak hold as one bright block.
/// Low-CPU design (user, 2026-10-21; same idea as the official panel's CMAVUMeter, v1.11.2: the graphic is rendered once into a cached
/// off-screen buffer, FinishCreateSelf @0xc8ad0; SetValue @0xc8c40 returns without drawing when neither the level nor the peak changed;
/// DrawMeterToPort @0xc9034 copies only the lit part of the cached image into the meter's own rect). Here: the background, unlit blocks,
/// scale and labels are drawn once (again only when the size or the title changes); the lit blocks are one cached image in a layer that is
/// only resized, a whole segment at a time; the hold is a plain layer; the readouts are small views that redraw only when their text
/// changes. A tick that changes no segment changes nothing; Core Animation composites the rest on the GPU.
final class BigMeterView: NSView {
    var title = "" { didSet { if title != oldValue { setNeedsDisplay(NSRect(x: 0, y: 0, width: bounds.width, height: BigMeterView.top)) } } }
    var menuProvider: (() -> NSMenu?)?
    private(set) var level: (Double, Double) = (-Double.infinity, -Double.infinity)     // displayed (with fall-back)
    private var hold: (Double, Double) = (-Double.infinity, -Double.infinity)
    private var holdTime: (CFAbsoluteTime, CFAbsoluteTime) = (0, 0)
    private var last: CFAbsoluteTime = 0
    static let fallDBPerSecond = 24.0, holdSeconds = 2.0
    static let pad: CGFloat = 18, labelW: CGFloat = 20, readW: CGFloat = 104, barH: CGFloat = 34, top: CGFloat = 32, scaleH: CGFloat = 22
    static let segInset: CGFloat = 4, segGap: CGFloat = 3, unlitAlpha: CGFloat = 0.14

    private final class Readout: FlippedView {
        var text = "" { didSet { if text != oldValue { needsDisplay = true } } }
        var hot = false { didSet { if hot != oldValue { needsDisplay = true } } }
        override init(frame: NSRect) { super.init(frame: frame); wantsLayer = true; layerContentsRedrawPolicy = .onSetNeedsDisplay }
        required init?(coder: NSCoder) { fatalError("unused") }
        override func draw(_ dirtyRect: NSRect) {
            Theme.draw(text, in: bounds, font: Theme.mono(24, .semibold), color: hot ? Theme.red : Theme.text, align: .right)
        }
    }
    private struct Row { let lit = CALayer(), hold = CALayer(); let readout = Readout(frame: .zero) }
    private let rows = [Row(), Row()]
    private var shown = [(-1, -1), (-1, -1)]                 // (lit segments, hold segment) per row, as on screen
    private var renderedFor: (NSSize, CGFloat)?

    override init(frame: NSRect) {
        super.init(frame: frame)
        wantsLayer = true
        layerContentsRedrawPolicy = .onSetNeedsDisplay
        let still: [String: CAAction] = ["position": NSNull(), "bounds": NSNull(), "frame": NSNull(), "hidden": NSNull(), "contents": NSNull(),
                                          "backgroundColor": NSNull()]
        for r in rows {
            for l in [r.lit, r.hold] { l.actions = still; l.anchorPoint = .zero }
            r.lit.contentsGravity = kCAGravityLeft                  // cached image at its natural size from the left; the frame crops it
            r.lit.masksToBounds = true
            r.hold.cornerRadius = 1.5
            addSubview(r.readout)
        }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    override var isFlipped: Bool { return true }

    private var barX: CGFloat { return BigMeterView.pad + BigMeterView.labelW }
    private var barW: CGFloat { return bounds.width - barX - BigMeterView.readW - BigMeterView.pad }
    private func rowY(_ i: Int) -> CGFloat { return BigMeterView.top + CGFloat(i) * (BigMeterView.barH + BigMeterView.scaleH) }
    private var pitch: CGFloat { return barW / CGFloat(BigMeterScale.segments) }
    /// Segment i of row `row` (view coordinates); x positions follow the dB scale exactly.
    private func segRect(_ i: Int, row: Int) -> CGRect {
        let x0 = (barX + CGFloat(i) * pitch).rounded(), x1 = (barX + CGFloat(i + 1) * pitch).rounded() - BigMeterView.segGap
        return CGRect(x: x0, y: rowY(row) + BigMeterView.segInset, width: max(1, x1 - x0), height: BigMeterView.barH - 2 * BigMeterView.segInset)
    }
    private static func zoneColor(_ z: Int) -> NSColor { return z == 2 ? Theme.red : (z == 1 ? Theme.solo : Theme.green) }

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); needsLayout = true }
    override func viewDidChangeBackingProperties() { super.viewDidChangeBackingProperties(); renderedFor = nil; needsLayout = true }
    override func layout() {
        super.layout()
        guard let root = layer else { return }
        let scale = window?.backingScaleFactor ?? 2
        if let r = renderedFor, r.0 == bounds.size, r.1 == scale { return }
        renderedFor = (bounds.size, scale)
        // all segments lit, rendered once per size (row 0 geometry, relative to the bar's start)
        let origin = segRect(0, row: 0).origin
        let w = max(1, Int((barW * scale).rounded())), h = max(1, Int((segRect(0, row: 0).height * scale).rounded()))
        var img: CGImage?
        if let ctx = CGContext(data: nil, width: w, height: h, bitsPerComponent: 8, bytesPerRow: 0, space: CGColorSpaceCreateDeviceRGB(),
                               bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue) {
            ctx.scaleBy(x: scale, y: scale)
            for i in 0..<BigMeterScale.segments {
                let r = segRect(i, row: 0)
                ctx.addPath(CGPath(roundedRect: CGRect(x: r.minX - origin.x, y: 0, width: r.width, height: r.height), cornerWidth: 1.5, cornerHeight: 1.5, transform: nil))
                ctx.setFillColor(BigMeterView.zoneColor(BigMeterScale.zone(segment: i)).cgColor)
                ctx.fillPath()
            }
            img = ctx.makeImage()
        }
        for (i, r) in rows.enumerated() {
            if r.lit.superlayer !== root { root.addSublayer(r.lit); root.addSublayer(r.hold) }
            r.lit.contentsScale = scale
            r.lit.contents = img
            r.readout.frame = NSRect(x: bounds.width - BigMeterView.pad - BigMeterView.readW, y: rowY(i) + BigMeterView.barH / 2 - 15,
                                     width: BigMeterView.readW, height: 30)
        }
        shown = [(-1, -1), (-1, -1)]
        apply()
    }

    /// One meter-page reading (raw peaks of L and R).
    func update(rawL: UInt32, rawR: UInt32) {
        let now = CFAbsoluteTimeGetCurrent()
        let dt = last == 0 ? 0 : min(0.5, now - last)
        last = now
        func step(_ shown: Double, _ raw: UInt32, _ h: Double, _ ht: CFAbsoluteTime) -> (Double, Double, CFAbsoluteTime) {
            let db = BigMeterScale.db(raw: raw)
            let fallen = shown.isFinite ? shown - BigMeterView.fallDBPerSecond * dt : shown
            let s = max(db, fallen)
            if s >= h || now - ht > BigMeterView.holdSeconds { return (s, s, now) }
            return (s, h, ht)
        }
        let l = step(level.0, rawL, hold.0, holdTime.0), r = step(level.1, rawR, hold.1, holdTime.1)
        level = (l.0, r.0); hold = (l.1, r.1); holdTime = (l.2, r.2)
        apply()
    }

    /// Like CMAVUMeter::SetValue: nothing is touched unless the lit count or the hold segment changed.
    private func apply() {
        for (i, (lv, h)) in [(level.0, hold.0), (level.1, hold.1)].enumerated() {
            let r = rows[i]
            let n = BigMeterScale.litSegments(lv), hs = BigMeterScale.litSegments(h)
            r.readout.text = h.isFinite && h > -100 ? String(format: "%.1f", h) : "-\u{221E}"
            r.readout.hot = h > -6
            if shown[i] == (n, hs) { continue }
            shown[i] = (n, hs)
            let first = segRect(0, row: i)
            let end = n == 0 ? first.minX : segRect(n - 1, row: i).maxX
            r.lit.frame = CGRect(x: first.minX, y: first.minY, width: end - first.minX, height: first.height)
            r.hold.isHidden = hs == 0
            if hs > 0 {
                r.hold.frame = segRect(hs - 1, row: i)
                r.hold.backgroundColor = BigMeterView.zoneColor(BigMeterScale.zone(segment: hs - 1)).cgColor
            }
        }
    }

    override func menu(for event: NSEvent) -> NSMenu? { return menuProvider?() }

    /// Static content only: background, title, channel labels, wells, unlit segments and the scale.
    override func draw(_ dirtyRect: NSRect) {
        let pad = BigMeterView.pad, barH = BigMeterView.barH
        Theme.fill(bounds, radius: 14, Theme.window)
        Theme.draw(title, in: NSRect(x: pad, y: 9, width: bounds.width - 2 * pad, height: 16), font: Theme.font(11, .semibold), color: Theme.text3, align: .left)
        for (i, name) in ["L", "R"].enumerated() {
            let y = rowY(i)
            Theme.draw(name, in: NSRect(x: pad, y: y + barH / 2 - 9, width: BigMeterView.labelW, height: 18), font: Theme.font(13, .semibold), color: Theme.text2, align: .left)
            Theme.fill(NSRect(x: barX - 3, y: y, width: barW + 3, height: barH), radius: 4, Theme.well)
            for s in 0..<BigMeterScale.segments {
                Theme.fill(segRect(s, row: i), radius: 1.5, BigMeterView.zoneColor(BigMeterScale.zone(segment: s)).withAlphaComponent(BigMeterView.unlitAlpha))
            }
        }
        // scale between the bars: marks every 3 dB, labels every 6 dB
        let sy = BigMeterView.top + barH, scaleH = BigMeterView.scaleH
        for m in BigMeterScale.marks {
            let x = barX + BigMeterScale.fraction(Double(m)) * barW - (m == 0 ? BigMeterView.segGap : 0)
            let major = m % BigMeterScale.labelStep == 0
            Theme.text3.setFill(); NSRect(x: x - 0.5, y: sy + 2, width: 1, height: major ? 6 : 3).fill()
            Theme.text3.setFill(); NSRect(x: x - 0.5, y: sy + scaleH - (major ? 8 : 5), width: 1, height: major ? 6 : 3).fill()
            if major {
                Theme.draw("\(m)", in: NSRect(x: x - 16, y: sy + 5, width: 32, height: 12), font: Theme.mono(9), color: Theme.text3)
            }
        }
    }
}

/// The window and its 30 Hz timer. `unit` returns the unit to show (the status menu's selected one, else the front panel's).
final class BigMeterController: NSObject {
    var unit: (() -> PanelWindowController?)?
    private var window: NSWindow?
    private let view = BigMeterView()
    private var timer: Timer?
    static let height: CGFloat = 150

    var isShown: Bool { return window?.isVisible ?? false }

    func toggle() { if isShown { hide() } else { show() } }

    func show() {
        if window == nil {
            let w = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 800, height: BigMeterController.height), styleMask: [.borderless], backing: .buffered, defer: false)
            w.isReleasedWhenClosed = false
            w.backgroundColor = .clear
            w.isOpaque = false
            w.hasShadow = true
            w.collectionBehavior = [.canJoinAllSpaces, .fullScreenAuxiliary]
            w.contentView = view
            view.menuProvider = { [weak self] in self?.optionsMenu() }
            window = w
        }
        applyOptions()
        window?.orderFrontRegardless()
        if timer == nil {
            let t = Timer(timeInterval: 1.0 / 30.0, repeats: true) { [weak self] _ in self?.tick() }
            RunLoop.main.add(t, forMode: .commonModes)
            timer = t
        }
    }

    func hide() {
        if let w = window, BigMeterPrefs.pin == .free { BigMeterPrefs.origin = w.frame.origin }
        window?.orderOut(nil)
        timer?.invalidate(); timer = nil
    }

    /// Size and position from the options: about 80 % of the screen's visible width, centred, pinned at the top or the bottom, or where it was left.
    func applyOptions() {
        guard let w = window, let screen = w.screen ?? NSScreen.main else { return }
        let vis = screen.visibleFrame
        let width = (vis.width * 0.8).rounded()
        var origin = NSPoint(x: vis.midX - width / 2, y: vis.minY + 12)
        switch BigMeterPrefs.pin {
        case .top: origin.y = vis.maxY - BigMeterController.height - 12
        case .bottom: break
        case .free: if let o = BigMeterPrefs.origin { origin = o }
        }
        w.setFrame(NSRect(origin: origin, size: NSSize(width: width, height: BigMeterController.height)), display: true)
        w.level = BigMeterPrefs.onTop ? .floating : .normal
        w.alphaValue = BigMeterPrefs.transparent ? 0.72 : 1
        w.ignoresMouseEvents = BigMeterPrefs.clickThrough
        w.isMovableByWindowBackground = BigMeterPrefs.pin == .free
    }

    private func tick() {
        if let w = window, !benchIgnoresOcclusion, !w.occlusionState.contains(.visible) { return }          // covered or on another space: nothing to show
        guard let c = unit?() else { view.title = "No interface"; view.update(rawL: 0, rawR: 0); return }
        let src = BigMeterPrefs.source
        view.title = "\(c.displayName.isEmpty ? c.device.info.name : c.displayName)  \u{00B7}  \(src.title)"
        let (a, b) = src.slots(c.ctx.peaks)
        guard let raw = c.device.readPeaks(), a >= 0, b >= 0, a < raw.count, b < raw.count else { view.update(rawL: 0, rawR: 0); return }
        view.update(rawL: raw[a], rawR: raw[b])
    }

    /// Options, shared by the status menu and the meter's own right-click menu.
    func optionsMenu() -> NSMenu {
        let m = NSMenu()
        let src = NSMenu()
        for s in BigMeterSource.all {
            let i = src.addItem(withTitle: s.title, action: #selector(chooseSource(_:)), keyEquivalent: "")
            i.target = self; i.representedObject = s.rawValue; i.state = BigMeterPrefs.source == s ? .on : .off
        }
        let srcItem = m.addItem(withTitle: "Source", action: nil, keyEquivalent: ""); srcItem.submenu = src
        let pin = NSMenu()
        for (p, t) in [(BigMeterPin.bottom, "Bottom of the screen"), (.top, "Top of the screen"), (.free, "Free (drag it)")] {
            let i = pin.addItem(withTitle: t, action: #selector(choosePin(_:)), keyEquivalent: "")
            i.target = self; i.representedObject = p.rawValue; i.state = BigMeterPrefs.pin == p ? .on : .off
        }
        let pinItem = m.addItem(withTitle: "Position", action: nil, keyEquivalent: ""); pinItem.submenu = pin
        func flag(_ t: String, _ on: Bool, _ sel: Selector) { let i = m.addItem(withTitle: t, action: sel, keyEquivalent: ""); i.target = self; i.state = on ? .on : .off }
        flag("Always on Top", BigMeterPrefs.onTop, #selector(toggleOnTop))
        flag("Transparent", BigMeterPrefs.transparent, #selector(toggleTransparent))
        flag("Click-Through", BigMeterPrefs.clickThrough, #selector(toggleClickThrough))
        m.addItem(NSMenuItem.separator())
        let close = m.addItem(withTitle: isShown ? "Hide Big Meter" : "Show Big Meter", action: #selector(toggleShown), keyEquivalent: ""); close.target = self
        return m
    }

    @objc private func chooseSource(_ i: NSMenuItem) { if let r = i.representedObject as? String, let s = BigMeterSource(rawValue: r) { BigMeterPrefs.source = s } }
    @objc private func choosePin(_ i: NSMenuItem) {
        if let r = i.representedObject as? String, let p = BigMeterPin(rawValue: r) {
            if p == .free, let w = window { BigMeterPrefs.origin = w.frame.origin }
            BigMeterPrefs.pin = p; applyOptions()
        }
    }
    @objc private func toggleOnTop() { BigMeterPrefs.onTop = !BigMeterPrefs.onTop; applyOptions() }
    @objc private func toggleTransparent() { BigMeterPrefs.transparent = !BigMeterPrefs.transparent; applyOptions() }
    @objc private func toggleClickThrough() { BigMeterPrefs.clickThrough = !BigMeterPrefs.clickThrough; applyOptions() }
    @objc private func toggleShown() { toggle() }
}
