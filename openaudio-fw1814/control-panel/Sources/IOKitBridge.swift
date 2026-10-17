// PN.6 -- IOKit side of the panel: device discovery, registry property read/write, property-change notifications, meter page.
// Mirrors the reference panel (A2-P1/P2/P3/P9): IOServiceAddMatchingNotification for hot-plug, IOServiceOpen for the vendor user client,
// IONotificationPort + CFMachPort run-loop source for the type-0 notification, IORegistryEntrySetCFProperties for every control write
// (never IOConnectCallMethod, B5 §10), IOConnectMapMemory type 1 (flags 0x1001 = anywhere + read-only) for uint32 peak[52].
// The kext notification (kernel template, FWKIPCUserClient.h) arrives as a 24-byte user header followed by the template tail: the
// word at +0x18 is the type field, 0 = "properties changed" (P2). The XML tail is ignored; the panel re-reads the whole dictionary
// (as the reference does) and diffs it. Status: written, type-checked on the build PC, never run against hardware.
import Foundation
import IOKit
import CoreAudio

/// Registry class of the clone's device object (Info.plist personality FW1814Device). Matching also finds subclasses.
let kFWDeviceClassName = "com_openaudio_FW1814Device"

struct FWDeviceInfo {
    let entryID: UInt64
    let guid: String
    let name: String
}

/// What the panel needs from a device: registry properties, notification hook, identity and the meter page. FWDevice is the real one; DemoBackend
/// (command line `--demo`) lets the UI be exercised without an interface.
protocol PanelBackend: PropertyTransport {
    var entryID: UInt64 { get }
    var info: FWDeviceInfo { get }
    var onPropertiesChanged: (() -> Void)? { get set }
    func readPeaks() -> [UInt32]?
    /// The CoreAudio side of the same interface (nil for the demo backend).
    var rate: RateBackend? { get }
}
extension PanelBackend { var rate: RateBackend? { return nil } }

/// The sample rate as the reference panel handles it (A2-P8 §6, Correction 2026-10-17): the panel never writes `MASampleRate`; 1 s after the last
/// pick (CXDevice::SendNewSampleRateTimer @0x6c8c8) it calls CoreAudio `setNominalSampleRate:` on the device's audio object (CXDevice+0x60), and the
/// displayed rate follows CoreAudio. The lock colour comes from `MARateLocked` (kext). A write through setProperties applies the rate to the engine
/// without completing the kext's pending state, so the device is never re-programmed and `MARateLocked` stays 0 (capture 80, 79).
protocol RateBackend: AnyObject {
    var nominalRate: UInt32? { get }
    /// `kAudioDevicePropertyAvailableNominalSampleRates` (the reference reads it through `-supportedSampleRates`, cookies 0x603/0x604, A2-P8 §10); empty when unknown.
    var availableRates: [UInt32] { get }
    @discardableResult func setNominalRate(_ hz: UInt32) -> Bool
    var onRateChanged: (() -> Void)? { get set }
}

/// CoreAudio rate access for one FW1814, matched by GUID (the HAL device UID ends with the GUID: `...:0xD6C040012337F`). The device appears in the HAL
/// a few seconds after the kext's registry object, so it is looked up lazily; a listener on the device list re-looks it up when it (re)appears.
final class HALRate: RateBackend {
    private let guid: String
    private var deviceID: AudioObjectID = 0
    private var listening: AudioObjectID = 0
    var onRateChanged: (() -> Void)?

    init(guid: String) {
        self.guid = guid.lowercased()
        var devs = AudioObjectPropertyAddress(mSelector: kAudioHardwarePropertyDevices, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        AudioObjectAddPropertyListenerBlock(AudioObjectID(kAudioObjectSystemObject), &devs, DispatchQueue.main) { [weak self] _, _ in
            self?.deviceID = 0
            self?.onRateChanged?()
        }
    }

    private func uid(_ id: AudioObjectID) -> String? {
        var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyDeviceUID, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var cf: Unmanaged<CFString>?
        var size = UInt32(MemoryLayout<Unmanaged<CFString>?>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &cf) == noErr, let s = cf else { return nil }
        return s.takeRetainedValue() as String
    }

    private func device() -> AudioObjectID? {
        if deviceID != 0 { return deviceID }
        var a = AudioObjectPropertyAddress(mSelector: kAudioHardwarePropertyDevices, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var size: UInt32 = 0
        guard AudioObjectGetPropertyDataSize(AudioObjectID(kAudioObjectSystemObject), &a, 0, nil, &size) == noErr, size > 0 else { return nil }
        var ids = [AudioObjectID](repeating: 0, count: Int(size) / MemoryLayout<AudioObjectID>.size)
        guard AudioObjectGetPropertyData(AudioObjectID(kAudioObjectSystemObject), &a, 0, nil, &size, &ids) == noErr else { return nil }
        for id in ids {
            if let u = uid(id)?.lowercased(), u.hasSuffix(guid) {
                deviceID = id
                if listening != id {
                    listening = id
                    var r = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyNominalSampleRate, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
                    AudioObjectAddPropertyListenerBlock(id, &r, DispatchQueue.main) { [weak self] _, _ in self?.onRateChanged?() }
                    var l = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyAvailableNominalSampleRates, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
                    AudioObjectAddPropertyListenerBlock(id, &l, DispatchQueue.main) { [weak self] _, _ in self?.onRateChanged?() }
                }
                return id
            }
        }
        return nil
    }

    var nominalRate: UInt32? {
        guard let id = device() else { return nil }
        var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyNominalSampleRate, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var rate: Float64 = 0
        var size = UInt32(MemoryLayout<Float64>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &rate) == noErr, rate > 0 else { return nil }
        return UInt32(rate.rounded())
    }

    var availableRates: [UInt32] {
        guard let id = device() else { return [] }
        var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyAvailableNominalSampleRates, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var size: UInt32 = 0
        guard AudioObjectGetPropertyDataSize(id, &a, 0, nil, &size) == noErr, size > 0 else { return [] }
        var ranges = [AudioValueRange](repeating: AudioValueRange(), count: Int(size) / MemoryLayout<AudioValueRange>.size)
        guard AudioObjectGetPropertyData(id, &a, 0, nil, &size, &ranges) == noErr else { return [] }
        // The FW1814 publishes discrete rates (minimum == maximum), as the reference's `-supportedSampleRates` list assumes.
        return Array(Set(ranges.map { UInt32($0.mMinimum.rounded()) })).sorted()
    }

    @discardableResult func setNominalRate(_ hz: UInt32) -> Bool {
        guard let id = device() else { return false }
        var a = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyNominalSampleRate, mScope: kAudioObjectPropertyScopeGlobal, mElement: 0)
        var rate = Float64(hz)
        return AudioObjectSetPropertyData(id, &a, 0, nil, UInt32(MemoryLayout<Float64>.size), &rate) == noErr
    }
}

final class FWDevice: PanelBackend {
    let service: io_service_t
    let entryID: UInt64
    private(set) var connection: io_connect_t = 0
    var onPropertiesChanged: (() -> Void)?
    private(set) lazy var rate: RateBackend? = HALRate(guid: info.guid)

    private var notifyPort: IONotificationPortRef?
    private var cfPort: CFMachPort?
    private var runLoopSource: CFRunLoopSource?
    private var peakAddress: mach_vm_address_t = 0
    private var peakLength: mach_vm_size_t = 0

    static let peakCount = 52       // 38 hardware + 14 SW-return slots (A2-P9 Correction); the page is one 4 KiB page

    init?(service: io_service_t) {
        var id: UInt64 = 0
        guard IORegistryEntryGetRegistryEntryID(service, &id) == KERN_SUCCESS else { return nil }
        IOObjectRetain(service)
        self.service = service
        self.entryID = id
    }

    deinit {
        close()
        IOObjectRelease(service)
    }

    // MARK: identity

    var info: FWDeviceInfo {
        var guid = ""
        // GUID lives on the FireWire nub above the device: search the parents too (reference reads the registry "GUID" key, A2-P3).
        if let g = IORegistryEntrySearchCFProperty(service, kIOServicePlane, "GUID" as CFString, kCFAllocatorDefault,
                                                   IOOptionBits(kIORegistryIterateRecursively | kIORegistryIterateParents)) {
            if let n = g as? NSNumber { guid = String(format: "0x%llX", n.uint64Value) }
            else if let s = g as? String { guid = s }
        }
        let name = (copyProperty(MAKeys.actualProductName.name) as? String) ?? "FireWire 1814"
        return FWDeviceInfo(entryID: entryID, guid: guid, name: name)
    }

    // MARK: registry properties (the whole control plane)

    func copyProperty(_ key: String) -> Any? {
        guard let v = IORegistryEntryCreateCFProperty(service, key as CFString, kCFAllocatorDefault, 0) else { return nil }
        return v.takeRetainedValue()
    }

    /// The registry table holds only the static keys; the mixer/clock keys live in the kext's property dictionary and are served one by one
    /// by its copyProperty override, or all at once under "MAMetaAllProperties" (A2-P3; seen live on 10.13.6, 2026-10-16: 8 keys in the
    /// table, 36 in MAMetaAllProperties). The reference panel seeds its cache from that umbrella key, so merge both.
    func copyProperties() -> [String: Any] {
        var out: [String: Any] = [:]
        var props: Unmanaged<CFMutableDictionary>?
        if IORegistryEntryCreateCFProperties(service, &props, kCFAllocatorDefault, 0) == KERN_SUCCESS, let p = props,
           let table = p.takeRetainedValue() as NSDictionary as? [String: Any] {
            out = table
        }
        if let all = copyProperty(MAKeys.allProperties.name) as? [String: Any] {
            for (k, v) in all { out[k] = v }
        }
        return out
    }

    /// One call carries every changed key (the reference batches in SendQueuedProperties, A2-P3).
    @discardableResult
    func setProperties(_ properties: [String: Any]) -> kern_return_t {
        if properties.isEmpty { return KERN_SUCCESS }
        return IORegistryEntrySetCFProperties(service, properties as CFDictionary)
    }

    // MARK: user client, notifications, meters

    /// IOServiceOpen type 0 (the reference passes kUserClient_FWKIPC), then the notification port (type 0, refCon 0).
    func open() -> Bool {
        if connection != 0 { return true }
        var conn: io_connect_t = 0
        guard IOServiceOpen(service, mach_task_self_, 0, &conn) == KERN_SUCCESS else { return false }
        connection = conn
        installNotificationPort()
        _ = mapMeters()
        return true
    }

    func close() {
        if let s = runLoopSource { CFRunLoopRemoveSource(CFRunLoopGetMain(), s, CFRunLoopMode.defaultMode) }
        runLoopSource = nil
        if let p = cfPort { CFMachPortInvalidate(p) }
        cfPort = nil
        if let np = notifyPort { IONotificationPortDestroy(np) }
        notifyPort = nil
        if connection != 0 {
            if peakAddress != 0 { _ = IOConnectUnmapMemory64(connection, 1, mach_task_self_, peakAddress) }
            peakAddress = 0
            IOServiceClose(connection)
            connection = 0
        }
    }

    private func installNotificationPort() {
        guard let np = IONotificationPortCreate(kIOMasterPortDefault) else { return }
        notifyPort = np
        let machPort = IONotificationPortGetMachPort(np)
        var ctx = CFMachPortContext(version: 0, info: Unmanaged.passUnretained(self).toOpaque(),
                                    retain: nil, release: nil, copyDescription: nil)
        let callback: CFMachPortCallBack = { _, msg, size, info in
            guard let msg = msg, let info = info, size >= 0x1c else { return }
            let type = msg.load(fromByteOffset: 0x18, as: UInt32.self)
            if type == 0 {                                   // 0 = property dictionary changed (A2-P2); other types are dropped
                let dev = Unmanaged<FWDevice>.fromOpaque(info).takeUnretainedValue()
                dev.onPropertiesChanged?()
            }
        }
        guard let port = CFMachPortCreateWithPort(kCFAllocatorDefault, machPort, callback, &ctx, nil) else { return }
        cfPort = port
        runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, port, 0)
        CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, CFRunLoopMode.defaultMode)
        _ = IOConnectSetNotificationPort(connection, 0, machPort, 0)
    }

    private func mapMeters() -> Bool {
        guard connection != 0 else { return false }
        var addr: mach_vm_address_t = 0
        var len: mach_vm_size_t = 0
        guard IOConnectMapMemory64(connection, 1, mach_task_self_, &addr, &len, 0x1001) == KERN_SUCCESS, addr != 0 else { return false }
        peakAddress = addr
        peakLength = len
        return true
    }

    /// Raw u32 peaks as published by the kext, or nil when the page is not mapped. Torn reads are accepted (reference, A2-P9 §8).
    func readPeaks() -> [UInt32]? {
        guard peakAddress != 0, peakLength >= UInt64(FWDevice.peakCount * 4),
              let p = UnsafePointer<UInt32>(bitPattern: UInt(peakAddress)) else { return nil }
        return (0..<FWDevice.peakCount).map { p[$0] }
    }
}

/// Peak page -> dB exactly as `CXDevice::GetHWSetting` cookie 0x5e9/0x5f3 (A2-P9 §7.5, constants read from the panel binary 2026-10-16):
/// raw 0 -> -64; else ceil(20 * log10(raw * 2^-15)) (data at 0xf8b40 = 2^-15, 0xf8b48 = 20.0). raw 32768 (full-scale 16-bit peak) = 0 dB.
func peakToDB(_ raw: UInt32) -> Int {
    if raw == 0 { return -64 }
    let v = ceil(20.0 * log10(Double(raw) * 0.000030517578125))
    return Int(max(-64.0, v))     // the reference does not clamp non-zero values; clamping only affects values below -64 dB
}

/// Slot of each meter in the shared page (A2-P13 §6): input channel n -> n-1, output channel n -> 18+n-1, SW-return k -> 38+k-1.
enum PeakSlot {
    static func input(_ channel: Int) -> Int { return channel - 1 }
    static func output(_ channel: Int) -> Int { return 18 + channel - 1 }
    static func swReturn(_ k: Int) -> Int { return 38 + k - 1 }
    /// The 14 software-return slots are 4 analog, 2 S/PDIF (k = 5, 6), 8 ADAT (k = 7...14): the kext's peak maps for the output formats
    /// (FWSamples.cpp kPeakMap6 / kPeakMap8 / kPeakMap12, ref SwitchDigitalSignals @0xe5dc) put ADAT channel n at slot 6 + n, the same shape as the
    /// 18 inputs (8 + 2 + 8) and the output slots. Hardware check 2026-10-17: audio sent to ADAT out 1-6 lit strips 3-8 when the panel used 4 + n.
    static func swReturnADAT(_ n: Int) -> Int { return swReturn(6 + n) }
}

/// Device list with hot-plug (A2-P1): publish and terminate notifications on the main run loop.
final class FWDeviceList {
    private(set) var devices: [FWDevice] = []
    var onChange: (() -> Void)?

    private var port: IONotificationPortRef?
    private var publishIter: io_iterator_t = 0
    private var terminateIter: io_iterator_t = 0

    func start() {
        guard let np = IONotificationPortCreate(kIOMasterPortDefault) else { return }
        port = np
        CFRunLoopAddSource(CFRunLoopGetMain(), IONotificationPortGetRunLoopSource(np).takeUnretainedValue(), CFRunLoopMode.defaultMode)
        let me = Unmanaged.passUnretained(self).toOpaque()

        let published: IOServiceMatchingCallback = { ref, iter in
            guard let ref = ref else { return }
            Unmanaged<FWDeviceList>.fromOpaque(ref).takeUnretainedValue().drain(iter, added: true)
        }
        let terminated: IOServiceMatchingCallback = { ref, iter in
            guard let ref = ref else { return }
            Unmanaged<FWDeviceList>.fromOpaque(ref).takeUnretainedValue().drain(iter, added: false)
        }
        // IOServiceAddMatchingNotification consumes one reference of the matching dictionary per call.
        if IOServiceAddMatchingNotification(np, kIOPublishNotification, IOServiceMatching(kFWDeviceClassName), published, me, &publishIter) == KERN_SUCCESS {
            drain(publishIter, added: true)
        }
        if IOServiceAddMatchingNotification(np, kIOTerminatedNotification, IOServiceMatching(kFWDeviceClassName), terminated, me, &terminateIter) == KERN_SUCCESS {
            drain(terminateIter, added: false)
        }
    }

    func stop() {
        if publishIter != 0 { IOObjectRelease(publishIter); publishIter = 0 }
        if terminateIter != 0 { IOObjectRelease(terminateIter); terminateIter = 0 }
        if let np = port { IONotificationPortDestroy(np); port = nil }
        devices.removeAll()
    }

    fileprivate func drain(_ iter: io_iterator_t, added: Bool) {
        var changed = false
        while case let svc = IOIteratorNext(iter), svc != 0 {
            var id: UInt64 = 0
            if IORegistryEntryGetRegistryEntryID(svc, &id) == KERN_SUCCESS {
                if added {
                    if !devices.contains(where: { $0.entryID == id }), let d = FWDevice(service: svc) {
                        _ = d.open()
                        devices.append(d)
                        changed = true
                    }
                } else if let i = devices.index(where: { $0.entryID == id }) {
                    devices[i].close()
                    devices.remove(at: i)
                    changed = true
                }
            }
            IOObjectRelease(svc)
        }
        if changed { onChange?() }
    }
}
