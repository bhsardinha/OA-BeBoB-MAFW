// OpenAudio BeBoB MAFW uninstaller: a one-window AppKit app. It lists what the installer put on the Mac, asks for the administrator password through the
// standard macOS dialog (osascript "with administrator privileges") and runs the uninstall script that is bundled inside the app (Resources/uninstall.sh,
// the same file the package installs at /Library/Application Support/OpenAudio/uninstall.sh). Optionally it also deletes the saved interface settings
// (the helper's per-user preferences file). Built with Xcode 9.4.1 / Swift 4.1 for 10.13; the same app ships in the Classic and the Modern disk image.
// Progress (user, 2026-10-22): the script appends "<percent>|<text>" lines to a file in this user's temporary folder (--progress); the app reads it ten
// times a second and shows the last line as the status and the bar. `--dry-run` (command line) runs the script without the password and changes nothing.
import AppKit

final class Controller: NSObject, NSApplicationDelegate {
    let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 500, height: 380), styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
    let keepSettings = NSButton(checkboxWithTitle: "Also delete the saved interface settings (levels, routing, clock)", target: nil, action: nil)
    let go = NSButton(title: "Uninstall", target: nil, action: nil)
    let cancel = NSButton(title: "Cancel", target: nil, action: nil)
    let bar = NSProgressIndicator()
    let status = NSTextField(labelWithString: "")
    let dryRun = CommandLine.arguments.contains("--dry-run")
    var progressFile = ""
    var poll: Timer?
    var lastStep = ""

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
        status.frame = NSRect(x: 24, y: 80, width: 452, height: 18); status.textColor = .secondaryLabelColor
        status.lineBreakMode = .byTruncatingTail; status.maximumNumberOfLines = 1; v.addSubview(status)
        bar.style = .bar; bar.isIndeterminate = false; bar.minValue = 0; bar.maxValue = 100; bar.doubleValue = 0
        bar.isDisplayedWhenStopped = true; bar.isHidden = true; bar.frame = NSRect(x: 24, y: 58, width: 452, height: 20); v.addSubview(bar)
        if dryRun { window.title += " (dry run: nothing is removed)"; go.title = "Dry Run" }
        go.frame = NSRect(x: 372, y: 18, width: 104, height: 32); go.bezelStyle = .rounded; go.keyEquivalent = "\r"; go.target = self; go.action = #selector(run); v.addSubview(go)
        cancel.frame = NSRect(x: 262, y: 18, width: 100, height: 32); cancel.bezelStyle = .rounded; cancel.keyEquivalent = "\u{1b}"; cancel.target = self; cancel.action = #selector(quit); v.addSubview(cancel)
        window.center(); window.makeKeyAndOrderFront(nil)
        // screenshot aid: `--dry-run --start` begins the dry run without a click (only together with --dry-run)
        if dryRun && CommandLine.arguments.contains("--start") { DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { self.run() } }
    }

    @objc func quit() { NSApp.terminate(nil) }
    func applicationShouldTerminateAfterLastWindowClosed(_ app: NSApplication) -> Bool { return true }

    /// Single quotes for the shell.
    static func shq(_ s: String) -> String { return "'" + s.replacingOccurrences(of: "'", with: "'\\''") + "'" }

    @objc func run() {
        guard let script = Bundle.main.path(forResource: "uninstall", ofType: "sh") else { finish(false, "The uninstall script is missing from this app."); return }
        go.isEnabled = false; cancel.isEnabled = false; keepSettings.isEnabled = false
        bar.isHidden = false; bar.doubleValue = 0      // determinate from the start (an indeterminate bar redraws late on 14 when switched over)
        status.stringValue = dryRun ? "Dry run\u{2026}" : "Waiting for the administrator password\u{2026}"
        // A fresh progress file of this user, in this user's own temporary folder; the script (root) appends to it.
        progressFile = (NSTemporaryDirectory() as NSString).appendingPathComponent("oa-uninstall-progress-\(getpid()).txt")
        FileManager.default.createFile(atPath: progressFile, contents: nil, attributes: nil)
        poll = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { [weak self] _ in self?.readProgress() }
        let deleteSettings = keepSettings.state == .on && !dryRun
        let command = Controller.shq(script) + " --progress " + Controller.shq(progressFile) + (dryRun ? " --dry-run" : "")
        let dry = dryRun
        DispatchQueue.global().async {
            let p = Process()
            if dry {
                p.launchPath = "/bin/bash"; p.arguments = ["-c", command]                // no password: the script changes nothing
            } else {
                // The script runs as root through the system's own authorization dialog; the command is escaped for the AppleScript string.
                let apple = "do shell script \"" + command.replacingOccurrences(of: "\\", with: "\\\\").replacingOccurrences(of: "\"", with: "\\\"") + "\" with administrator privileges"
                p.launchPath = "/usr/bin/osascript"; p.arguments = ["-e", apple]
            }
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
            DispatchQueue.main.async {
                self.readProgress()
                // keep the finished bar on screen for a moment before the alert
                DispatchQueue.main.asyncAfter(deadline: .now() + 0.6) { self.finish(ok, ok ? "" : msg) }
            }
        }
    }

    /// The last "<percent>|<text>" line of the progress file -> bar and status line.
    func readProgress() {
        guard let text = try? String(contentsOfFile: progressFile, encoding: .utf8),
              let line = text.split(separator: "\n").last else { return }
        let parts = line.split(separator: "|", maxSplits: 1)
        guard parts.count == 2, let pct = Double(parts[0]) else { return }
        bar.doubleValue = min(100, max(0, pct))
        lastStep = String(parts[1])
        status.stringValue = "\(Int(bar.doubleValue)) %  \u{2014}  " + lastStep
    }

    func stopPolling() {
        poll?.invalidate(); poll = nil
        if !progressFile.isEmpty { try? FileManager.default.removeItem(atPath: progressFile) }
    }

    func reset(_ text: String) {
        stopPolling()
        bar.isHidden = true; bar.doubleValue = 0
        go.isEnabled = true; cancel.isEnabled = true; keepSettings.isEnabled = true; status.stringValue = text
    }

    func finish(_ ok: Bool, _ detail: String) {
        let stoppedAt = lastStep
        stopPolling()
        let a = NSAlert()
        if ok && dryRun {
            a.messageText = "Dry run finished"
            a.informativeText = "Every step ran in dry-run mode; nothing was removed."
            a.addButton(withTitle: "Quit")
        } else if ok {
            a.messageText = "OpenAudio BeBoB MAFW was removed"
            a.informativeText = "Restart the Mac to finish unloading the driver. The interface will not appear in Core Audio until the driver is installed again."
            a.addButton(withTitle: "Quit")
        } else {
            a.alertStyle = .warning
            a.messageText = "The uninstall did not finish"
            let base = detail.isEmpty ? "Something went wrong. Quit the applications that use the interface and try again." : detail
            a.informativeText = stoppedAt.isEmpty ? base : "Stopped at: " + stoppedAt + "\n\n" + base
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
