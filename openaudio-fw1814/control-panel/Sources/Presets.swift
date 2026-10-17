// PN.7 -- mixer presets for the sidebar `settings` buttons: save, load, delete (reset is MAMetaResetToFactorySettings). The reference keeps
// settings files through a Mac file dialog (manual §9); its file format is not in `_extracted/` (A2-P13 §7), so this panel uses a plain
// property list of the mixer keys (DeviceModel.presetKeys) and does not read the reference's files. Dark mode and shortcuts come with the
// system controls; no capability of the reference is dropped.
import AppKit

final class PresetStore {
    static let folder: URL = {
        let base = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]
        return base.appendingPathComponent("OpenAudio FW1814/Presets", isDirectory: true)
    }()

    private let model: DeviceModel
    private(set) var loadedFile: URL?
    var onChange: (() -> Void)?

    init(model: DeviceModel) { self.model = model }

    var canDelete: Bool { return loadedFile != nil }

    func save(from window: NSWindow?) {
        try? FileManager.default.createDirectory(at: PresetStore.folder, withIntermediateDirectories: true, attributes: nil)
        let panel = NSSavePanel()
        panel.directoryURL = PresetStore.folder
        panel.nameFieldStringValue = "FW1814 settings.plist"
        panel.allowedFileTypes = ["plist"]
        let write: (NSApplication.ModalResponse) -> Void = { [weak self] r in
            guard r == .OK, let url = panel.url, let me = self else { return }
            let snap = me.model.presetSnapshot() as NSDictionary
            if !snap.write(to: url, atomically: true) { NSSound.beep(); return }
            me.loadedFile = url; me.onChange?()
        }
        if let w = window { panel.beginSheetModal(for: w, completionHandler: write) } else { write(panel.runModal()) }
    }

    func load(from window: NSWindow?) {
        let panel = NSOpenPanel()
        panel.directoryURL = PresetStore.folder
        panel.allowedFileTypes = ["plist"]
        panel.canChooseDirectories = false
        let read: (NSApplication.ModalResponse) -> Void = { [weak self] r in
            guard r == .OK, let url = panel.url, let me = self, let d = NSDictionary(contentsOf: url) as? [String: Any] else { return }
            me.model.restore(d)
            me.loadedFile = url; me.onChange?()
        }
        if let w = window { panel.beginSheetModal(for: w, completionHandler: read) } else { read(panel.runModal()) }
    }

    /// Deletes the loaded settings file after confirmation (delete is greyed until a file is loaded, as in the screenshots).
    func deleteLoaded(from window: NSWindow?) {
        guard let url = loadedFile else { return }
        let alert = NSAlert()
        alert.messageText = "Delete \"\(url.lastPathComponent)\"?"
        alert.informativeText = "The settings file is removed. The interface keeps its current settings."
        alert.addButton(withTitle: "Delete"); alert.addButton(withTitle: "Cancel")
        let done: (NSApplication.ModalResponse) -> Void = { [weak self] r in
            guard r == .alertFirstButtonReturn, let me = self else { return }
            try? FileManager.default.removeItem(at: url)
            me.loadedFile = nil; me.onChange?()
        }
        if let w = window { alert.beginSheetModal(for: w, completionHandler: done) } else { done(alert.runModal()) }
    }
}
