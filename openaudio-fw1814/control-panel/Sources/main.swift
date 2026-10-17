// PN.1 -- process entry for the AppKit panel.
import AppKit

// Single instance: if the panel is already running (e.g. started with `open -n` or from another copy of the bundle), bring that process to
// the front and exit. The debug modes (`--demo`, `--snapshot`) may run beside a real panel, so they skip the check.
let launchArgs = CommandLine.arguments
if !launchArgs.contains("--demo") && !launchArgs.contains("--snapshot") {
    let me = ProcessInfo.processInfo.processIdentifier
    let bundleID = Bundle.main.bundleIdentifier ?? "com.openaudio.fw1814.panel"
    let others = NSRunningApplication.runningApplications(withBundleIdentifier: bundleID).filter { $0.processIdentifier != me }
    if let running = others.first {
        running.activate(options: [.activateIgnoringOtherApps, .activateAllWindows])
        exit(0)
    }
}

let application = NSApplication.shared
let appDelegate = AppDelegate()
application.delegate = appDelegate
application.setActivationPolicy(.regular)
application.run()
