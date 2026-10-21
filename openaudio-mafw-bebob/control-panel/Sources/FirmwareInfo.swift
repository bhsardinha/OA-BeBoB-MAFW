// PN.5 -- decoder of the MAFirmwareInfo block shown on the about tab. The device fields come from the 80-byte MAFirmwareInfo block
// published by the kext (bootloader info register block, A1 Q-P3 §3, FFADO info_register_t): hardware model/revision at 0x18/0x1C,
// firmware date "YYYYMMDD" at 0x20 and time "HHMMSS" at 0x28, bootloader date at 0x40 and time at 0x48, GUID at 0x10. The reference panel
// builds the same strings (cookies 0x5de bootloader, 0x5e0 firmware, 0x5df "model %d version %d", 0x5e1 GUID: A2 decompile 2026-10-16,
// CalendarDateFromFirmwareDate @0x6f2c0). The quadlets are raw wire data (big-endian, the kext does not swap); the decoder prefers
// big-endian and falls back to little-endian when the big-endian reading is implausible -- **unverified until a live read**.
import Foundation

struct FirmwareInfo: Equatable {
    var hardwareModel: UInt32 = 0, hardwareRevision: UInt32 = 0
    var firmwareDate = "", firmwareTime = "", bootloaderDate = "", bootloaderTime = ""
    var guid = ""

    static func parse(_ d: Data) -> FirmwareInfo? {
        guard d.count >= 0x50 else { return nil }
        func ascii(_ off: Int, _ len: Int) -> String {
            let bytes = d.subdata(in: off..<(off + len)).prefix(while: { $0 != 0 })
            return String(bytes: bytes, encoding: .ascii) ?? ""
        }
        func quad(_ off: Int) -> UInt32 {
            let b = [UInt8](d.subdata(in: off..<(off + 4)))
            let be = UInt32(b[0]) << 24 | UInt32(b[1]) << 16 | UInt32(b[2]) << 8 | UInt32(b[3])
            let le = UInt32(b[3]) << 24 | UInt32(b[2]) << 16 | UInt32(b[1]) << 8 | UInt32(b[0])
            return be <= 0xFFFF || le > 0xFFFF ? be : le
        }
        var f = FirmwareInfo()
        f.hardwareModel = quad(0x18); f.hardwareRevision = quad(0x1C)
        f.firmwareDate = ascii(0x20, 8); f.firmwareTime = ascii(0x28, 8)
        f.bootloaderDate = ascii(0x40, 8); f.bootloaderTime = ascii(0x48, 8)
        // m_guid: the live block shows "04 6c 0d 00 fb b9 74 00" for GUID 000d6c040074b9fb: two little-endian quadlets, vendor part first.
        let g = [UInt8](d.subdata(in: 0x10..<0x18))
        let first = UInt32(g[3]) << 24 | UInt32(g[2]) << 16 | UInt32(g[1]) << 8 | UInt32(g[0])
        let second = UInt32(g[7]) << 24 | UInt32(g[6]) << 16 | UInt32(g[5]) << 8 | UInt32(g[4])
        f.guid = String(format: "0x%llX", UInt64(first) << 32 | UInt64(second))      // the screenshot shows "0xD6C040074B9FB"
        return f
    }

    /// "2007-07-13 08:04:40 -0300": the reference parses the strings as local time and prints with the calendar format "%Y-%m-%d %H:%M:%S %z".
    static func display(date: String, time: String) -> String {
        guard date.count == 8, time.count >= 6 else { return "-" }
        let d = Array(date), t = Array(time)
        let tz = TimeZone.current.secondsFromGMT()
        let sign = tz < 0 ? "-" : "+"
        let off = String(format: "%@%02d%02d", sign, abs(tz) / 3600, (abs(tz) % 3600) / 60)
        return "\(String(d[0..<4]))-\(String(d[4..<6]))-\(String(d[6..<8])) \(String(t[0..<2])):\(String(t[2..<4])):\(String(t[4..<6])) \(off)"
    }
}

