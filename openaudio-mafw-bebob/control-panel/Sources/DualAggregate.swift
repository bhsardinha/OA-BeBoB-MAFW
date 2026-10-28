// Dual (PANEL_REDESIGN_PLAN_v3 §4, M7): the Core Audio side of the "Dual FireWire 1814" aggregate — find it, read its composition, create and
// remove it — and the HAL UID of a unit (the UID ends with the unit's GUID, as HALRate matches it). AudioHardwareCreateAggregateDevice /
// AudioHardwareDestroyAggregateDevice and the "uid" / "name" / "subdevices" / "master" / "private" / "drift" keys: 10.13 SDK AudioHardware.h
// (available since 10.9). The panel creates or removes the aggregate only when the user presses the button.
import CoreAudio
import IOKit

enum DualAggregate {
    private static let system = AudioObjectID(kAudioObjectSystemObject)

    private static func devices() -> [AudioObjectID] {
        var a = AudioObjectPropertyAddress(mSelector: kAudioHardwarePropertyDevices, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var size: UInt32 = 0
        guard AudioObjectGetPropertyDataSize(system, &a, 0, nil, &size) == noErr, size > 0 else { return [] }
        var ids = [AudioObjectID](repeating: 0, count: Int(size) / MemoryLayout<AudioObjectID>.size)
        guard AudioObjectGetPropertyData(system, &a, 0, nil, &size, &ids) == noErr else { return [] }
        return ids
    }
    private static func string(_ id: AudioObjectID, _ sel: AudioObjectPropertySelector) -> String? {
        var a = AudioObjectPropertyAddress(mSelector: sel, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var cf: Unmanaged<CFString>?
        var size = UInt32(MemoryLayout<Unmanaged<CFString>?>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &cf) == noErr, let s = cf else { return nil }
        return s.takeRetainedValue() as String
    }

    /// The HAL UID of the unit whose GUID is `guid` ("0xD6C040012337F"); nil until Core Audio lists it.
    static func unitUID(guid: String) -> String? {
        let g = guid.lowercased()
        for id in devices() { if let u = string(id, kAudioDevicePropertyDeviceUID), u.lowercased().hasSuffix(g) { return u } }
        return nil
    }

    static func find() -> AudioObjectID? {
        return devices().first { string($0, kAudioDevicePropertyDeviceUID) == DualSetup.aggregateUID }
    }

    private static func channels(_ id: AudioObjectID, input: Bool) -> Int {
        var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyStreamConfiguration,
                                           mScope: input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput, mElement: 0)
        var size: UInt32 = 0
        guard AudioObjectGetPropertyDataSize(id, &a, 0, nil, &size) == noErr, size > 0 else { return 0 }
        let raw = UnsafeMutableRawPointer.allocate(byteCount: Int(size), alignment: MemoryLayout<AudioBufferList>.alignment)
        defer { raw.deallocate() }
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, raw) == noErr else { return 0 }
        let list = UnsafeMutableAudioBufferListPointer(raw.assumingMemoryBound(to: AudioBufferList.self))
        return list.reduce(0) { $0 + Int($1.mNumberChannels) }
    }

    /// The aggregate's composition as Core Audio has it.
    static func state() -> DualAggregateState {
        guard let id = find() else { return .missing }
        var g = DualAggregateState(exists: true, subUIDs: [], masterUID: string(id, kAudioAggregateDevicePropertyMasterSubDevice), drift: [:],
                                   inputs: channels(id, input: true), outputs: channels(id, input: false))
        var a = AudioObjectPropertyAddress(mSelector: kAudioAggregateDevicePropertyFullSubDeviceList, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var cf: Unmanaged<CFArray>?
        var size = UInt32(MemoryLayout<Unmanaged<CFArray>?>.size)
        if AudioObjectGetPropertyData(id, &a, 0, nil, &size, &cf) == noErr, let arr = cf?.takeRetainedValue() as? [String] { g.subUIDs = arr }
        // drift compensation per sub-device (kAudioSubDevicePropertyDriftCompensation on the aggregate's sub-device objects)
        var sa = AudioObjectPropertyAddress(mSelector: kAudioObjectPropertyOwnedObjects, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var n: UInt32 = 0
        if AudioObjectGetPropertyDataSize(id, &sa, 0, nil, &n) == noErr, n > 0 {
            var subs = [AudioObjectID](repeating: 0, count: Int(n) / MemoryLayout<AudioObjectID>.size)
            if AudioObjectGetPropertyData(id, &sa, 0, nil, &n, &subs) == noErr {
                for s in subs {
                    var d = AudioObjectPropertyAddress(mSelector: kAudioSubDevicePropertyDriftCompensation, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
                    var v: UInt32 = 0
                    var vs = UInt32(MemoryLayout<UInt32>.size)
                    if AudioObjectGetPropertyData(s, &d, 0, nil, &vs, &v) == noErr, let u = string(s, kAudioDevicePropertyDeviceUID) { g.drift[u] = v != 0 }
                }
            }
        }
        return g
    }

    /// Creates the aggregate (A then B, A master, no drift correction). Returns the OSStatus.
    @discardableResult static func create(uidA: String, uidB: String) -> OSStatus {
        var id: AudioObjectID = 0
        return AudioHardwareCreateAggregateDevice(DualSetup.description(uidA: uidA, uidB: uidB) as CFDictionary, &id)
    }

    @discardableResult static func destroy() -> OSStatus {
        guard let id = find() else { return noErr }
        return AudioHardwareDestroyAggregateDevice(id)
    }

    /// Registry id of the FireWire controller (bus) above a registry entry, nil if none is found.
    static func busID(of service: io_registry_entry_t) -> UInt64? {
        var cur = service
        IOObjectRetain(cur)
        for _ in 0..<32 {
            var parent: io_registry_entry_t = 0
            let kr = IORegistryEntryGetParentEntry(cur, kIOServicePlane, &parent)
            IOObjectRelease(cur)
            guard kr == KERN_SUCCESS else { return nil }
            if IOObjectConformsTo(parent, "IOFireWireController") != 0 {
                var id: UInt64 = 0
                IORegistryEntryGetRegistryEntryID(parent, &id)
                IOObjectRelease(parent)
                return id
            }
            cur = parent
        }
        IOObjectRelease(cur)
        return nil
    }
}
