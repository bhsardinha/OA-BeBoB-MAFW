// Advanced window (PANEL_REDESIGN_PLAN_v3 §2, v2 §3): what each unit's Advanced window remembers, one property list per GUID beside the
// presets (Application Support/OpenAudio BeBoB MAFW/Advanced/<GUID>.plist): the current mode and one interface scene per mode. Panel-side
// only; the driver and the helper never read it.
import Foundation

final class AdvancedStore {
    static let folder: URL = {
        let base = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]
        return base.appendingPathComponent("OpenAudio BeBoB MAFW/Advanced", isDirectory: true)
    }()

    let url: URL
    /// nil until the window has been opened once for this unit: the current mode's key (a built-in mode's rawValue or a custom mode's id)
    private(set) var modeKey: String?
    var mode: AdvancedMode? { return modeKey.flatMap(kind(of:)) }
    /// Scenes by mode key (built-in modes keep their rawValue, so older files read unchanged)
    private var scenes: [String: [String: Any]] = [:]
    /// M6 (plan v2 §3): modes the user made by duplicating one, each with its own name, scene and view options
    private(set) var customModes: [CustomMode] = []
    /// View option per mode key (plan v2 §3): channels the user hid (AdvancedChannel.id); the Dual views use "dual-daw" / "dual-console"
    private var hidden: [String: [String]] = [:]
    /// The key the views read their view options under (the window's current mode; set before a view is built)
    var viewKey = AdvancedMode.first.rawValue
    /// Channel names the user gave (AdvancedChannel.id -> name); panel-side, per unit
    private(set) var names: [String: String] = [:]
    /// Console mode: the dBFS level shown as 0 VU on its meters (plan v2 §4: -20 / -18 / -14 / -12)
    var meterReference: Int = -18 { didSet { save() } }
    static let references = [-20, -18, -14, -12]
    /// Hybrid mode: the user's outboard loops
    var loops: [HybridLoop] = [] { didSet { save() } }
    /// Monitor mode: input pairs the user linked as one stereo source, by first (odd) mixer channel. Panel-side on purpose: the interface's
    /// own link flag (MAInputLink) is on for every pair on a real FW1814 (capture 23), so it cannot mean "stereo source" (inputs are mono
    /// first, user 2026-10-28)
    var stereoInputs: [Int] = [] { didSet { save() } }
    /// Monitor mode: DAW return pairs (1 = DAW 1/2, 2 = DAW 3/4) joined as one stereo return; the others are two mono returns (user,
    /// 2026-10-28). Panel-side, independent of MASWReturnLink.
    var stereoReturns: [Int] = [] { didSet { save() } }
    /// Monitor mode: levels the panel took down to switch a source OFF, given back by IN (MonitorMixer.parked)
    var parkedLevels: [String: Int] = [:] { didSet { save() } }
    /// Console CUE: Phones 1's source before the first cue (given back after the last one); nil while no cue is on
    var cuePhonesSource: Int? { didSet { save() } }
    /// Monitor mode: the cue mixes' names and mono / stereo (one per hardware bus)
    var monitorMixes: [MonitorMix] = MonitorMix.defaults { didSet { save() } }

    init(guid: String, folder: URL = AdvancedStore.folder) {
        let safe = guid.replacingOccurrences(of: "/", with: "_")
        url = folder.appendingPathComponent("\(safe).plist")
        guard let d = NSDictionary(contentsOf: url) as? [String: Any] else { return }
        scenes = d["Scenes"] as? [String: [String: Any]] ?? [:]
        customModes = (d["CustomModes"] as? [[String: Any]] ?? []).compactMap(CustomMode.init(plist:))
        hidden = d["Hidden"] as? [String: [String]] ?? [:]
        modeKey = (d["Mode"] as? String).flatMap { kind(of: $0) == nil ? nil : $0 }
        names = d["Names"] as? [String: String] ?? [:]
        if let r = d["MeterReference"] as? Int, AdvancedStore.references.contains(r) { meterReference = r }
        if let a = d["MonitorMixes"] as? [[String: Any]] {
            let read = a.compactMap { e -> MonitorMix? in
                guard let b = (e["Bus"] as? String).flatMap({ MixBus(rawValue: $0) }), let n = e["Name"] as? String, let m = e["Mono"] as? Bool else { return nil }
                // "Split" (a short-lived name for the two mono lines) reads as mono
                // the Aux is a stereo pair (odd L, even R; user 2026-10-28); its old default name "Cue C" reads as "Aux"
                return MonitorMix(bus: b, name: b == .aux && n == "Cue C" ? "Aux" : n, mono: b == .aux ? false : m || (e["Split"] as? Bool ?? false),
                                  rightName: e["RightName"] as? String ?? "")
            }
            if read.count == 3 { monitorMixes = read }
        }
        if let a = d["Loops"] as? [[String: Any]] { loops = a.compactMap(HybridLoop.init(plist:)) }
        cuePhonesSource = d["CuePhonesSource"] as? Int
        stereoInputs = (d["StereoInputs"] as? [Int] ?? []).filter { $0 % 2 == 1 }
        stereoReturns = (d["StereoReturns"] as? [Int] ?? []).filter { $0 == 1 || $0 == 2 }
        parkedLevels = d["ParkedLevels"] as? [String: Int] ?? [:]
    }

    func name(_ c: AdvancedChannel) -> String { return names[c.id] ?? c.name }
    /// An empty name goes back to the default.
    func setName(_ n: String, for id: String) {
        let t = n.trimmingCharacters(in: .whitespacesAndNewlines)
        if t.isEmpty { names[id] = nil } else { names[id] = String(t.prefix(16)) }
        save()
    }

    func scene(_ m: AdvancedMode) -> [String: Any]? { return scenes[m.rawValue] }
    func scene(key: String) -> [String: Any]? { return scenes[key] }

    func store(mode m: AdvancedMode, scene s: [String: Any]?) { store(modeKey: m.rawValue, scene: s) }
    func store(modeKey k: String, scene s: [String: Any]?) {
        modeKey = k
        if let s = s { scenes[k] = s }
        save()
    }
    func store(scene s: [String: Any], for m: AdvancedMode) { store(scene: s, key: m.rawValue) }
    func store(scene s: [String: Any], key: String) { scenes[key] = s; save() }

    // MARK: modes by key (M6)

    /// Built-in modes first, then the custom ones in the order they were made
    var modeKeys: [String] { return AdvancedMode.all.map { $0.rawValue } + customModes.map { $0.id } }
    /// The mode a key stands for (a custom mode is its base); nil for an unknown key
    func kind(of key: String) -> AdvancedMode? {
        if let m = AdvancedMode(rawValue: key) { return m }
        return customModes.first { $0.id == key }?.base
    }
    func modeName(_ key: String) -> String { return customModes.first { $0.id == key }?.name ?? AdvancedMode(rawValue: key)?.name ?? key }
    func custom(_ key: String) -> CustomMode? { return customModes.first { $0.id == key } }

    /// ＋: a new mode duplicating `from` (its scene and hidden channels); returns its key
    @discardableResult func addCustom(name: String, from key: String, scene: [String: Any]) -> String? {
        guard let base = kind(of: key) else { return nil }
        var n = 1
        while customModes.contains(where: { $0.id == "custom-\(n)" }) { n += 1 }
        let id = "custom-\(n)"
        customModes.append(CustomMode(id: id, name: CustomMode.clean(name, fallback: base.name + " 2"), base: base))
        scenes[id] = scene
        hidden[id] = hidden[key]
        save()
        return id
    }
    func renameCustom(_ key: String, to name: String) {
        guard let i = customModes.index(where: { $0.id == key }) else { return }
        customModes[i].name = CustomMode.clean(name, fallback: customModes[i].name)
        save()
    }
    /// Removes a custom mode with its scene and view options (the caller switches away first)
    func removeCustom(_ key: String) {
        customModes = customModes.filter { $0.id != key }
        scenes[key] = nil; hidden[key] = nil
        if modeKey == key { modeKey = customModes.isEmpty ? nil : modeKey }
        save()
    }

    // MARK: hidden channels (view option per mode key)

    func hiddenIDs(_ key: String) -> Set<String> { return Set(hidden[key] ?? []) }
    func setHidden(_ id: String, _ on: Bool, in key: String) {
        var s = hiddenIDs(key)
        if on { s.insert(id) } else { s.remove(id) }
        hidden[key] = s.isEmpty ? nil : s.sorted()
        save()
    }
    func showAll(in key: String) { hidden[key] = nil; save() }

    private func save() {
        try? FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true, attributes: nil)
        var d: [String: Any] = ["Scenes": scenes, "Names": names, "MeterReference": meterReference,
                                "MonitorMixes": monitorMixes.map { ["Bus": $0.bus.rawValue, "Name": $0.name, "Mono": $0.mono, "RightName": $0.rightName] as [String: Any] },
                                "Loops": loops.map { $0.plist }, "CustomModes": customModes.map { $0.plist }, "Hidden": hidden,
                                "StereoInputs": stereoInputs, "StereoReturns": stereoReturns,
                                "ParkedLevels": parkedLevels]
        if let m = modeKey { d["Mode"] = m }
        if let c = cuePhonesSource { d["CuePhonesSource"] = c }
        (d as NSDictionary).write(to: url, atomically: true)
    }
}
