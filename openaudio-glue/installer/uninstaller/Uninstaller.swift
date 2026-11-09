// OpenAudio Glue uninstaller: a one-window AppKit app on the disk image. It asks for the administrator password through the standard macOS dialog and runs
// the script bundled in the app (Resources/glue-uninstall.sh, the same file the package installs in /Library/Application Support/OpenAudio Glue). The
// OpenAudio driver and panel are not touched. `--dry-run` shows the window and reports what would be removed, without the password. Swift 4.1, 10.13 SDK.
import AppKit

final class Controller: NSObject, NSApplicationDelegate {
    let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 500, height: 270), styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
    let go = NSButton(title: "Uninstall", target: nil, action: nil)
    let cancel = NSButton(title: "Cancel", target: nil, action: nil)
    let status = NSTextField(labelWithString: "")
    let dryRun = CommandLine.arguments.contains("--dry-run")

    func applicationDidFinishLaunching(_ n: Notification) {
        NSApp.activate(ignoringOtherApps: true)
        window.title = "Uninstall OpenAudio Glue" + (dryRun ? " (dry run: nothing is removed)" : "")
        let v = window.contentView!
        let icon = NSImageView(frame: NSRect(x: 24, y: 182, width: 64, height: 64)); icon.image = NSApp.applicationIconImage; v.addSubview(icon)
        let title = NSTextField(labelWithString: "Remove OpenAudio Glue?")
        title.font = NSFont.boldSystemFont(ofSize: 16); title.frame = NSRect(x: 104, y: 216, width: 372, height: 24); v.addSubview(title)
        let sub = NSTextField(wrappingLabelWithString: "Quit any application that uses a Glue device first. Core Audio restarts, so audio stops for a moment. The OpenAudio driver and control panel stay installed.")
        sub.frame = NSRect(x: 104, y: 164, width: 372, height: 50); v.addSubview(sub)
        let list = NSTextField(wrappingLabelWithString:
            "This will remove:\n" +
            "   \u{2022}  the Glue device (/Library/Audio/Plug-Ins/HAL/OpenAudioGlue.driver)\n" +
            "   \u{2022}  the app (/Applications/OpenAudio Glue.app)\n" +
            "   \u{2022}  /Library/Application Support/OpenAudio Glue")
        list.frame = NSRect(x: 24, y: 76, width: 452, height: 76); v.addSubview(list)
        status.frame = NSRect(x: 24, y: 54, width: 452, height: 18); status.textColor = .secondaryLabelColor; v.addSubview(status)
        go.frame = NSRect(x: 372, y: 14, width: 104, height: 32); go.bezelStyle = .rounded; go.keyEquivalent = "\r"; go.target = self; go.action = #selector(run); v.addSubview(go)
        cancel.frame = NSRect(x: 262, y: 14, width: 100, height: 32); cancel.bezelStyle = .rounded; cancel.keyEquivalent = "\u{1b}"; cancel.target = self; cancel.action = #selector(quit); v.addSubview(cancel)
        window.center(); window.makeKeyAndOrderFront(nil)
    }

    @objc func quit() { NSApp.terminate(nil) }
    func applicationShouldTerminateAfterLastWindowClosed(_ app: NSApplication) -> Bool { return true }

    @objc func run() {
        if dryRun { status.stringValue = "Dry run: nothing was removed."; return }
        guard let script = Bundle.main.path(forResource: "glue-uninstall", ofType: "sh") else { status.stringValue = "The uninstall script is missing from this app."; return }
        go.isEnabled = false; cancel.isEnabled = false; status.stringValue = "Removing\u{2026}"
        let shell = "'" + script.replacingOccurrences(of: "'", with: "'\\''") + "'"
        let apple = "do shell script \"" + shell.replacingOccurrences(of: "\\", with: "\\\\").replacingOccurrences(of: "\"", with: "\\\"") + "\" with administrator privileges"
        DispatchQueue.global().async {
            let p = Process(); p.launchPath = "/usr/bin/osascript"; p.arguments = ["-e", apple]
            p.launch(); p.waitUntilExit()
            let ok = p.terminationStatus == 0
            DispatchQueue.main.async {
                self.cancel.isEnabled = true; self.cancel.title = "Close"
                if ok { self.status.stringValue = "OpenAudio Glue was removed."; self.go.isHidden = true }
                else { self.status.stringValue = "Nothing was removed (cancelled, or the script failed)."; self.go.isEnabled = true }
            }
        }
    }
}

let app = NSApplication.shared
let controller = Controller()
app.delegate = controller
app.setActivationPolicy(.regular)
app.run()
