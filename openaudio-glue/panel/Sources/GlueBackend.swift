// OpenAudio Glue app - where the Glue devices come from: Core Audio (the AudioServerPlugIn's devices, UID com.openaudio.glue:<GUID>,
// settings as the custom property 'glst', meters as 'glmt'; openaudio-glue/driver/src/GlueDevice.h) or a demo with two simulated
// 16-channel FW1814s (--demo, for the layout snapshots).
import AppKit
import CoreAudio

struct GlueUnit {
    let key: String                 // the device UID (demo: "demo-<n>")
    let guidHex: String
    let name: String
    let channels: Int
    let channelNames: [String]
    let rate: Double
    let latency: Int                // input latency in frames, Glue's compensation included
}

struct GlueMeters {
    var gr: [Float] = []
    var hit: [Float] = []
    var running = false
}

protocol GlueBackend: AnyObject {
    var units: [GlueUnit] { get }
    var onUnitsChanged: (() -> Void)? { get set }
    func settings(_ unit: GlueUnit) -> GlueSettings
    func setSettings(_ unit: GlueUnit, _ s: GlueSettings)
    func meters(_ unit: GlueUnit) -> GlueMeters
    /// The order of the main panel's Dual aggregate (unit GUIDs, A first), if it exists.
    func dualOrder() -> [String]
    func dualGlueExists() -> Bool
    func createDualGlue(guidA: String, guidB: String) -> OSStatus
    func removeDualGlue()
}

enum GlueIDs {
    static let gluePrefix = "com.openaudio.glue:"
    static let unitPrefix = "com_m_audio_FWEngine2:"                    // the units' own Core Audio UIDs (FWBaseEngine::getGlobalUniqueID)
    static let dualUID = "com.openaudio.bebobmafw.dual1814"             // the main panel's aggregate (AdvancedDual.swift)
    static let dualGlueUID = "com.openaudio.glue.dual"
    static let dualGlueName = "Dual FireWire 1814 Glue"
    static let settingsSelector: AudioObjectPropertySelector = 0x676C7374   // 'glst'
    static let metersSelector: AudioObjectPropertySelector = 0x676C6D74     // 'glmt'
}

// MARK: - Core Audio

final class CoreAudioGlueBackend: GlueBackend {
    private(set) var units: [GlueUnit] = []
    private var ids: [String: AudioObjectID] = [:]
    var onUnitsChanged: (() -> Void)?

    init() {
        rescan()
        var a = AudioObjectPropertyAddress(mSelector: kAudioHardwarePropertyDevices, mScope: kAudioObjectPropertyScopeGlobal,
                                           mElement: kAudioObjectPropertyElementMaster)
        AudioObjectAddPropertyListenerBlock(AudioObjectID(kAudioObjectSystemObject), &a, DispatchQueue.main) { [weak self] _, _ in
            self?.rescan()
            self?.onUnitsChanged?()
        }
    }

    private func deviceIDs() -> [AudioObjectID] {
        var a = AudioObjectPropertyAddress(mSelector: kAudioHardwarePropertyDevices, mScope: kAudioObjectPropertyScopeGlobal,
                                           mElement: kAudioObjectPropertyElementMaster)
        var size: UInt32 = 0
        guard AudioObjectGetPropertyDataSize(AudioObjectID(kAudioObjectSystemObject), &a, 0, nil, &size) == noErr else { return [] }
        var list = [AudioObjectID](repeating: 0, count: Int(size) / MemoryLayout<AudioObjectID>.size)
        guard AudioObjectGetPropertyData(AudioObjectID(kAudioObjectSystemObject), &a, 0, nil, &size, &list) == noErr else { return [] }
        return list
    }

    private func string(_ id: AudioObjectID, _ sel: AudioObjectPropertySelector, scope: AudioObjectPropertyScope = kAudioObjectPropertyScopeGlobal,
                        element: UInt32 = kAudioObjectPropertyElementMaster) -> String? {
        var a = AudioObjectPropertyAddress(mSelector: sel, mScope: scope, mElement: element)
        var v: Unmanaged<CFString>?
        var size = UInt32(MemoryLayout<Unmanaged<CFString>?>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &v) == noErr, let s = v else { return nil }
        return s.takeRetainedValue() as String
    }

    private func uint32(_ id: AudioObjectID, _ sel: AudioObjectPropertySelector, scope: AudioObjectPropertyScope) -> UInt32 {
        var a = AudioObjectPropertyAddress(mSelector: sel, mScope: scope, mElement: kAudioObjectPropertyElementMaster)
        var v: UInt32 = 0
        var size = UInt32(MemoryLayout<UInt32>.size)
        _ = AudioObjectGetPropertyData(id, &a, 0, nil, &size, &v)
        return v
    }

    private func inputChannels(_ id: AudioObjectID) -> Int {
        var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyStreamConfiguration, mScope: kAudioObjectPropertyScopeInput,
                                           mElement: kAudioObjectPropertyElementMaster)
        var size: UInt32 = 0
        guard AudioObjectGetPropertyDataSize(id, &a, 0, nil, &size) == noErr, size > 0 else { return 0 }
        let mem = UnsafeMutableRawPointer.allocate(byteCount: Int(size), alignment: 16)
        defer { mem.deallocate() }
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, mem) == noErr else { return 0 }
        let list = UnsafeMutableAudioBufferListPointer(mem.assumingMemoryBound(to: AudioBufferList.self))
        return list.reduce(0) { $0 + Int($1.mNumberChannels) }
    }

    func rescan() {
        var found: [GlueUnit] = []
        ids = [:]
        for id in deviceIDs() {
            guard let uid = string(id, kAudioDevicePropertyDeviceUID), uid.hasPrefix(GlueIDs.gluePrefix) else { continue }
            let ch = inputChannels(id)
            var names: [String] = []
            for c in 0..<ch { names.append(string(id, kAudioObjectPropertyElementName, scope: kAudioObjectPropertyScopeInput, element: UInt32(c + 1)) ?? "Input \(c + 1)") }
            var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyNominalSampleRate, mScope: kAudioObjectPropertyScopeGlobal,
                                               mElement: kAudioObjectPropertyElementMaster)
            var rate: Float64 = 0; var size = UInt32(MemoryLayout<Float64>.size)
            _ = AudioObjectGetPropertyData(id, &a, 0, nil, &size, &rate)
            let unit = GlueUnit(key: uid, guidHex: String(uid.dropFirst(GlueIDs.gluePrefix.count)), name: string(id, kAudioObjectPropertyName) ?? "Glue",
                                channels: ch, channelNames: names, rate: rate,
                                latency: Int(uint32(id, kAudioDevicePropertyLatency, scope: kAudioObjectPropertyScopeInput)))
            ids[uid] = id
            found.append(unit)
        }
        units = found.sorted { $0.guidHex < $1.guidHex }
    }

    private func plist(_ id: AudioObjectID, _ sel: AudioObjectPropertySelector) -> [String: Any]? {
        var a = AudioObjectPropertyAddress(mSelector: sel, mScope: kAudioObjectPropertyScopeGlobal, mElement: kAudioObjectPropertyElementMaster)
        var v: Unmanaged<CFPropertyList>?
        var size = UInt32(MemoryLayout<Unmanaged<CFPropertyList>?>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &v) == noErr, let p = v else { return nil }
        return p.takeRetainedValue() as? [String: Any]
    }

    func settings(_ unit: GlueUnit) -> GlueSettings {
        guard let id = ids[unit.key], let p = plist(id, GlueIDs.settingsSelector) else { return GlueSettings() }
        return GlueSettings(plist: p)
    }

    func setSettings(_ unit: GlueUnit, _ s: GlueSettings) {
        guard let id = ids[unit.key] else { return }
        var a = AudioObjectPropertyAddress(mSelector: GlueIDs.settingsSelector, mScope: kAudioObjectPropertyScopeGlobal,
                                           mElement: kAudioObjectPropertyElementMaster)
        var p: CFPropertyList = s.plist(channels: unit.channels) as NSDictionary
        _ = AudioObjectSetPropertyData(id, &a, 0, nil, UInt32(MemoryLayout<CFPropertyList>.size), &p)
    }

    func meters(_ unit: GlueUnit) -> GlueMeters {
        guard let id = ids[unit.key], let p = plist(id, GlueIDs.metersSelector) else { return GlueMeters() }
        var m = GlueMeters()
        m.gr = (p["gr"] as? [NSNumber])?.map { $0.floatValue } ?? []
        m.hit = (p["hit"] as? [NSNumber])?.map { $0.floatValue } ?? []
        m.running = (p["running"] as? Bool) ?? false
        return m
    }

    private func device(uid: String) -> AudioObjectID? {
        for id in deviceIDs() where string(id, kAudioDevicePropertyDeviceUID) == uid { return id }
        return nil
    }

    func dualOrder() -> [String] {
        guard let id = device(uid: GlueIDs.dualUID) else { return [] }
        var a = AudioObjectPropertyAddress(mSelector: kAudioAggregateDevicePropertyFullSubDeviceList, mScope: kAudioObjectPropertyScopeGlobal,
                                           mElement: kAudioObjectPropertyElementMaster)
        var v: Unmanaged<CFArray>?
        var size = UInt32(MemoryLayout<Unmanaged<CFArray>?>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &v) == noErr, let arr = v?.takeRetainedValue() as? [String] else { return [] }
        return arr.filter { $0.hasPrefix(GlueIDs.unitPrefix) }.map { String($0.dropFirst(GlueIDs.unitPrefix.count)) }
    }

    func dualGlueExists() -> Bool { return device(uid: GlueIDs.dualGlueUID) != nil }

    /// "Dual FireWire 1814 Glue": Glue A, Glue B, unit A, unit B; clock from unit A; no drift correction (B is locked to A).
    func createDualGlue(guidA: String, guidB: String) -> OSStatus {
        removeDualGlue()
        let subs: [[String: Any]] = [[kAudioSubDeviceUIDKey: GlueIDs.gluePrefix + guidA, kAudioSubDeviceDriftCompensationKey: 0],
                                     [kAudioSubDeviceUIDKey: GlueIDs.gluePrefix + guidB, kAudioSubDeviceDriftCompensationKey: 0],
                                     [kAudioSubDeviceUIDKey: GlueIDs.unitPrefix + guidA, kAudioSubDeviceDriftCompensationKey: 0],
                                     [kAudioSubDeviceUIDKey: GlueIDs.unitPrefix + guidB, kAudioSubDeviceDriftCompensationKey: 0]]
        let desc: [String: Any] = [kAudioAggregateDeviceUIDKey: GlueIDs.dualGlueUID, kAudioAggregateDeviceNameKey: GlueIDs.dualGlueName,
                                   kAudioAggregateDeviceSubDeviceListKey: subs, kAudioAggregateDeviceMasterSubDeviceKey: GlueIDs.unitPrefix + guidA,
                                   kAudioAggregateDeviceIsPrivateKey: 0]
        var out: AudioObjectID = 0
        return AudioHardwareCreateAggregateDevice(desc as CFDictionary, &out)
    }

    func removeDualGlue() {
        if let id = device(uid: GlueIDs.dualGlueUID) { _ = AudioHardwareDestroyAggregateDevice(id) }
    }
}

// MARK: - demo (no hardware)

final class DemoGlueBackend: GlueBackend {
    let units: [GlueUnit]
    var onUnitsChanged: (() -> Void)?
    private var stored: [String: GlueSettings] = [:]
    private var dual = false
    private let start = CFAbsoluteTimeGetCurrent()

    init() {
        let names = (1...8).map { "Input Analog \($0)" } + (1...8).map { "Input ADAT \($0)" }
        units = [GlueUnit(key: "demo-a", guidHex: "0xD6C040012337F", name: "FireWire 1814 Glue", channels: 16, channelNames: names, rate: 48000, latency: 114),
                 GlueUnit(key: "demo-b", guidHex: "0xD6C040074B9FB", name: "FireWire 1814 Glue", channels: 16, channelNames: names, rate: 48000, latency: 114)]
        var s = GlueSettings()
        s.stages = [2, 1, 2, 3]
        s.channels[0].threshold = -18; s.channels[0].source = 2
        s.channels[0].bands[0] = GlueBand(freq: 90, gain: -3); s.channels[0].bands[2] = GlueBand(freq: 3200, gain: 3.5)
        s.channels[1].threshold = -24; s.channels[1].source = 1; s.channels[1].bands[1] = GlueBand(freq: 380, gain: -5)
        s.channels[2].source = 3; s.channels[2].threshold = -12; s.channels[2].bands[3] = GlueBand(freq: 9000, gain: 4)
        s.channels[3].source = 4; s.channels[3].threshold = -20; s.channels[3].bands[0] = GlueBand(freq: 70, gain: 4); s.channels[3].bands[1] = GlueBand(freq: 250, gain: -4)
        s.channels[6].on = false; s.channels[7].on = false
        for c in 8..<16 { s.channels[c].on = false }
        stored["demo-a"] = s
        stored["demo-b"] = GlueSettings()
    }
    func settings(_ unit: GlueUnit) -> GlueSettings { return stored[unit.key] ?? GlueSettings() }
    func setSettings(_ unit: GlueUnit, _ s: GlueSettings) { stored[unit.key] = s }
    func meters(_ unit: GlueUnit) -> GlueMeters {
        let s = settings(unit), t = CFAbsoluteTimeGetCurrent() - start
        var m = GlueMeters(); m.running = true
        for c in 0..<unit.channels {
            let on = s.channels[c].on && s.channels[c].threshold < 0
            m.gr.append(on ? Float(-(4 + 3 * sin(t * 2 + Double(c)))) : 0)
            m.hit.append(on && s.channels[c].source == 1 ? Float(max(0, 3 * sin(t * 7 + Double(c)))) : 0)
        }
        return m
    }
    func dualOrder() -> [String] { return [units[0].guidHex, units[1].guidHex] }
    func dualGlueExists() -> Bool { return dual }
    func createDualGlue(guidA: String, guidB: String) -> OSStatus { dual = true; return noErr }
    func removeDualGlue() { dual = false }
}
