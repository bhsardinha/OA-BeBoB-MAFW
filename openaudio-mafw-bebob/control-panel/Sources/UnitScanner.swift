// AP-9.1 -- read-only discovery of the attached units: the driver's nub (com_openaudio_FWMetaNub, one per M-Audio BeBoB unit, runtime or
// bootloader) publishes "FireWire Product Name" and MAFirmwareInfo (the unit's 80-byte info block, read over FireWire from 0xFFFF:C8020000 by
// FWMetaNub @0x1cc9e, A1 Q-P3 §3). Only IORegistryEntryCreateCFProperty is used: nothing is opened and nothing is written.
import Foundation
import IOKit

struct UnitInfo {
    let productName: String
    let guid: String
    let bootloader: Bool              // MAFirmwareInfo 0x0C (bootloader version) != 0
    let info: FirmwareInfo?
    let official: OfficialFirmware?
    let driverAttached: Bool          // a device object (generic base com_openaudio_FWAVCAudioDevice) runs on this nub
    var status: FirmwareStatus {
        guard let i = info else { return official == nil ? .unknownModel : .noInfo }
        return FirmwareCatalog.status(firmwareDate: i.firmwareDate, firmwareTime: i.firmwareTime, official: official)
    }
}

enum UnitScanner {
    /// Our driver's nub, and the official M-Audio driver's (same registry contract, A1 Q-P3 §3), so a unit is checked whichever driver runs it.
    static let nubClasses = ["com_openaudio_FWMetaNub", "com_m_audio_FWMetaNub"]
    static let deviceClasses = ["com_openaudio_FWAVCAudioDevice", "m_audio_b_FWAVCAudioDevice"]

    static func scan() -> [UnitInfo] {
        var out: [UnitInfo] = []
        for cls in nubClasses {
            var it: io_iterator_t = 0
            guard IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(cls), &it) == KERN_SUCCESS else { continue }
            while case let nub = IOIteratorNext(it), nub != 0 {
                out.append(unit(nub))
                IOObjectRelease(nub)
            }
            IOObjectRelease(it)
        }
        return out.sorted { $0.guid < $1.guid }
    }

    private static func prop(_ e: io_registry_entry_t, _ k: String, parents: Bool = false) -> Any? {
        if parents {
            return IORegistryEntrySearchCFProperty(e, kIOServicePlane, k as CFString, kCFAllocatorDefault,
                                                   IOOptionBits(kIORegistryIterateRecursively | kIORegistryIterateParents))
        }
        return IORegistryEntryCreateCFProperty(e, k as CFString, kCFAllocatorDefault, 0)?.takeRetainedValue()
    }

    private static func unit(_ nub: io_registry_entry_t) -> UnitInfo {
        let name = (prop(nub, "FireWire Product Name") as? String) ?? "unknown"
        var guid = ""
        if let g = prop(nub, "GUID", parents: true) as? NSNumber { guid = String(format: "0x%llX", g.uint64Value) }
        let data = prop(nub, "MAFirmwareInfo") as? Data
        var boot = false
        if let d = data, d.count >= 0x10 { boot = d[0x0C] != 0 || d[0x0D] != 0 || d[0x0E] != 0 || d[0x0F] != 0 }
        return UnitInfo(productName: name, guid: guid, bootloader: boot, info: data.flatMap { FirmwareInfo.parse($0) },
                        official: FirmwareCatalog.model(forProductName: name), driverAttached: hasDevice(nub))
    }

    private static func hasDevice(_ nub: io_registry_entry_t) -> Bool {
        var it: io_iterator_t = 0
        guard IORegistryEntryGetChildIterator(nub, kIOServicePlane, &it) == KERN_SUCCESS else { return false }
        defer { IOObjectRelease(it) }
        while case let c = IOIteratorNext(it), c != 0 {
            defer { IOObjectRelease(c) }
            if deviceClasses.contains(where: { IOObjectConformsTo(c, $0) != 0 }) { return true }
        }
        return false
    }

    /// Recorded units for the demo / snapshot mode: the bench FW1814 (capture 12) and Audiophile (capture 89) as the driver read them, and an
    /// invented older FW 410 to show the "older" state.
    static func demo() -> [UnitInfo] {
        func block(_ hex: String) -> Data {
            return Data(stride(from: 0, to: hex.count, by: 2).map { i -> UInt8 in
                let a = hex.index(hex.startIndex, offsetBy: i); return UInt8(hex[a..<hex.index(a, offsetBy: 2)], radix: 16)! })
        }
        let b1814 = block("627269646765436f0100000000000000046c0d00fbb974008300000001000000323030373037313330383034343000000000000000000000000008200000180032303034303333303032353930390000")
        let bAP = block("627269646765436f0100000000000000036c0d00541d21000d000000010000003230303730353034313032323132000060000100ffffff00000008200000180032303033313030363130303034310000")
        var b410 = [UInt8](bAP); let old = Array("20060201120000".utf8)
        for i in 0..<8 { b410[0x20 + i] = old[i] }; for i in 0..<6 { b410[0x28 + i] = old[8 + i] }
        return [UnitInfo(productName: "FW 1814", guid: "0xD6C040074B9FB", bootloader: false, info: FirmwareInfo.parse(b1814),
                         official: FirmwareCatalog.model(forProductName: "FW 1814"), driverAttached: true),
                UnitInfo(productName: "FW Audiophile", guid: "0xD6C0300211D54", bootloader: false, info: FirmwareInfo.parse(bAP),
                         official: FirmwareCatalog.model(forProductName: "FW Audiophile"), driverAttached: true),
                UnitInfo(productName: "FW 410 (demo, invented)", guid: "0xD6C0100000000", bootloader: false, info: FirmwareInfo.parse(Data(b410)),
                         official: FirmwareCatalog.model(forProductName: "FW 410"), driverAttached: true)]
    }
}
