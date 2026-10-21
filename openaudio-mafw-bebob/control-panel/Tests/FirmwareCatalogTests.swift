// Host test of the firmware catalog and verdicts (AP-9.1). Expected values: _vendor/firmware-1.11.2/README.md (build dates), A4 H4 (name rule),
// captures 12 (FW1814) and 89 (Audiophile) for the recorded info blocks.
import Foundation

var fails = 0
func check(_ c: Bool, _ what: String) { print((c ? "OK   " : "FAIL ") + what); if !c { fails += 1 } }

check(FirmwareCatalog.all.count == 8, "8 official images")
check(FirmwareCatalog.model(forProductName: "FW 1814")?.file == "fw1814.bcd", "runtime name FW 1814 -> fw1814.bcd")
check(FirmwareCatalog.model(forProductName: "FW 1814 Bootloader")?.file == "fw1814.bcd", "bootloader name strips 'Bootloader'")
check(FirmwareCatalog.model(forProductName: "FW Audiophile Bootloader")?.build == "20070504102212", "Audiophile build 2007-05-04 10:22:12")
check(FirmwareCatalog.model(forProductName: "FW Bootloader")?.file == "FW410.bcd", "'FW Bootloader' -> FW 410 (kDeviceSpecs row 1)")
check(FirmwareCatalog.model(forProductName: "FWSolo Bootloader")?.file == "FWSolo.bcd", "'FWSolo Bootloader' -> FW Solo")
check(FirmwareCatalog.model(forProductName: "ProFire Lightbridge Bootloader")?.file == "fwadat.bcd", "Lightbridge")
check(FirmwareCatalog.model(forProductName: "Something Else") == nil, "unknown product")
let o1814 = FirmwareCatalog.model(forProductName: "FW 1814")
check(FirmwareCatalog.status(firmwareDate: "20070713", firmwareTime: "080440", official: o1814) == .current, "bench FW1814 (capture 12) is current")
check(FirmwareCatalog.status(firmwareDate: "20070504", firmwareTime: "102212", official: FirmwareCatalog.model(forProductName: "FW Audiophile")) == .current, "bench Audiophile (capture 89) is current")
check(FirmwareCatalog.status(firmwareDate: "20060201", firmwareTime: "120000", official: o1814) == .older, "older build -> older")
check(FirmwareCatalog.status(firmwareDate: "20080101", firmwareTime: "000000", official: o1814) == .newer, "newer build -> newer")
check(FirmwareCatalog.status(firmwareDate: "", firmwareTime: "", official: o1814) == .noInfo, "empty block -> no information")
check(FirmwareCatalog.status(firmwareDate: "20070713", firmwareTime: "080440", official: nil) == .unknownModel, "no official image -> unknown model")
// the recorded Audiophile block through the panel's decoder
let hex = "627269646765436f0100000000000000036c0d00541d21000d000000010000003230303730353034313032323132000060000100ffffff00000008200000180032303033313030363130303034310000"
let d = Data(stride(from: 0, to: hex.count, by: 2).map { i -> UInt8 in let a = hex.index(hex.startIndex, offsetBy: i); return UInt8(hex[a..<hex.index(a, offsetBy: 2)], radix: 16)! })
let fi = FirmwareInfo.parse(d)!
check(fi.firmwareDate == "20070504" && fi.firmwareTime == "102212" && fi.hardwareModel == 13, "capture 89 block decodes to 2007-05-04 10:22:12, model 13")
print(fails == 0 ? "ALL PASS" : "\(fails) FAILED")
exit(fails == 0 ? 0 : 1)
