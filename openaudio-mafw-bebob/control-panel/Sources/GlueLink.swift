// The main panel's only hook into OpenAudio Glue (Glue ARCHITECTURE_v1, R144): one menu item that opens the standalone app. Glue has its own
// package; its app is pinned to /Applications by that package, so the item is shown only when the app is there and a FireWire 1814 is connected (Glue is FW1814-only, DeviceProfile.glue).
import AppKit

enum GlueLink {
    static let appPath = "/Applications/OpenAudio Glue.app"
    static var installed: Bool { return FileManager.default.fileExists(atPath: appPath) }
    static func open() { if !NSWorkspace.shared.open(URL(fileURLWithPath: appPath)) { NSSound.beep() } }
}
