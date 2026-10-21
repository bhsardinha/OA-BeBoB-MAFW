// AP-9.1 -- the firmware check window (part of the panel since 2026-10-21, user: formerly the separate OpenAudio Firmware Tool app): one card per attached unit (product, GUID, state, firmware / bootloader dates, hardware, the official
// 1.11.2 build of that model and the verdict). Read-only: the window never writes to a unit. Look and widgets shared with the panel
// (control-panel/Sources/Theme.swift, Widgets.swift); the info block decoder is the panel's FirmwareInfo.swift.
import AppKit

final class UnitCard: FlippedView {
    static let height: CGFloat = 200        // rows end at 159, the message line at 163-178, then 22 pt to the edge
    private let unit: UnitInfo
    init(_ u: UnitInfo) { unit = u; super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }

    static func verdict(_ s: FirmwareStatus) -> (String, NSColor, String) {
        switch s {
        case .current: return ("Current", Theme.green, "This unit runs the official firmware of its model.")
        case .older: return ("Older", Theme.solo, "Older than the official 1.11.2 build. The official driver updates it; this tool only checks.")
        case .newer: return ("Newer", Theme.accent, "Newer than the official 1.11.2 build. The official driver would flash it back to 1.11.2.")
        case .unknownModel: return ("Unknown model", Theme.text3, "Not one of the models the official driver supports.")
        case .noInfo: return ("No information", Theme.text3, "The unit's information block could not be read.")
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        Theme.fill(bounds, radius: 12, Theme.surface)
        let w = bounds.width, pad: CGFloat = 20
        let (label, color, note) = UnitCard.verdict(unit.status)
        Theme.draw(unit.productName, in: NSRect(x: pad, y: 16, width: w - 2 * pad - 140, height: 20), font: Theme.font(15, .semibold), color: Theme.text, align: .left)
        Theme.draw(unit.guid, in: NSRect(x: pad, y: 38, width: 300, height: 14), font: Theme.mono(10), color: Theme.text3, align: .left)
        // verdict pill
        let pw: CGFloat = 120, pill = NSRect(x: w - pad - pw, y: 16, width: pw, height: 24)
        let p = NSBezierPath(roundedRect: pill, xRadius: 12, yRadius: 12)
        color.withAlphaComponent(0.16).setFill(); p.fill()
        Theme.draw(label, in: pill, font: Theme.font(12, .semibold), color: color)          // Theme.draw centres vertically
        // rows
        let i = unit.info
        let rows: [(String, String)] = [
            ("State", unit.bootloader ? "Bootloader (no firmware running)" : (unit.driverAttached ? "Running, driver attached" : "Running")),
            ("Firmware", i.map { FirmwareInfo.display(date: $0.firmwareDate, time: $0.firmwareTime) } ?? "-"),
            ("Official \(FirmwareCatalog.release)", unit.official.map { "\(FirmwareCatalog.display($0.build))   (\($0.file))" } ?? "-"),
            ("Bootloader", i.map { FirmwareInfo.display(date: $0.bootloaderDate, time: $0.bootloaderTime) } ?? "-"),
            ("Hardware", i.map { "model \($0.hardwareModel) version \($0.hardwareRevision)" } ?? "-"),
        ]
        var y: CGFloat = 64
        for (k, v) in rows {
            Theme.draw(k, in: NSRect(x: pad, y: y, width: 230, height: 15), font: Theme.font(12), color: Theme.text3, align: .left)
            Theme.draw(v, in: NSRect(x: pad + 236, y: y, width: w - 2 * pad - 236, height: 15), font: Theme.font(12), color: Theme.text, align: .left)
            y += 19
        }
        Theme.draw(note, in: NSRect(x: pad, y: y + 4, width: w - 2 * pad, height: 15), font: Theme.font(11), color: color, align: .left)
    }
}

final class CheckWindow: NSObject, NSWindowDelegate {
    let window: NSWindow
    private let demo: Bool
    private var units: [UnitInfo] = []
    private let refresh = FlatButton("Refresh")

    init(demo: Bool) {
        self.demo = demo
        window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 640, height: 300), styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
        super.init()
        window.title = "Firmware Check"
        window.isReleasedWhenClosed = false
        window.delegate = self
        styleWindow(window)
        refresh.onAction = { [weak self] in self?.reload() }
        reload()
    }


    func reload() {
        units = demo ? UnitScanner.demo() : UnitScanner.scan()
        let width: CGFloat = 640, pad: CGFloat = 20, gap: CGFloat = 12
        let root = FillView(Theme.window)
        let title = makeLabel("Firmware check", size: 18, weight: .semibold, color: Theme.text)
        title.frame = NSRect(x: pad, y: 18, width: 400, height: 24)
        let sub = makeLabel("Each attached interface compared with the official \(FirmwareCatalog.release) firmware of its model. Read-only: nothing is written to the interfaces.", size: 12, color: Theme.text2)
        sub.maximumNumberOfLines = 2; sub.lineBreakMode = .byWordWrapping
        sub.frame = NSRect(x: pad, y: 46, width: width - 2 * pad, height: 34)
        refresh.frame = NSRect(x: width - pad - 96, y: 18, width: 96, height: 26)
        [title, sub, refresh].forEach { root.addSubview($0) }
        var y: CGFloat = 92
        if units.isEmpty {
            let none = makeLabel("No M-Audio FireWire interface found. Turn the interface on and make sure the OpenAudio BeBoB MAFW driver is installed.", size: 12, color: Theme.text3)
            none.maximumNumberOfLines = 2; none.lineBreakMode = .byWordWrapping
            none.frame = NSRect(x: pad, y: y, width: width - 2 * pad, height: 34); root.addSubview(none); y += 46
        }
        for u in units {
            let c = UnitCard(u); c.frame = NSRect(x: pad, y: y, width: width - 2 * pad, height: UnitCard.height)
            root.addSubview(c); y += UnitCard.height + gap
        }
        root.frame = NSRect(x: 0, y: 0, width: width, height: y + pad - gap)
        let center = NSPoint(x: window.frame.midX, y: window.frame.midY)
        let size = root.frame.size                       // read before assigning: the window resizes its content view
        window.contentView = root
        window.setContentSize(size)
        if window.isVisible { placeWindow(window, center: center) }
    }

    /// Text listing for SSH / logs (panel `--firmware-list`).
    static func listing(_ units: [UnitInfo]) -> String {
        if units.isEmpty { return "no units" }
        return units.map { u -> String in
            let (v, _, _) = UnitCard.verdict(u.status)
            let fw = u.info.map { "\($0.firmwareDate) \($0.firmwareTime)" } ?? "-"
            return "\(u.productName)  \(u.guid)  \(u.bootloader ? "bootloader" : "running")  firmware \(fw)  official \(u.official?.build ?? "-")  -> \(v)"
        }.joined(separator: "\n")
    }

    func snapshot(to path: String) {
        guard let v = window.contentView, let rep = v.bitmapImageRepForCachingDisplay(in: v.bounds) else { return }
        v.cacheDisplay(in: v.bounds, to: rep)
        if let d = rep.representation(using: .png, properties: [:]) { try? d.write(to: URL(fileURLWithPath: path)) }
    }
}
