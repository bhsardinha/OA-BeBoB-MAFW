// Panel look: the token set every view draws with (colours, type, metrics) and a few drawing helpers. Dark and minimal: areas are
// separated by tonal steps and spacing instead of borders, type is white/grey, one accent colour, and the semantic colours (mute red,
// solo amber, aux green, meter zones) only where a state matters. Everything is drawn in code with the system font; the only image is the front-panel photo on the about tab (Resources/fw1814-front.png).
import AppKit

enum Theme {
    static func hex(_ v: UInt32, _ a: CGFloat = 1) -> NSColor {
        return NSColor(srgbRed: CGFloat((v >> 16) & 0xff) / 255, green: CGFloat((v >> 8) & 0xff) / 255, blue: CGFloat(v & 0xff) / 255, alpha: a)
    }

    // MARK: surfaces, darkest first
    static let chrome = hex(0x0B0C0D)        // header band (and the window background behind the title bar)
    static let window = hex(0x101113)        // tab area
    static let sidebar = hex(0x15161A)       // right-hand sidebar
    static let surface = hex(0x191B1E)       // strips, cards
    static let raised = hex(0x202327)        // segmented-bar track, inset rows
    static let control = hex(0x272A2F)       // buttons (off), knob bodies
    static let controlHi = hex(0x33373D)     // pressed / selected segment
    static let well = hex(0x08090A)          // LCD, meter wells
    static let track = hex(0x2C3035)         // fader tracks, unlit knob rings
    static let ledOff = hex(0x24272B)        // unlit meter segments
    static let hairline = NSColor(white: 1, alpha: 0.06)

    // MARK: type colours
    static let text = hex(0xECEEF1)
    static let text2 = hex(0xA4AAB2)
    static let text3 = hex(0x6B717A)
    static let text4 = hex(0x43474E)         // greyed text

    // MARK: accent + semantic
    static let accent = hex(0x62AEFF)        // ctrl, link, routing, selection
    static let mute = hex(0xE5484D)
    static let solo = hex(0xF1B33A)
    static let green = hex(0x3DC97B)         // aux on output, locked
    static let red = hex(0xEF5248)           // unlocked
    static let meterLow = hex(0x33CF7D), meterMid = hex(0xEFC43B), meterHigh = hex(0xF2493F)
    static let capFace = hex(0xD9DCE0)       // fader cap
    static let capLine = hex(0x2A2D31)
    static let disabledAlpha: CGFloat = 0.32

    // MARK: metrics (8-pt grid)
    static let pad: CGFloat = 16             // outer padding of every tab
    static let stripWidth: CGFloat = 84
    static let stripGap: CGFloat = 6
    static let sectionGap: CGFloat = 24
    static let sectionLabelHeight: CGFloat = 24
    static let radius: CGFloat = 10
    static let headerHeight: CGFloat = 52
    static let sidebarWidth: CGFloat = 200

    // MARK: type
    // Font lookups are cached: creating a system font per draw call showed up in the 2026-10-21 CPU profile.
    private static var fontCache: [String: NSFont] = [:]
    private static func cached(_ key: String, _ make: () -> NSFont) -> NSFont {
        if let f = fontCache[key] { return f }
        let f = make(); fontCache[key] = f; return f
    }
    static func font(_ size: CGFloat, _ weight: NSFont.Weight = .regular) -> NSFont {
        return cached("s\(size)/\(weight.rawValue)") { NSFont.systemFont(ofSize: size, weight: weight) }
    }
    static func mono(_ size: CGFloat, _ weight: NSFont.Weight = .regular) -> NSFont {
        return cached("m\(size)/\(weight.rawValue)") { NSFont.monospacedDigitSystemFont(ofSize: size, weight: weight) }
    }

    /// Text colour that reads on a solid fill of `c` (dark text on light fills).
    static func textOn(_ c: NSColor) -> NSColor {
        guard let s = c.usingColorSpace(.sRGB) else { return .white }
        let l = 0.2126 * s.redComponent + 0.7152 * s.greenComponent + 0.0722 * s.blueComponent
        return l > 0.5 ? hex(0x0B0C0D) : .white
    }

    // MARK: drawing helpers

    /// Draws `body` as one group at `alpha` (greyed controls).
    static func dimmed(_ alpha: CGFloat, _ body: () -> Void) {
        guard alpha < 1, let ctx = NSGraphicsContext.current?.cgContext else { body(); return }
        ctx.saveGState()
        ctx.setAlpha(alpha)
        ctx.beginTransparencyLayer(auxiliaryInfo: nil)
        body()
        ctx.endTransparencyLayer()
        ctx.restoreGState()
    }

    static func fill(_ r: NSRect, radius: CGFloat, _ c: NSColor) {
        c.setFill()
        NSBezierPath(roundedRect: r, xRadius: radius, yRadius: radius).fill()
    }

    static func attributed(_ s: String, font: NSFont, color: NSColor, align: NSTextAlignment = .center, kern: CGFloat = 0) -> NSAttributedString {
        let p = NSMutableParagraphStyle()
        p.alignment = align
        p.lineBreakMode = .byClipping
        var a: [NSAttributedStringKey: Any] = [.font: font, .foregroundColor: color, .paragraphStyle: p]
        if kern != 0 { a[.kern] = kern }
        return NSAttributedString(string: s, attributes: a)
    }

    /// One line of text, vertically centred in `r` (works in flipped and unflipped views).
    static func draw(_ s: String, in r: NSRect, font: NSFont, color: NSColor, align: NSTextAlignment = .center, kern: CGFloat = 0) {
        let a = attributed(s, font: font, color: color, align: align, kern: kern)
        let h = ceil(a.size().height)
        a.draw(in: NSRect(x: r.minX, y: r.midY - h / 2, width: r.width, height: h))
    }

    /// Small upper-case caption (section and control labels).
    static func caption(_ s: String, in r: NSRect, color: NSColor = Theme.text3, align: NSTextAlignment = .center, size: CGFloat = 9) {
        draw(s.uppercased(), in: r, font: font(size, .semibold), color: color, align: align, kern: 0.8)
    }

    static func textWidth(_ s: String, font: NSFont, kern: CGFloat = 0) -> CGFloat {
        return ceil(attributed(s, font: font, color: .white, kern: kern).size().width)
    }

    /// Display strings of the property domains (A2-P13 §3): levels are 0.5 dB steps, the minimum shows as -∞ as the reference's aux
    /// knobs do; pan -255...255 shows as L127...C...R127; balance shows <C>, Lnn, Rnn.
    /// Console law: -129 and below read -inf (the official panel's stop). Monitor law (plan v7 AP-5.4): only off (-255) does; -144 = -72 dB.
    static func levelText(_ v: Int, law: FaderLaw = .console) -> String {
        if v <= (law == .monitor ? FaderLaw.off : MARange.levelMin) { return "-\u{221E}" }
        return String(format: "%.1f", Double(v) / 2)
    }
    static func panText(_ v: Int) -> String {
        if v == 0 { return "C" }
        return v < 0 ? "L\(-v / 2)" : "R\(v / 2)"
    }
    static func balanceText(_ b: Int) -> String {
        if b == 0 { return "<C>" }
        return b < 0 ? "L\(-b)" : "R\(b)"
    }
    static func rateText(_ hz: UInt32) -> String {
        let k = Double(hz) / 1000
        return k == k.rounded() ? String(format: "%.0f", k) : String(format: "%.1f", k)
    }
}

/// Top-left origin, so manual layouts read top-down.
/// Bench aid: OA_BENCH_IGNORE_OCCLUSION=1 keeps the meters drawing in covered windows (CPU measurements on a locked or sleeping screen).
let benchIgnoresOcclusion = ProcessInfo.processInfo.environment["OA_BENCH_IGNORE_OCCLUSION"] != nil

class FlippedView: NSView {
    override var isFlipped: Bool { return true }
}

/// Fills its bounds with one colour (window, header and sidebar backgrounds).
class FillView: FlippedView {
    var color: NSColor { didSet { needsDisplay = true } }
    init(_ c: NSColor) { color = c; super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }
    override var isOpaque: Bool { return color.alphaComponent >= 1 }
    override func draw(_ dirtyRect: NSRect) { color.setFill(); dirtyRect.fill() }
}

/// Places a window so its centre is `center`, kept inside the visible frame of the screen under that point.
func placeWindow(_ w: NSWindow, center: NSPoint) {
    let screen = NSScreen.screens.first(where: { $0.frame.contains(center) }) ?? NSScreen.main ?? NSScreen.screens.first
    var f = w.frame
    f.origin = NSPoint(x: (center.x - f.width / 2).rounded(), y: (center.y - f.height / 2).rounded())
    if let vf = screen?.visibleFrame {
        f.origin.x = max(vf.minX, min(f.origin.x, vf.maxX - f.width))
        f.origin.y = max(vf.minY, min(f.origin.y, vf.maxY - f.height))
    }
    w.setFrameOrigin(f.origin)
}

/// Centre of the main screen's visible area.
func screenCenter() -> NSPoint {
    let vf = (NSScreen.main ?? NSScreen.screens.first)?.visibleFrame ?? NSRect(x: 0, y: 0, width: 1440, height: 900)
    return NSPoint(x: vf.midX, y: vf.midY)
}

/// Window chrome shared by the panel windows: dark title bar that blends into the header band.
func styleWindow(_ w: NSWindow) {
    w.appearance = NSAppearance(named: .vibrantDark)
    w.titlebarAppearsTransparent = true
    w.backgroundColor = Theme.chrome
}
