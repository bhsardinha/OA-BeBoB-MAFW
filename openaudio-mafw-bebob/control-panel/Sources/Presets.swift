// PN.7 -- mixer presets for the sidebar `settings` buttons: save, load, delete (reset is MAMetaResetToFactorySettings). The reference keeps
// settings files through a Mac file dialog (manual §9); its file format is not in `_extracted/` (A2-P13 §7), so this panel uses a plain
// property list of the mixer keys (DeviceModel.presetKeys) and does not read the reference's files. Dark mode and shortcuts come with the
// system controls; no capability of the reference is dropped.
import AppKit

final class PresetStore {
    static let folder: URL = {
        let base = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]
        return base.appendingPathComponent("OpenAudio BeBoB MAFW/Presets", isDirectory: true)
    }()

    private let model: DeviceModel
    private(set) var loadedFile: URL?
    var onChange: (() -> Void)?

    private let fileStem: String
    init(model: DeviceModel, fileStem: String = "FW1814") { self.model = model; self.fileStem = fileStem }

    var canDelete: Bool { return loadedFile != nil }

    func save(from window: NSWindow?) {
        try? FileManager.default.createDirectory(at: PresetStore.folder, withIntermediateDirectories: true, attributes: nil)
        let panel = NSSavePanel()
        panel.directoryURL = PresetStore.folder
        panel.nameFieldStringValue = "\(fileStem) settings.plist"
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

    /// Status menu (AP-5.5): the preset files in the folder that fit this unit (fittingPreset not empty), by name.
    func fittingFiles() -> [URL] {
        let urls = (try? FileManager.default.contentsOfDirectory(at: PresetStore.folder, includingPropertiesForKeys: nil)) ?? []
        return urls.filter { $0.pathExtension == "plist" }
                   .filter { u in (NSDictionary(contentsOf: u) as? [String: Any]).map { !model.fittingPreset($0).isEmpty } ?? false }
                   .sorted { $0.lastPathComponent.localizedStandardCompare($1.lastPathComponent) == .orderedAscending }
    }
    /// Loads one file without a dialog, only the keys that fit this unit.
    func loadFile(_ url: URL) {
        guard let d = NSDictionary(contentsOf: url) as? [String: Any] else { NSSound.beep(); return }
        let fit = model.fittingPreset(d)
        guard !fit.isEmpty else { NSSound.beep(); return }
        model.restore(fit)
        loadedFile = url; onChange?()
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
