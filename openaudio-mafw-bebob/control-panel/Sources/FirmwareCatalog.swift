// AP-9.1 -- the official firmware builds of the M-Audio FireWire driver 1.11.2 (2014), one per model, and the comparison of a unit's firmware
// with them. Pure logic (host-tested). Source: _vendor/firmware-1.11.2/README.md (the 8 images extracted from M-Audio_FireWire_1-11.2.dmg,
// header build date at 0x08, model id at 0x18, SHA-256). Model folders = the kext's kDeviceSpecs runtime product names (ref @0x46450).
import Foundation

struct OfficialFirmware: Equatable {
    let model: String        // folder / runtime "FireWire Product Name"
    let file: String
    let build: String        // "YYYYMMDDhhmmss" from the .bcd header (0x08)
    let modelID: UInt32      // .bcd header 0x18 (0 in the older FW1814 / ProjectMix headers)
    let sha256: String
}

enum FirmwareStatus: Equatable {
    case current             // the unit runs the official 1.11.2 build of its model
    case older               // older than it: the official driver would update it
    case newer               // newer than the 1.11.2 image (the official helper re-flashes on any date mismatch, `isEqual:`, R113)
    case unknownModel        // not one of the 8 models of the official list
    case noInfo              // no readable info block (driver not attached)
}

enum FirmwareCatalog {
    static let release = "M-Audio FireWire 1.11.2 (2014)"
    static let all: [OfficialFirmware] = [
        OfficialFirmware(model: "FW 1814", file: "fw1814.bcd", build: "20070713080440", modelID: 0x00000000, sha256: "974da5e937a7cf59094201a8457230c13f90d57dc1143d3b5d11dfc52870371c"),
        OfficialFirmware(model: "FW 410", file: "FW410.bcd", build: "20070504102656", modelID: 0x00010046, sha256: "3ee6f5973053d4e6bcef68824fd99690c4796d55b9f7731fce0e6d50dd830e02"),
        OfficialFirmware(model: "FW Audiophile", file: "FWAP.bcd", build: "20070504102212", modelID: 0x00010060, sha256: "0c1eb6ee469936efbfcc13d46617e9a310deb650078bcbd66dddf637e6a7d2a8"),
        OfficialFirmware(model: "FW Solo", file: "FWSolo.bcd", build: "20070808135628", modelID: 0x00010062, sha256: "3d6d14710f32372dd02ae2c9e6095d471d1e9d6238d172955e41626d1d310233"),
        OfficialFirmware(model: "NRV-10", file: "nrv10.bcd", build: "20070504103229", modelID: 0x00010081, sha256: "030d821c855fa28de282eecc72e02063d2340c3d677f78c21505b555996674fe"),
        OfficialFirmware(model: "Ozonic", file: "Ozonic.bcd", build: "20050905091022", modelID: 0x0000000a, sha256: "eb49f029a300f5dbc8d3813472c90529c836e810f5cd36ff5a46c014623106a3"),
        OfficialFirmware(model: "ProFire Lightbridge", file: "fwadat.bcd", build: "20060914145430", modelID: 0x000100a1, sha256: "1982e13f155071de4ef18619b8c47ec233ffdeeb106a466a707164fdc54cdec5"),
        OfficialFirmware(model: "ProjectMix", file: "projectMix.bcd", build: "20070713080544", modelID: 0x00000000, sha256: "797f8d91c7c9ac53e9f3d69dee0d952a3339dc6f8f248f130c95ce8b9aa70ad9"),
    ]

    /// The official helper's rule (A4 H4 `+firmwareFilesAtPath:forDevice:`): cut the product name at "Bootloader", then two aliases, else the
    /// name is the folder. Deviation: the reference aliases name folders that its own installer does not have ("FW410", "FWSolo"); here they
    /// point at the installed folders "FW 410" / "FW Solo". Surrounding spaces are trimmed.
    static func model(forProductName name: String) -> OfficialFirmware? {
        var n = name
        if let r = n.range(of: "Bootloader") { n = String(n[..<r.lowerBound]) }
        n = n.trimmingCharacters(in: .whitespaces)
        if n == "FW" { n = "FW 410" }                  // FW 410 bootloader publishes "FW Bootloader" (kDeviceSpecs row 1)
        if n == "FWSolo" { n = "FW Solo" }              // "FWSolo Bootloader" (kDeviceSpecs row 5)
        return all.first { $0.model == n }
    }

    /// Firmware date + time of the unit ("YYYYMMDD" + "hhmmss", MAFirmwareInfo 0x20 / 0x28) against the official build.
    static func status(firmwareDate: String, firmwareTime: String, official: OfficialFirmware?) -> FirmwareStatus {
        guard let o = official else { return .unknownModel }
        let unit = firmwareDate + String(firmwareTime.prefix(6))
        guard unit.count == 14 else { return .noInfo }
        if unit == o.build { return .current }
        return unit < o.build ? .older : .newer
    }

    static func display(_ build: String) -> String {
        let b = Array(build)
        guard b.count == 14 else { return build }
        return "\(String(b[0..<4]))-\(String(b[4..<6]))-\(String(b[6..<8])) \(String(b[8..<10])):\(String(b[10..<12])):\(String(b[12..<14]))"
    }
}
