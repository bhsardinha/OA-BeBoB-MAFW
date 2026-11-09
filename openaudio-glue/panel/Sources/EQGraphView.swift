// OpenAudio Glue app - one channel's EQ as an XY graph: 20 Hz-20 kHz (log) across, -12...+12 dB up. The grid is drawn once into the
// view's layer; the response curve is a CAShapeLayer and the four band dots are small layers, so dragging redraws neither the grid
// nor the card (the panel's GPU-first rule, AdvancedWindow.swift:243). Drag a dot to set its frequency and gain (each band stays in
// its own range, GlueEQMath.ranges); double-click a dot to put it back at 0 dB.
import AppKit

final class EQGraphView: FlippedView {
    static let minHz = 20.0, maxHz = 20000.0, rangeDb = 12.0
    static let bandColors = [Theme.hex(0xE8894A), Theme.hex(0xEFC43B), Theme.hex(0x3DC97B), Theme.accent]

    var bands: [GlueBand] = GlueChannel().bands { didSet { if bands != oldValue { updateCurve() } } }
    var rate = 48000.0 { didSet { updateCurve() } }
    var onChange: (([GlueBand]) -> Void)?
    var isEnabled = true { didSet { alphaValue = isEnabled ? 1 : Theme.disabledAlpha } }

    private let curve = CAShapeLayer(), fill = CAShapeLayer()
    private var dots: [CALayer] = []
    private var dragging = -1
    private let readout = CATextLayer()

    override init(frame: NSRect) {
        super.init(frame: frame)
        wantsLayer = true
        layerContentsRedrawPolicy = .onSetNeedsDisplay
        fill.fillColor = Theme.accent.withAlphaComponent(0.10).cgColor
        fill.strokeColor = nil
        curve.fillColor = nil
        curve.strokeColor = Theme.accent.cgColor
        curve.lineWidth = 1.6
        layer?.addSublayer(fill)
        layer?.addSublayer(curve)
        for c in EQGraphView.bandColors {
            let d = CALayer()
            d.bounds = CGRect(x: 0, y: 0, width: 10, height: 10)
            d.cornerRadius = 5
            d.backgroundColor = c.cgColor
            d.borderColor = Theme.well.cgColor
            d.borderWidth = 1.5
            layer?.addSublayer(d)
            dots.append(d)
        }
        readout.fontSize = 9
        readout.font = Theme.mono(9)
        readout.foregroundColor = Theme.text2.cgColor
        readout.alignmentMode = kCAAlignmentRight
        readout.contentsScale = 2
        layer?.addSublayer(readout)
        for l in [fill, curve, readout] + dots { l.actions = ["position": NSNull(), "bounds": NSNull(), "path": NSNull(), "contents": NSNull()] }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    // MARK: geometry
    private var plot: NSRect { return bounds.insetBy(dx: 1, dy: 1) }
    private func x(_ hz: Double) -> CGFloat {
        let t = log10(hz / EQGraphView.minHz) / log10(EQGraphView.maxHz / EQGraphView.minHz)
        return plot.minX + CGFloat(t) * plot.width
    }
    private func hz(_ x: CGFloat) -> Double {
        let t = Double((x - plot.minX) / plot.width)
        return EQGraphView.minHz * pow(EQGraphView.maxHz / EQGraphView.minHz, min(max(t, 0), 1))
    }
    private func y(_ db: Double) -> CGFloat { return plot.midY - CGFloat(db / EQGraphView.rangeDb) * plot.height / 2 }
    private func db(_ y: CGFloat) -> Double { return Double((plot.midY - y) / (plot.height / 2)) * EQGraphView.rangeDb }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        readout.frame = CGRect(x: 4, y: 3, width: bounds.width - 8, height: 12)       // top right while dragging
        needsDisplay = true
        updateCurve()
    }

    // MARK: static grid (drawn into the layer once per size)
    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 6, Theme.well)
        let grid = NSBezierPath()
        for hz in [50.0, 100, 200, 500, 1000, 2000, 5000, 10000] {
            grid.move(to: NSPoint(x: x(hz), y: plot.minY)); grid.line(to: NSPoint(x: x(hz), y: plot.maxY))
        }
        for d in [-6.0, 6] { grid.move(to: NSPoint(x: plot.minX, y: y(d))); grid.line(to: NSPoint(x: plot.maxX, y: y(d))) }
        grid.lineWidth = 1
        NSColor(white: 1, alpha: 0.05).setStroke(); grid.stroke()
        let zero = NSBezierPath()
        zero.move(to: NSPoint(x: plot.minX, y: y(0))); zero.line(to: NSPoint(x: plot.maxX, y: y(0)))
        NSColor(white: 1, alpha: 0.12).setStroke(); zero.stroke()
        for (hz, t) in [(100.0, "100"), (1000.0, "1k"), (10000.0, "10k")] {        // frequencies along the bottom
            Theme.caption(t, in: NSRect(x: x(hz) + 2, y: plot.maxY - 11, width: 30, height: 10), color: Theme.text4, align: .left, size: 8)
        }
        Theme.caption("+12", in: NSRect(x: plot.minX + 3, y: plot.minY + 2, width: 22, height: 10), color: Theme.text4, align: .left, size: 8)
        Theme.caption("0", in: NSRect(x: plot.minX + 3, y: y(0) - 11, width: 22, height: 10), color: Theme.text4, align: .left, size: 8)
    }

    // MARK: curve and dots
    private func updateCurve() {
        guard bounds.width > 4 else { return }
        let p = CGMutablePath()
        let n = Int(plot.width / 2)
        var first = true
        for i in 0...n {
            let px = plot.minX + CGFloat(i) * plot.width / CGFloat(n)
            let m = GlueEQMath.magnitudeDb(bands: bands, rate: rate, at: hz(px))
            let pt = CGPoint(x: px, y: y(max(-EQGraphView.rangeDb, min(EQGraphView.rangeDb, m))))
            if first { p.move(to: pt); first = false } else { p.addLine(to: pt) }
        }
        curve.path = p
        let f = p.mutableCopy()!
        f.addLine(to: CGPoint(x: plot.maxX, y: y(0))); f.addLine(to: CGPoint(x: plot.minX, y: y(0))); f.closeSubpath()
        fill.path = f
        for (i, b) in bands.enumerated() where i < dots.count {
            dots[i].position = CGPoint(x: x(b.freq), y: y(b.gain))
            dots[i].opacity = b.gain == 0 ? 0.55 : 1
        }
    }

    private func showReadout(_ i: Int) {
        guard i >= 0 && i < bands.count else { readout.string = ""; return }
        let b = bands[i]
        let f = b.freq >= 1000 ? String(format: "%.1fk", b.freq / 1000) : String(format: "%.0f", b.freq)
        readout.string = "\(GlueEQMath.bandNames[i]) \(f) Hz \(String(format: "%+.1f", b.gain)) dB"
    }

    // MARK: mouse
    private func nearest(_ p: NSPoint) -> Int {
        var best = -1, dist = CGFloat(14)
        for (i, b) in bands.enumerated() {
            let d = hypot(p.x - x(b.freq), p.y - y(b.gain))
            if d < dist { dist = d; best = i }
        }
        return best
    }

    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        let p = convert(event.locationInWindow, from: nil)
        dragging = nearest(p)
        if dragging >= 0 && event.clickCount == 2 {
            bands[dragging].gain = 0
            onChange?(bands)
        }
        showReadout(dragging)
    }

    override func mouseDragged(with event: NSEvent) {
        guard isEnabled, dragging >= 0 else { return }
        let p = convert(event.locationInWindow, from: nil)
        let (f, g) = GlueEQMath.clamp(band: dragging, freq: hz(p.x), gain: (db(p.y) * 2).rounded() / 2, rate: rate)   // 0.5 dB steps
        if f != bands[dragging].freq || g != bands[dragging].gain {
            bands[dragging].freq = f
            bands[dragging].gain = g
            onChange?(bands)
        }
        showReadout(dragging)
    }

    override func mouseUp(with event: NSEvent) { dragging = -1; readout.string = "" }
}
