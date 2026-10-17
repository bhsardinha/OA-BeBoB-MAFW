// PN.4 -- meter containers: the meter-only bus strip that sits in a strip row (digital return / output bus), the bus meter panels of the
// mixer and output tabs (stereo pairs as two slim horizontal LED bars), and the two mini-meter windows. Data comes from
// PanelContext.meterDB (shared peak page, A2-P9; slot map A2-P13 §6). Values are dB, floor -64.
import AppKit

/// Meter-only strip (digital return bus on the mixer, digital output bus on the output tab): same height and title row as the strips of
/// its tab, meters over the fader area, channel numbers underneath, and an optional button in the bottom row (S/PDIF out main/aux).
final class BusStripView: FlippedView {
    let meters: [MeterBar]
    let title: String
    private let numbers: [String]
    private let geometry: StripView.Geometry
    private let scale = MeterScale()
    private var greyedFrom: Int

    init(title: String, channels: Int, kind: StripView.Kind, numbers: [String]? = nil) {
        meters = (0..<channels).map { _ in MeterBar() }
        self.title = title
        self.numbers = numbers ?? (1...channels).map { String($0) }
        geometry = StripView.Geometry(kind)
        greyedFrom = channels
        let pitch: CGFloat = 10
        let width = max(Theme.stripWidth, 8 + 16 + 10 + CGFloat(channels) * pitch + 8)
        super.init(frame: NSRect(x: 0, y: 0, width: width, height: geometry.height))
        let top = geometry.faderY + Fader.trackPad, h = geometry.faderHeight - 2 * Fader.trackPad
        scale.frame = NSRect(x: 8, y: top, width: 16, height: h)
        addSubview(scale)
        let span = CGFloat(channels - 1) * pitch + StripView.Geometry.meterWidth
        let x0 = (34 + (width - 8 - 34 - span) / 2).rounded()
        for (i, m) in meters.enumerated() {
            m.frame = NSRect(x: x0 + CGFloat(i) * pitch, y: top, width: StripView.Geometry.meterWidth, height: h)
            addSubview(m)
        }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func meterCenter(_ i: Int) -> CGFloat { return meters[i].frame.midX }

    /// A button in the strip's bottom row (the S/PDIF out main/aux toggle).
    func addBottomButton(_ b: PillButton) {
        b.frame = NSRect(x: 6, y: geometry.bottomY, width: bounds.width - 12, height: 20)
        addSubview(b)
    }

    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: Theme.radius, Theme.surface)
        Theme.draw(title, in: NSRect(x: 2, y: geometry.titleY, width: bounds.width - 4, height: 16), font: Theme.font(11, .semibold), color: Theme.text)
        for (i, n) in numbers.enumerated() where i < meters.count {
            Theme.draw(n, in: NSRect(x: meterCenter(i) - 8, y: geometry.readoutY, width: 16, height: 11), font: Theme.mono(9),
                       color: i < greyedFrom ? Theme.text3 : Theme.text4)
        }
    }

    /// Channels at index >= first are greyed (ADAT 5-8 at 88.2/96 kHz).
    func setGreyed(from first: Int) {
        if first != greyedFrom { greyedFrom = first; needsDisplay = true }
        for (i, m) in meters.enumerated() { m.isEnabled = i < first }
    }
}

/// Bus meter panel: titled groups of stereo meters. `.row` puts the pairs side by side under the strips (mixer); `.column` stacks them as
/// rows beside the strips (output tab).
final class MeterBridge: FlippedView {
    enum Axis { case row, column }
    struct Group { let title: String; let labels: [String] }

    /// One stereo pair: label, two slim horizontal bars (L above R) and the held peak of the louder side.
    final class Pair: FlippedView {
        let label: String
        let bars = [MeterBar(), MeterBar()]
        var stacked = true
        var isEnabled = true { didSet { bars.forEach { $0.isEnabled = isEnabled }; needsDisplay = true } }
        private var shownPeak = Int.min
        init(_ label: String) {
            self.label = label
            super.init(frame: .zero)
            for b in bars { b.horizontal = true; addSubview(b) }
        }
        required init?(coder: NSCoder) { fatalError("unused") }
        static let labelWidth: CGFloat = 74, readoutWidth: CGFloat = 28
        override func setFrameSize(_ newSize: NSSize) {
            super.setFrameSize(newSize)
            if stacked {
                bars[0].frame = NSRect(x: 0, y: 18, width: newSize.width, height: 3)
                bars[1].frame = NSRect(x: 0, y: 24, width: newSize.width, height: 3)
            } else {
                let w = newSize.width - Pair.labelWidth - Pair.readoutWidth - 8
                bars[0].frame = NSRect(x: Pair.labelWidth, y: newSize.height / 2 - 4, width: w, height: 3)
                bars[1].frame = NSRect(x: Pair.labelWidth, y: newSize.height / 2 + 1, width: w, height: 3)
            }
        }
        private var readoutRect: NSRect {
            return stacked ? NSRect(x: bounds.width - Pair.readoutWidth, y: 0, width: Pair.readoutWidth, height: 14)
                           : NSRect(x: bounds.width - Pair.readoutWidth, y: 0, width: Pair.readoutWidth, height: bounds.height)
        }
        override func draw(_ dirtyRect: NSRect) {
            let lc = isEnabled ? Theme.text2 : Theme.text4
            if stacked {
                Theme.draw(label, in: NSRect(x: 0, y: 0, width: bounds.width - Pair.readoutWidth, height: 14), font: Theme.font(11, .medium), color: lc, align: .left)
            } else {
                Theme.draw(label, in: NSRect(x: 0, y: 0, width: Pair.labelWidth - 6, height: bounds.height), font: Theme.font(11, .medium), color: lc, align: .left)
            }
            let p = max(bars[0].hold, bars[1].hold)
            Theme.draw(isEnabled && p > -64 ? "\(p)" : "-", in: readoutRect, font: Theme.mono(9), color: isEnabled ? Theme.text3 : Theme.text4, align: .right)
        }
        func set(_ l: Int, _ r: Int) {
            bars[0].db = l; bars[1].db = r
            let p = max(bars[0].hold, bars[1].hold)
            if p != shownPeak { shownPeak = p; setNeedsDisplay(readoutRect) }
        }
    }

    let axis: Axis
    private let groups: [Group]
    private(set) var pairs: [Pair] = []

    init(axis: Axis, groups: [Group]) {
        self.axis = axis
        self.groups = groups
        super.init(frame: .zero)
        for g in groups { for l in g.labels { let p = Pair(l); p.stacked = (axis == .row); pairs.append(p); addSubview(p) } }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    static let rowHeight: CGFloat = 74           // height of a .row panel
    /// Height a .column panel needs for its rows.
    var columnContentHeight: CGFloat { return 14 + CGFloat(groups.count) * (24 + 12) + CGFloat(pairs.count) * 24 + 4 }
    private var titleRects: [(String, NSRect)] = []

    /// Lays the pairs out for a panel of the given size.
    func layout(width: CGFloat, height: CGFloat) {
        setFrameSize(NSSize(width: width, height: height))
        titleRects.removeAll()
        let pad: CGFloat = 18
        var k = 0
        switch axis {
        case .row:
            let n = CGFloat(pairs.count), gaps = CGFloat(max(0, pairs.count - 1))
            let groupExtra: CGFloat = 16 * CGFloat(max(0, groups.count - 1))
            let cell = ((width - 2 * pad - gaps * 20 - groupExtra) / max(1, n)).rounded(.down)
            var x = pad
            for g in groups {
                let gx = x
                for _ in g.labels { pairs[k].frame = NSRect(x: x, y: 34, width: cell, height: 28); x += cell + 20; k += 1 }
                titleRects.append((g.title, NSRect(x: gx, y: 13, width: x - gx - 20, height: 14)))
                x += 16
            }
        case .column:
            var y: CGFloat = 14
            for g in groups {
                titleRects.append((g.title, NSRect(x: pad, y: y, width: width - 2 * pad, height: 14)))
                y += 24
                for _ in g.labels { pairs[k].frame = NSRect(x: pad, y: y, width: width - 2 * pad, height: 16); y += 24; k += 1 }
                y += 12
            }
        }
        needsDisplay = true
    }

    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: Theme.radius, Theme.surface)
        for (t, r) in titleRects { Theme.caption(t, in: r, align: .left) }
    }

    func setValues(_ v: [[Int]]) {
        for (i, p) in pairs.enumerated() where i < v.count && v[i].count >= 2 { p.set(v[i][0], v[i][1]) }
    }
    func setEnabled(_ flags: [Bool]) { for (i, p) in pairs.enumerated() where i < flags.count { p.isEnabled = flags[i] } }
}

/// The two mini-meter windows (wireframe PN1): wide (vertical bars, labels underneath) and narrow (horizontal bars, one row per channel).
/// Channels per panel follow the layout, as the reference's four layout variants do (PPob 10001..10008 "Mini Meters A/S I/O Horz/Vert"):
/// input 1-8 then S/PDIF (spd, or sL/sR in the narrow view) or ADAT; sw rtn 1-4 then S/PDIF or ADAT; output 1-4 then S/PDIF or ADAT.
/// The ADAT variants' channel lists are INFERRED from the layout names; slot map A2-P13 §6 (S/PDIF in = input ids 9, 10; ADAT in = 11...18;
/// S/PDIF out = output ids 5, 6; ADAT out = 7...14; digital sw rtn = SW-return channels 5...: inferred like the strip meters).
/// Presentation: channels are shown in captioned sub-groups ("Analog 1-8", "ADAT 1-8", "S/PDIF L R") with plain numbers per bar.
final class MiniMetersWindow: NSObject, NSWindowDelegate {
    struct Item { let label: String; let slots: [Int]; let group: String }
    let window: NSWindow
    private let ctx: PanelContext
    private let narrow: Bool
    private let onSwitch: (Int) -> Void
    private var bars: [(MeterBar, [Int])] = []
    private var builtFor: ChannelLayout?
    private let modeSwitch = SegmentedBar(items: [.icon(.full), .icon(.wide), .icon(.narrow)], segmentWidth: 30,
                                          tooltips: ["Full panel", "Wide meters", "Narrow meters"])
    private var nameView: HeaderText?
    /// Interface name in the window's header (with the GUID tail when several interfaces are connected).
    var deviceTitle = "" { didSet { nameView?.text = deviceTitle; window.title = "\(deviceTitle) Meters" } }

    init(context: PanelContext, narrow: Bool, onSwitch: @escaping (Int) -> Void) {
        ctx = context; self.narrow = narrow; self.onSwitch = onSwitch
        window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: narrow ? 220 : 560, height: narrow ? 600 : 280),
                          styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
        super.init()
        window.title = "FireWire 1814 Meters"
        window.isReleasedWhenClosed = false
        window.delegate = self
        styleWindow(window)
        modeSwitch.selected = narrow ? 2 : 1
        modeSwitch.onSelect = { [weak self] i in
            guard let me = self else { return }
            me.onSwitch(i)
            me.modeSwitch.selected = me.narrow ? 2 : 1          // this window's own mode next time it shows
        }
        rebuild()
    }

    /// Closing a meter window returns to the full panel instead of leaving no window on screen.
    func windowShouldClose(_ sender: NSWindow) -> Bool { onSwitch(0); return false }

    private func items(_ layout: ChannelLayout) -> [(String, [Item])] {
        func numbered(_ n: Int, _ group: String, _ slot: (Int) -> Int) -> [Item] { return (1...n).map { Item(label: "\($0)", slots: [slot($0)], group: group) } }
        // 176.4 / 192 kHz: no DSP mixer (user, hardware, 2026-10-17): no input metering; the only live meters are the software peaks of the four
        // outputs Core Audio sends (kext peak map, SW-return slots 1...4).
        if layout.rateClass == .quad { return [("Output", numbered(4, "Analog", PeakSlot.swReturn))] }
        var input = numbered(8, "Analog", PeakSlot.input)
        var sw = numbered(4, "", PeakSlot.swReturn)
        var out = numbered(4, "Analog", PeakSlot.output)
        if layout.digitalAvailable {
            if layout.adatIn { input += (1...8).map { Item(label: "\($0)", slots: [PeakSlot.input(10 + $0)], group: "ADAT") } }
            else if narrow { input += [Item(label: "L", slots: [PeakSlot.input(9)], group: "S/PDIF"), Item(label: "R", slots: [PeakSlot.input(10)], group: "S/PDIF")] }
            else { input.append(Item(label: "L+R", slots: [PeakSlot.input(9), PeakSlot.input(10)], group: "S/PDIF")) }
            if layout.adatOut {
                sw += (1...8).map { Item(label: "\($0)", slots: [PeakSlot.swReturnADAT($0)], group: "ADAT") }
                out += (1...8).map { Item(label: "\($0)", slots: [PeakSlot.output(6 + $0)], group: "ADAT") }
            } else if narrow {
                sw += [Item(label: "L", slots: [PeakSlot.swReturn(5)], group: "S/PDIF"), Item(label: "R", slots: [PeakSlot.swReturn(6)], group: "S/PDIF")]
                out += [Item(label: "L", slots: [PeakSlot.output(5)], group: "S/PDIF"), Item(label: "R", slots: [PeakSlot.output(6)], group: "S/PDIF")]
            } else {
                sw.append(Item(label: "L+R", slots: [PeakSlot.swReturn(5), PeakSlot.swReturn(6)], group: "S/PDIF"))
                out.append(Item(label: "L+R", slots: [PeakSlot.output(5), PeakSlot.output(6)], group: "S/PDIF"))
            }
        } else {
            input = numbered(2, "Analog", PeakSlot.input)       // 192 kHz: 2 in / 4 out analog
        }
        return [("Input", input), ("Software Return", sw), ("Output", out)]
    }

    /// Splits a panel's items into consecutive runs of one group.
    private func runs(_ list: [Item]) -> [(String, [Item])] {
        var r: [(String, [Item])] = []
        for it in list {
            if let last = r.last, last.0 == it.group { r[r.count - 1].1.append(it) } else { r.append((it.group, [it])) }
        }
        return r
    }

    /// A text-drawing card used for the meter panels (captions and numbers are drawn, bars are subviews).
    private final class Panel: FlippedView {
        var texts: [(String, NSRect, Bool, Bool)] = []    // (text, rect, isCaption, greyed)
        var title = ""
        override func draw(_ dirtyRect: NSRect) {
            Theme.fill(bounds, radius: 12, Theme.surface)
            Theme.caption(title, in: NSRect(x: 16, y: 12, width: bounds.width - 32, height: 14), color: Theme.text2, align: .left)
            for (t, r, cap, grey) in texts {
                if cap { Theme.caption(t, in: r, align: .left, size: 8) } else { Theme.draw(t, in: r, font: Theme.mono(9), color: grey ? Theme.text4 : Theme.text3) }
            }
        }
    }

    func rebuild() {
        let layout = ctx.layout
        builtFor = layout
        bars.removeAll()
        let root = FillView(Theme.chrome)
        let pad: CGFloat = 16
        var panels: [Panel] = []
        // ADAT 5-8 are greyed at 88.2/96 kHz, as on the strips and bus meters (ChannelLayout rules; input follows optical in, the others optical out).
        let inHalf = layout.inputBusMeters.contains { !$0.enabled }, outHalf = layout.adatBusHalfGreyed
        func greyed(_ panel: Int, _ it: Item) -> Bool { return it.group == "ADAT" && (Int(it.label) ?? 0) > 4 && (panel == 0 ? inHalf : outHalf) }
        if narrow {
            let width: CGFloat = 228
            var y: CGFloat = 84
            for (pi, (title, list)) in items(layout).enumerated() {
                let p = Panel(); p.title = title
                var py: CGFloat = 38
                let barX: CGFloat = 40, barW = width - 2 * pad - barX - 16
                for (group, its) in runs(list) {
                    if !group.isEmpty { p.texts.append((group, NSRect(x: 16, y: py, width: 140, height: 12), true, false)); py += 16 }
                    for it in its {
                        let b = MeterBar(); b.horizontal = true
                        b.frame = NSRect(x: barX, y: py + 3, width: barW, height: 4)
                        b.isEnabled = !greyed(pi, it)
                        p.addSubview(b); bars.append((b, it.slots))
                        p.texts.append((it.label, NSRect(x: 14, y: py, width: 20, height: 10), false, !b.isEnabled))
                        py += 11
                    }
                    py += 6
                }
                // dB marks under the bars
                let len = barW - MeterBar.clipLength - MeterBar.clipGap
                for d in MeterBar.scaleMarks {
                    let x = barX + MeterBar.fraction(d) * len
                    p.texts.append((d == 0 ? "0" : "\(-d)", NSRect(x: x - 10, y: py, width: 20, height: 10), false, false))
                }
                py += 22
                p.frame = NSRect(x: pad, y: y, width: width - 2 * pad, height: py)
                panels.append(p)
                y += py + 10
            }
            root.frame = NSRect(x: 0, y: 0, width: width, height: y + 6)
        } else {
            var x = pad
            let barTop: CGFloat = 58, barH: CGFloat = 140, pitch: CGFloat = 11
            for (pi, (title, list)) in items(layout).enumerated() {
                let p = Panel(); p.title = title
                let sc = MeterScale(); sc.frame = NSRect(x: 12, y: barTop, width: 16, height: barH); p.addSubview(sc)
                var px: CGFloat = 38
                for (group, its) in runs(list) {
                    let gx = px
                    for it in its {
                        let b = MeterBar()
                        b.frame = NSRect(x: px, y: barTop, width: 4, height: barH)
                        b.isEnabled = !greyed(pi, it)
                        p.addSubview(b); bars.append((b, it.slots))
                        p.texts.append((it.label, NSRect(x: px - 10, y: barTop + barH + 6, width: 24, height: 11), false, !b.isEnabled))
                        px += pitch
                    }
                    let capW = max(px - gx, Theme.textWidth(group.uppercased(), font: Theme.font(8, .semibold), kern: 0.8) + 4)
                    p.texts.append((group, NSRect(x: gx - 2, y: barTop - 18, width: capW + 8, height: 12), true, false))
                    px = gx + capW + 12
                }
                let w = max(px + 6, Theme.textWidth(title.uppercased(), font: Theme.font(9, .semibold), kern: 0.8) + 36)
                p.frame = NSRect(x: x, y: 52, width: w, height: barTop + barH + 30)
                panels.append(p)
                x += w + 10
            }
            root.frame = NSRect(x: 0, y: 0, width: x - 10 + pad, height: 52 + barTop + barH + 30 + pad)
        }
        panels.forEach { root.addSubview($0) }
        let name = HeaderText(deviceTitle.isEmpty ? (ctx.device?.info.name ?? "FireWire 1814") : deviceTitle)
        nameView = name
        if narrow {                                       // name on its own row, switch centred under it
            name.align = .center
            name.frame = NSRect(x: pad, y: 8, width: root.frame.width - 2 * pad, height: 26)
            modeSwitch.setFrameOrigin(NSPoint(x: ((root.frame.width - modeSwitch.frame.width) / 2).rounded(), y: 40))
        } else {
            name.frame = NSRect(x: pad, y: 10, width: root.frame.width - 2 * pad - modeSwitch.frame.width - 8, height: 30)
            modeSwitch.setFrameOrigin(NSPoint(x: root.frame.width - pad - modeSwitch.frame.width, y: 10))
        }
        root.addSubview(name)
        root.addSubview(modeSwitch)

        let wasVisible = window.isVisible
        let center = NSPoint(x: window.frame.midX, y: window.frame.midY)
        let size = root.frame.size                       // read before assigning: the window resizes its content view
        window.contentView = root
        window.setContentSize(size)
        if wasVisible { placeWindow(window, center: center) }
        update()
    }

    func update() {
        if builtFor != ctx.layout { rebuild(); return }
        for (b, slots) in bars { b.db = slots.map { ctx.db($0) }.max() ?? -64 }
    }
}

/// Device name in a header band.
final class HeaderText: FlippedView {
    var text: String { didSet { needsDisplay = true } }
    var align: NSTextAlignment = .left
    init(_ text: String) { self.text = text; super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func draw(_ dirtyRect: NSRect) {
        let big = Theme.font(13, .semibold)
        let f = Theme.textWidth(text, font: big) <= bounds.width ? big : Theme.font(11, .semibold)     // long names in the narrow window
        Theme.draw(text, in: bounds, font: f, color: Theme.text, align: align)
    }
}
