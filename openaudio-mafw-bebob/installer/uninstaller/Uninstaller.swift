// OpenAudio BeBoB MAFW uninstaller: a one-window AppKit app. It lists what the installer put on the Mac, asks for the administrator password through the
// standard macOS dialog (osascript "with administrator privileges") and runs the uninstall script that is bundled inside the app (Resources/uninstall.sh,
// the same file the package installs at /Library/Application Support/OpenAudio/uninstall.sh). Optionally it also deletes the saved interface settings
// (the helper's per-user preferences file). Built with Xcode 9.4.1 / Swift 4.1 for 10.13.
import AppKit

final class Controller: NSObject, NSApplicationDelegate {
    let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 500, height: 380), styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
    let keepSettings = NSButton(checkboxWithTitle: "Also delete the saved interface settings (levels, routing, clock)", target: nil, action: nil)
    let go = NSButton(title: "Uninstall", target: nil, action: nil)
    let cancel = NSButton(title: "Cancel", target: nil, action: nil)
    let spinner = NSProgressIndicator()
    let status = NSTextField(labelWithString: "")

    func applicationDidFinishLaunching(_ n: Notification) {
        NSApp.activate(ignoringOtherApps: true)
        window.title = "Uninstall OA BeBoB MAFW"
        let v = window.contentView!
        let icon = NSImageView(frame: NSRect(x: 24, y: 292, width: 64, height: 64)); icon.image = NSApp.applicationIconImage; v.addSubview(icon)
        let title = NSTextField(labelWithString: "Remove the OpenAudio BeBoB MAFW driver?")
        title.font = NSFont.boldSystemFont(ofSize: 16); title.frame = NSRect(x: 104, y: 326, width: 372, height: 24); v.addSubview(title)
        let sub = NSTextField(wrappingLabelWithString: "Quit any audio or MIDI application that uses the interface first. Your audio files and projects are not touched.")
        sub.frame = NSRect(x: 104, y: 288, width: 372, height: 34); v.addSubview(sub)
        let list = NSTextField(wrappingLabelWithString:
            "This will remove:\n" +
            "   \u{2022}  the driver (/Library/Extensions/OpenAudioBeBoBMAFW.kext)\n" +
            "   \u{2022}  the helper daemon (firmware start, settings)\n" +
            "   \u{2022}  the control panel (/Applications/OA BeBoB MAFW.app)\n" +
            "   \u{2022}  the M-Audio CoreMIDI driver and the device icons\n" +
            "   \u{2022}  this uninstall script")
        list.frame = NSRect(x: 24, y: 138, width: 452, height: 120); v.addSubview(list)
        keepSettings.frame = NSRect(x: 24, y: 106, width: 452, height: 20); keepSettings.state = .off; v.addSubview(keepSettings)
        status.frame = NSRect(x: 24, y: 64, width: 452, height: 34); status.textColor = .secondaryLabelColor; v.addSubview(status)
        spinner.style = .spinning; spinner.isDisplayedWhenStopped = false; spinner.frame = NSRect(x: 24, y: 24, width: 24, height: 24); v.addSubview(spinner)
        go.frame = NSRect(x: 372, y: 18, width: 104, height: 32); go.bezelStyle = .rounded; go.keyEquivalent = "\r"; go.target = self; go.action = #selector(run); v.addSubview(go)
        cancel.frame = NSRect(x: 262, y: 18, width: 100, height: 32); cancel.bezelStyle = .rounded; cancel.keyEquivalent = "\u{1b}"; cancel.target = self; cancel.action = #selector(quit); v.addSubview(cancel)
        window.center(); window.makeKeyAndOrderFront(nil)
    }

    @objc func quit() { NSApp.terminate(nil) }
    func applicationShouldTerminateAfterLastWindowClosed(_ app: NSApplication) -> Bool { return true }

    @objc func run() {
        guard let script = Bundle.main.path(forResource: "uninstall", ofType: "sh") else { finish(false, "The uninstall script is missing from this app."); return }
        go.isEnabled = false; cancel.isEnabled = false; spinner.startAnimation(nil); status.stringValue = "Waiting for the administrator password\u{2026}"
        let deleteSettings = keepSettings.state == .on
        DispatchQueue.global().async {
            // The script runs as root through the system's own authorization dialog; the path is quoted for the shell inside AppleScript.
            let quoted = script.replacingOccurrences(of: "'", with: "'\\''")
            let apple = "do shell script \"'" + quoted.replacingOccurrences(of: "\\", with: "\\\\").replacingOccurrences(of: "\"", with: "\\\"") + "'\" with administrator privileges"
            let p = Process(); p.launchPath = "/usr/bin/osascript"; p.arguments = ["-e", apple]
            let err = Pipe(); p.standardError = err; p.standardOutput = Pipe()
            p.launch(); p.waitUntilExit()
            let msg = String(data: err.fileHandleForReading.readDataToEndOfFile(), encoding: .utf8) ?? ""
            var ok = p.terminationStatus == 0
            if ok && deleteSettings {
                let d = Process(); d.launchPath = "/usr/bin/defaults"; d.arguments = ["delete", "com.m-audio.device-settings.firewire"]
                d.standardError = Pipe(); d.standardOutput = Pipe(); d.launch(); d.waitUntilExit()
            }
            if !ok && msg.contains("User canceled") { DispatchQueue.main.async { self.reset("Cancelled.") }; return }
            if !ok && msg.isEmpty { ok = false }
            DispatchQueue.main.async { self.finish(ok, ok ? "" : msg) }
        }
    }

    func reset(_ text: String) {
        spinner.stopAnimation(nil); go.isEnabled = true; cancel.isEnabled = true; status.stringValue = text
    }

    func finish(_ ok: Bool, _ detail: String) {
        spinner.stopAnimation(nil)
        let a = NSAlert()
        if ok {
            a.messageText = "OpenAudio BeBoB MAFW was removed"
            a.informativeText = "Restart the Mac to finish unloading the driver. The interface will not appear in Core Audio until the driver is installed again."
            a.addButton(withTitle: "Quit")
        } else {
            a.alertStyle = .warning
            a.messageText = "The uninstall did not finish"
            a.informativeText = detail.isEmpty ? "Something went wrong. Quit the applications that use the interface and try again." : detail
            a.addButton(withTitle: "Quit")
        }
        a.runModal()
        NSApp.terminate(nil)
    }
}

let app = NSApplication.shared
let controller = Controller()
app.delegate = controller
app.setActivationPolicy(.regular)
app.run()
