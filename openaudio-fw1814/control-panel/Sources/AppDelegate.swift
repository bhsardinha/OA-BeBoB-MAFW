// PN.1/PN.6 -- application entry: one panel controller per FireWire 1814 found (hot-plug via IOServiceAddMatchingNotification, A2-P1), each
// with its own device model and windows, as the reference keeps one CXDevice and its windows per device (CDeviceList, CDeviceWindowManager).
// Like the reference menu bar (panel rsrc MENU 132 "Device", built by CControlPanelApp::BuildDeviceMenu; MENU 133 "View"), a Device menu
// lists the connected interfaces and a View menu picks the tab or meter view of the front one; the header of each panel offers the same
// device switch. Switching shows the chosen interface in place of the current one (same spot, same view). A waiting window shows while no
// interface is present. The reference panel is a Carbon app plus prefpane; Carbon is 32-bit on 10.13, so this is a plain AppKit app
// (deviation forced by 10.13.6, Proposal.md). The prefpane and the mandatory status UI are dropped.
import AppKit

final class AppDelegate: NSObject, NSApplicationDelegate, NSMenuDelegate {
    private let list = FWDeviceList()
    private var controllers: [UInt64: PanelWindowController] = [:]
    private var order: [UInt64] = []                       // arrival order, for menus and the default panel
    private var demo: [DemoBackend] = []
    private var waiting: NSWindow?
    private let deviceMenu = NSMenu(title: "Device")

    private var ordered: [PanelWindowController] { return order.compactMap { controllers[$0] } }
    /// The interface whose window is in front (key or main), else the one with a visible window.
    private var front: PanelWindowController? {
        let w = NSApp.keyWindow ?? NSApp.mainWindow
        return ordered.first(where: { $0.owns(w) }) ?? ordered.first(where: { $0.visibleWindow != nil })
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        buildMenu()
        let args = CommandLine.arguments
        if args.contains("--demo") {
            // Debug aid: `--devices N` simulates N interfaces (the second and later ones at 44.1 kHz S/PDIF, so they look different).
            let rate = args.index(of: "--rate").flatMap { i in i + 1 < args.count ? UInt32(args[i + 1]) : nil } ?? 48000
            let n = max(1, min(4, args.index(of: "--devices").flatMap { i in i + 1 < args.count ? Int(args[i + 1]) : nil } ?? 1))
            let guids = ["0xD6C040074B9FB", "0xD6C04007512C4", "0xD6C0400753A10", "0xD6C04007560D7"]
            demo = (0..<n).map { k in
                k == 0 ? DemoBackend(id: 1, rate: rate, adat: !args.contains("--spdif"), guid: guids[0])
                       : DemoBackend(id: UInt64(k + 1), rate: 44100, adat: false, guid: guids[k])
            }
            sync()
            if let i = args.index(of: "--snapshot"), i + 1 < args.count {
                let dir = args[i + 1]
                let knob = args.index(of: "--knob").flatMap { k in k + 1 < args.count ? Int(args[k + 1]) : nil } ?? 0
                let abArg = args.index(of: "--ab").flatMap { k in k + 1 < args.count ? args[k + 1] : nil }
                let abList = (abArg ?? "").split(separator: ",").compactMap { UInt32($0).flatMap { MAHeadphoneSource(rawValue: $0) } }
                let ab: (MAHeadphoneSource, MAHeadphoneSource)? = abList.count == 2 ? (abList[0], abList[1]) : nil
                let prefix = "\(rate)-\(args.contains("--spdif") ? "spdif" : "adat")" + (knob != 0 ? "-knob\(knob)" : "") + (ab != nil ? "-ab" : "")
                DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in
                    self?.snapshot(dir: dir, prefix: prefix, knob: knob, ab: ab)
                    NSApp.terminate(nil)
                }
            }
            return
        }
        list.onChange = { [weak self] in self?.sync() }
        list.start()
        // `--snapshot DIR` also works against a real interface (bench aid for the integrated run): PNGs of every view, then quit.
        if let i = args.index(of: "--snapshot"), i + 1 < args.count {
            let dir = args[i + 1]
            DispatchQueue.main.asyncAfter(deadline: .now() + 4.0) { [weak self] in
                self?.snapshot(dir: dir, prefix: "real", knob: 0, ab: nil)
                NSApp.terminate(nil)
            }
        }
        sync()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool { return false }
    func applicationWillTerminate(_ notification: Notification) {
        list.stop()
    }

    /// Reopening from the Dock brings the panel back.
    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        if !flag { (front ?? ordered.first)?.show() }
        return true
    }

    // MARK: device list

    private func sync() {
        let backends: [PanelBackend] = demo.isEmpty ? list.devices.map { $0 as PanelBackend } : demo.map { $0 as PanelBackend }
        let present = Set(backends.map { $0.entryID })
        var center: NSPoint?
        var changed = false
        for (id, c) in controllers where !present.contains(id) {
            if let w = c.visibleWindow { center = NSPoint(x: w.frame.midX, y: w.frame.midY) }
            c.close(); controllers[id] = nil
            order = order.filter { $0 != id }
            changed = true
        }
        for d in backends where controllers[d.entryID] == nil {
            let c = PanelWindowController(device: d)
            c.onSelectDevice = { [weak self] from, id in self?.switchDevice(from: from, to: id) }
            controllers[d.entryID] = c
            order.append(d.entryID)
            changed = true
        }
        // Names: with several interfaces each is told apart by its GUID (the reference matches devices by GUID, A2-P1).
        let multi = controllers.count > 1
        let entries = ordered.map { c -> (id: UInt64, title: String) in (c.device.entryID, "\(c.device.info.name)  \u{2014}  \(c.device.info.guid)") }
        for c in ordered {
            let info = c.device.info
            c.displayName = multi ? "\(info.name) \u{00B7} \(String(info.guid.suffix(6)))" : info.name
            c.window.title = multi ? "M-Audio FireWire 1814 \u{2014} \(info.guid)" : "M-Audio FireWire 1814"
            c.setDevices(entries)
        }
        // Show a panel when the first interface appears or the visible one went away; a newly added second interface waits in the menu.
        if changed, !ordered.contains(where: { $0.visibleWindow != nil }), let first = ordered.first {
            first.setMode(max(0, first.mode), center: center)
        }
        if controllers.isEmpty { showWaiting() } else { waiting?.orderOut(nil); waiting = nil }
    }

    /// Shows interface `id` where `from` is showing (same centre, same view), and hides `from`.
    private func switchDevice(from: PanelWindowController?, to id: UInt64) {
        guard let target = controllers[id] else { return }
        guard let src = from, src !== target else { target.show(); return }
        let w = src.visibleWindow
        let center = w.map { NSPoint(x: $0.frame.midX, y: $0.frame.midY) } ?? screenCenter()
        let m = max(0, src.mode)
        src.hideAll()
        target.setMode(m, center: center)
    }

    private func showWaiting() {
        if waiting != nil { return }
        let w = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 440, height: 132), styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
        w.title = "M-Audio FireWire 1814"
        w.isReleasedWhenClosed = false
        styleWindow(w)
        let root = FillView(Theme.window)
        let h = makeLabel("No FireWire 1814 found", size: 14, weight: .semibold, color: Theme.text)
        h.frame = NSRect(x: 24, y: 24, width: 392, height: 20)
        let t = makeLabel("Turn the interface on and connect it before starting the computer; the panel opens when it appears.", size: 12, color: Theme.text2)
        t.maximumNumberOfLines = 3; t.lineBreakMode = .byWordWrapping
        t.frame = NSRect(x: 24, y: 52, width: 392, height: 52)
        root.addSubview(h); root.addSubview(t)
        w.contentView = root
        placeWindow(w, center: screenCenter()); w.makeKeyAndOrderFront(nil)
        waiting = w
    }

    // MARK: menus

    private func buildMenu() {
        let main = NSMenu()
        func add(_ menu: NSMenu) { let item = NSMenuItem(); item.submenu = menu; main.addItem(item) }
        let app = NSMenu()
        app.addItem(withTitle: "About OpenAudio FW1814 Panel", action: #selector(NSApplication.orderFrontStandardAboutPanel(_:)), keyEquivalent: "")
        app.addItem(NSMenuItem.separator())
        app.addItem(withTitle: "Hide OpenAudio FW1814 Panel", action: #selector(NSApplication.hide(_:)), keyEquivalent: "h")
        app.addItem(NSMenuItem.separator())
        app.addItem(withTitle: "Quit OpenAudio FW1814 Panel", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q")
        add(app)
        deviceMenu.delegate = self                          // rebuilt when opened (reference: DoDeferedDeviceMenuUpdate / BuildDeviceMenu)
        add(deviceMenu)
        let view = NSMenu(title: "View")
        for (i, t) in ["Mixer", "Output", "Hardware", "About"].enumerated() {
            let item = NSMenuItem(title: t, action: #selector(viewTab(_:)), keyEquivalent: "\(i + 1)")
            item.target = self; item.tag = i
            view.addItem(item)
        }
        view.addItem(NSMenuItem.separator())
        for (i, t) in ["Full Panel", "Wide Meters", "Narrow Meters"].enumerated() {
            let item = NSMenuItem(title: t, action: #selector(viewMode(_:)), keyEquivalent: "")
            item.target = self; item.tag = i
            view.addItem(item)
        }
        add(view)
        let win = NSMenu(title: "Window")
        win.addItem(withTitle: "Minimize", action: #selector(NSWindow.performMiniaturize(_:)), keyEquivalent: "m")
        win.addItem(withTitle: "Close", action: #selector(NSWindow.performClose(_:)), keyEquivalent: "w")
        add(win)
        NSApp.mainMenu = main
        NSApp.windowsMenu = win
    }

    func menuNeedsUpdate(_ menu: NSMenu) {
        guard menu === deviceMenu else { return }
        menu.removeAllItems()
        let cur = front
        if ordered.isEmpty { menu.addItem(withTitle: "No FireWire 1814 connected", action: nil, keyEquivalent: "") }
        for c in ordered {
            let info = c.device.info
            let item = NSMenuItem(title: "\(info.name)  \u{2014}  \(info.guid)", action: #selector(chooseDevice(_:)), keyEquivalent: "")
            item.target = self
            item.representedObject = NSNumber(value: c.device.entryID)
            item.state = (c === cur) ? .on : .off
            menu.addItem(item)
        }
    }

    @objc private func chooseDevice(_ item: NSMenuItem) {
        guard let n = item.representedObject as? NSNumber else { return }
        switchDevice(from: front, to: n.uint64Value)
    }
    @objc private func viewTab(_ item: NSMenuItem) {
        guard let c = front ?? ordered.first else { return }
        c.setMode(0)
        c.selectTab(["mixer", "output", "hardware", "about"][item.tag])
    }
    @objc private func viewMode(_ item: NSMenuItem) { (front ?? ordered.first)?.setMode(item.tag) }

    // MARK: debug snapshots (`--demo ... --snapshot DIR`)

    private func snapshot(dir: String, prefix: String, knob: Int, ab: (MAHeadphoneSource, MAHeadphoneSource)?) {
        let cs = ordered
        guard let first = cs.first else { return }
        if cs.count == 1 { first.snapshotAll(to: dir, prefix: prefix, knob: knob, ab: ab); return }
        first.snapshotAll(to: dir, prefix: prefix + "-dev1", knob: knob, ab: ab)
        // Switch to the second interface as the header menu does, and show where its window lands.
        let before = first.window.frame
        switchDevice(from: first, to: cs[1].device.entryID)
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.4))
        let after = cs[1].window.frame
        print(String(format: "%@ switch dev1 -> dev2: dev1 centre %.0f,%.0f visible %@; dev2 centre %.0f,%.0f visible %@", prefix,
                     before.midX, before.midY, first.visibleWindow == nil ? "no" : "yes", after.midX, after.midY, cs[1].visibleWindow == nil ? "no" : "yes"))
        cs[1].snapshotAll(to: dir, prefix: prefix + "-dev2", knob: 0, ab: nil)
    }
}
