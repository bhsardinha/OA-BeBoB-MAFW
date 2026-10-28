// Advanced windows: every vertical meter bar of a row drawn as one image (user, 2026-10-28: "the hardware sends one snapshot; handle it as
// one"). The driver's peak page is one snapshot of all channels (clientMemoryForType(1), uint32 peak[52]); the panel reads it once a tick
// and this view writes it into a small framebuffer — one texel row per LED segment, one texel column per device pixel across the row —
// that the compositor scales up with nearest-neighbour filtering. A static overlay drawn once adds the gaps between segments.
// Page flipping (user: "8-bit developer mindset"): two IOSurfaces shared with the GPU; a tick writes only the bars that changed into the
// back one and flips it to the front. No image object, no copy, no upload by the CPU (measured on the iMac, Dual DAW demo: a new CGImage
// per tick cost ~2 % of a core in the panel and ~5 % in WindowServer).
// Behaviour is the standard panel's MeterBar (user: "fast and clean"): the bar follows the peak, the hold segment waits 1.2 s then drops
// 2 dB a tick.
import AppKit
import IOSurface

final class MeterField: NSView {
    private struct Bar {
        let x0: Int, x1: Int
        var hold = -64, holdStamp: CFAbsoluteTime = 0
        var drawn = [(-1, -1), (-1, -1)]                 // (lit, hold segment) as written in each buffer
        init(x0: Int, x1: Int) { self.x0 = x0; self.x1 = x1 }
    }
    private var rects: [NSRect] = []
    private var bars: [Bar] = []
    private let lights = CALayer(), gaps = CALayer()
    private var buffers: [IOSurface] = []
    private var back = 0
    private var texW = 0, n = 0
    private var colors: (zone: [UInt32], off: UInt32) = ([], 0)
    private var builtFor: (NSSize, CGFloat, [NSRect])?
    /// The window's colour space: a bitmap in it goes to the screen as is (in device RGB, Core Animation colour-matched every new bitmap
    /// on the CPU, vImage tables, ~20 % of a core on the iMac for the single DAW view)
    private var space = CGColorSpaceCreateDeviceRGB()
    static let pitch = MeterBar.segment + MeterBar.segmentGap

    override init(frame: NSRect) {
        super.init(frame: frame)
        wantsLayer = true
        layerContentsRedrawPolicy = .never
        let still: [String: CAAction] = ["position": NSNull(), "bounds": NSNull(), "frame": NSNull(), "contents": NSNull()]
        for l in [lights, gaps] { l.actions = still; l.anchorPoint = .zero }
        lights.contentsGravity = kCAGravityResize
        lights.magnificationFilter = kCAFilterNearest
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override var isFlipped: Bool { return false }
    override var wantsUpdateLayer: Bool { return true }
    override func updateLayer() { if let l = layer, lights.superlayer !== l { [lights, gaps].forEach { l.addSublayer($0) } } }
    override func hitTest(_ point: NSPoint) -> NSView? { return nil }     // clicks go to the columns underneath (rename)

    /// The bars, in this view's coordinates, in the order of the values `update` will receive. All bars of a field share top and height.
    func setBars(_ r: [NSRect]) { rects = r.map { $0.integral }; needsLayout = true }

    override func setFrameSize(_ s: NSSize) { super.setFrameSize(s); needsLayout = true }
    override func viewDidChangeBackingProperties() { super.viewDidChangeBackingProperties(); builtFor = nil; needsLayout = true }
    override func layout() { super.layout(); updateLayer(); build() }

    /// Geometry, colours, the gap overlay and an all-unlit bitmap; once per size / scale / bar set.
    private func build() {
        let scale = window?.backingScaleFactor ?? 2
        if let b = builtFor, b.0 == bounds.size, b.1 == scale, b.2 == rects { return }
        builtFor = (bounds.size, scale, rects)
        space = window?.colorSpace?.cgColorSpace ?? CGColorSpaceCreateDeviceRGB()
        guard let first = rects.first, bounds.width > 0 else { bars = []; lights.contents = nil; gaps.contents = nil; return }
        n = max(1, Int((first.height + MeterBar.segmentGap) / MeterField.pitch))
        texW = Int((bounds.width * scale).rounded())
        let zones = MeterBar.zones(n)
        colors = ((0..<n).map { pixel($0 >= zones.red ? Theme.meterHigh : ($0 >= zones.yellow ? Theme.meterMid : Theme.meterLow)) }, pixel(Theme.ledOff))
        bars = rects.map { Bar(x0: max(0, Int(($0.minX * scale).rounded())), x1: min(texW, Int(($0.maxX * scale).rounded()))) }
        let props: [IOSurfacePropertyKey: Any] = [.width: texW, .height: n, .bytesPerElement: 4, .pixelFormat: 0x42475241 /* 'BGRA' */]
        buffers = (0..<2).compactMap { _ in IOSurface(properties: props) }
        guard buffers.count == 2 else { bars = []; return }
        for b in buffers { b.lock(options: [], seed: nil); memset(b.baseAddress, 0, b.allocationSize); b.unlock(options: [], seed: nil) }
        back = 0
        lights.frame = CGRect(x: 0, y: first.minY, width: bounds.width, height: CGFloat(n) * MeterField.pitch)
        gaps.frame = lights.frame                                           // the overlay covers only what the bitmap covers
        gaps.contentsScale = scale
        gaps.contents = gapImage(scale: scale)
        for k in 0..<2 { write(k) { p in for i in bars.indices { paint(i, lit: 0, hold: 0, into: p, k) } } }
        lights.contents = buffers[1]
    }

    /// The well between the segments, over each bar, in the overlay's own coordinates (the segments stay transparent: the bitmap shows
    /// through). Drawn once per geometry.
    private func gapImage(scale: CGFloat) -> CGImage? {
        let size = lights.frame.size
        let w = Int((size.width * scale).rounded()), h = Int((size.height * scale).rounded())
        guard w > 0, h > 0, let ctx = CGContext(data: nil, width: w, height: h, bitsPerComponent: 8, bytesPerRow: 0, space: space,
                                                bitmapInfo: MeterField.format.rawValue) else { return nil }
        ctx.scaleBy(x: scale, y: scale)
        ctx.setFillColor(Theme.well.cgColor)
        for r in rects {
            ctx.fill(CGRect(x: r.minX, y: 0, width: r.width, height: size.height))
            for i in 0..<n { ctx.clear(CGRect(x: r.minX, y: CGFloat(i) * MeterField.pitch, width: r.width, height: MeterBar.segment)) }
        }
        return ctx.makeImage()
    }

    private func litCount(_ v: Int) -> Int { return v <= -64 ? 0 : Int((MeterBar.fraction(v) * CGFloat(n)).rounded(.up)) }

    private var want: [(Int, Int)] = []
    /// One value per bar (dB, -64 = silence), in `setBars` order. Nothing happens unless a bar's lit or hold segment moved; then the back
    /// buffer gets the bars it is behind on (this tick's and last tick's changes) and becomes the front.
    func update(_ values: [Int]) {
        guard buffers.count == 2, !bars.isEmpty, values.count == bars.count else { return }
        let now = CFAbsoluteTimeGetCurrent(), front = 1 - back
        if want.count != bars.count { want = [(Int, Int)](repeating: (0, 0), count: bars.count) }
        var changed = false
        for i in bars.indices {
            let v = values[i]
            if v >= bars[i].hold { bars[i].hold = v; bars[i].holdStamp = now }
            else if now - bars[i].holdStamp > 1.2 { bars[i].hold = max(v, bars[i].hold - 2) }
            want[i] = (litCount(v), litCount(bars[i].hold))
            if want[i] != bars[i].drawn[front] { changed = true }
        }
        guard changed else { return }
        let k = back
        write(k) { p in for i in bars.indices where want[i] != bars[i].drawn[k] { paint(i, lit: want[i].0, hold: want[i].1, into: p, k) } }
        lights.contents = buffers[k]
        back = 1 - k
    }

    /// Locks buffer k for the CPU, hands its pixels to `body`, unlocks.
    private func write(_ k: Int, _ body: (UnsafeMutablePointer<UInt32>) -> Void) {
        let s = buffers[k]
        s.lock(options: [], seed: nil)
        body(s.baseAddress.assumingMemoryBound(to: UInt32.self))
        s.unlock(options: [], seed: nil)
    }

    private func paint(_ i: Int, lit: Int, hold: Int, into p: UnsafeMutablePointer<UInt32>, _ k: Int) {
        bars[i].drawn[k] = (lit, hold)
        let b = bars[i], stride = buffers[k].bytesPerRow / 4
        guard b.x1 > b.x0 else { return }
        for row in 0..<n {
            let c = row < lit || (row == hold - 1 && hold > lit) ? colors.zone[row] : colors.off
            let line = p + (n - 1 - row) * stride                                // buffer row 0 is the top segment
            for x in b.x0..<b.x1 { line[x] = c }
        }
    }

    private static let format = CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue)
    /// A colour as one texel of the bitmap, drawn by Core Graphics in the bitmap's colour space (it matches the other meters on screen).
    private func pixel(_ c: NSColor) -> UInt32 {
        var v: UInt32 = 0
        withUnsafeMutablePointer(to: &v) { p in
            guard let ctx = CGContext(data: p, width: 1, height: 1, bitsPerComponent: 8, bytesPerRow: 4, space: space,
                                      bitmapInfo: MeterField.format.rawValue) else { return }
            ctx.setFillColor(c.cgColor); ctx.fill(CGRect(x: 0, y: 0, width: 1, height: 1))
        }
        return v
    }
}
