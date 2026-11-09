// OpenAudio Glue app - entry point. An autonomous app (user, 2026-10-28: "like the firmware tool: can be opened from the panel,
// but is an autonomous thing"). Arguments:
//   --demo            two simulated FW1814 Glue devices instead of Core Audio
//   --snapshot DIR    write the window as PNGs (unit A, unit B, unit A with the Dual Glue aggregate) and quit; with --demo this needs
//                     no hardware (the layout check, as the main panel's --demo --snapshot)
import AppKit

final class AppDelegate: NSObject, NSApplicationDelegate {
    private var controller: GlueWindowController?
    private let args = CommandLine.arguments

    private func value(after flag: String) -> String? {
        guard let i = args.index(of: flag), i + 1 < args.count else { return nil }
        return args[i + 1]
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        let backend: GlueBackend = args.contains("--demo") ? DemoGlueBackend() : CoreAudioGlueBackend()
        let c = GlueWindowController(backend: backend)
        controller = c
        c.fitToContent()
        c.window.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
        buildMenu()
        if let t = value(after: "--template") {                       // demo / snapshot aid: band16 or dual32
            if t == "dual32" { c.applyDual32() } else { c.applyBand16() }
        }
        if args.contains("--rename-test") {                             // test aid: start renaming card 1 (then type, or snapshot)
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { c.debugBeginRename(0) }
        }
        if let dir = value(after: "--snapshot") {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.8) { self.snapshots(c, dir: dir, backend: backend) }
        }
    }

    // MARK: menus
    private func item(_ title: String, _ action: Selector, _ key: String = "", target: AnyObject? = nil) -> NSMenuItem {
        let i = NSMenuItem(title: title, action: action, keyEquivalent: key)
        i.target = target ?? self
        return i
    }

    private func buildMenu() {
        let main = NSMenu()
        let appItem = NSMenuItem(); main.addItem(appItem)
        let appMenu = NSMenu()
        appMenu.addItem(item("About OpenAudio Glue", #selector(NSApplication.orderFrontStandardAboutPanel(_:)), target: NSApp))
        appMenu.addItem(.separator())
        appMenu.addItem(item("Quit OpenAudio Glue", #selector(NSApplication.terminate(_:)), "q", target: NSApp))
        appItem.submenu = appMenu
        // Edit: the standard actions, so renaming a card has copy / paste / select all
        let editItem = NSMenuItem(); main.addItem(editItem)
        let edit = NSMenu(title: "Edit")
        edit.addItem(NSMenuItem(title: "Cut", action: #selector(NSText.cut(_:)), keyEquivalent: "x"))
        edit.addItem(NSMenuItem(title: "Copy", action: #selector(NSText.copy(_:)), keyEquivalent: "c"))
        edit.addItem(NSMenuItem(title: "Paste", action: #selector(NSText.paste(_:)), keyEquivalent: "v"))
        edit.addItem(NSMenuItem(title: "Select All", action: #selector(NSText.selectAll(_:)), keyEquivalent: "a"))
        editItem.submenu = edit
        // Names: standard channel names (user, 2026-10-28)
        let namesItem = NSMenuItem(); main.addItem(namesItem)
        let names = NSMenu(title: "Names")
        names.autoenablesItems = true
        names.addItem(item("Band \u{2014} 16 Inputs (This Unit)", #selector(band16)))
        names.addItem(item("Band \u{2014} 32 Inputs (Dual: A + B)", #selector(dual32)))
        names.addItem(.separator())
        names.addItem(item("Also Set Sources, On/Off and Stereo Links", #selector(toggleAlso)))
        names.addItem(.separator())
        names.addItem(item("Use the Device's Names (This Unit)", #selector(resetNames)))
        namesItem.submenu = names
        NSApp.mainMenu = main
    }

    @objc private func band16() { controller?.applyBand16() }
    @objc private func dual32() { controller?.applyDual32() }
    @objc private func resetNames() { controller?.resetNames() }
    @objc private func toggleAlso() { controller?.templatesAlsoSetSettings = !(controller?.templatesAlsoSetSettings ?? true) }

    override func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        if menuItem.action == #selector(dual32) { return controller?.hasTwoUnits ?? false }
        if menuItem.action == #selector(toggleAlso) { menuItem.state = (controller?.templatesAlsoSetSettings ?? true) ? .on : .off }
        return true
    }

    private func write(_ w: NSWindow, _ path: String) {
        // Layers (EQ curves, meters) are captured as WindowServer shows the window (cacheDisplay misses them), content area only;
        // the same method as the main panel's DemoBackend.writePNG.
        if let img = CGWindowListCreateImage(.null, .optionIncludingWindow, CGWindowID(w.windowNumber), [.boundsIgnoreFraming, .bestResolution]) {
            let scale = CGFloat(img.width) / w.frame.width
            let content = w.contentRect(forFrameRect: w.frame)
            let crop = CGRect(x: 0, y: (w.frame.maxY - content.maxY) * scale, width: content.width * scale, height: content.height * scale)
            if let cut = img.cropping(to: crop), let data = NSBitmapImageRep(cgImage: cut).representation(using: .png, properties: [:]) {
                try? data.write(to: URL(fileURLWithPath: path))
            }
        }
    }

    private func snapshots(_ c: GlueWindowController, dir: String, backend: GlueBackend) {
        try? FileManager.default.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        write(c.window, dir + "/glue-unit-a.png")
        c.selectForSnapshot(unit: 1)
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.4) {
            self.write(c.window, dir + "/glue-unit-b.png")
            c.selectForSnapshot(unit: 0, dual: true)
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.4) {
                self.write(c.window, dir + "/glue-unit-a-dual.png")
                NSApp.terminate(nil)
            }
        }
    }
}

let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate
app.setActivationPolicy(.regular)
app.run()
