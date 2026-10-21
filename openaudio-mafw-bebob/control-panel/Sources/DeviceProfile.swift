// AP-5.1 -- the per-model layer of the panel, shaped like the official panel: one generic translator (CXDevice) driven by four per-model counts,
// and per-model settings/views (CFW1814DeviceSettings, CFWDJDeviceSettings for the Audiophile). The tabs render the plans a profile returns;
// model data lives only in Sources/Models/<Model>/. Sources: A2-P13 §1-§6 (CX1814Device translator), A2-PNAP §1-§7 (CXAudiophileDevice).
import AppKit

struct StripPlan: Equatable {
    let title: String
    let pair: Int            // pair inside its group (channels 2p-1, 2p)
    let enabled: Bool
}

/// A meter-only bus strip (the FW1814's digital output / return bus): meters read `slots`, greyed from index `greyedFrom`.
struct BusPlan {
    let title: String
    let numbers: [String]?   // nil = 1...n
    let slots: [Int]
    let greyedFrom: Int
    let mainAuxPair: Int?    // main/aux pill under the strip (MAMonitorAuxOnOutput pair), nil = none
}

struct MeterGroupPlan {
    let title: String
    let labels: [String]
    let slots: [[Int]]       // per label: the two (or more) peak slots it shows
    let enabled: [Bool]
}

struct MixerPlan {
    var pageNote: String?            // replaces the whole page (e.g. the FW1814 at 176.4/192 kHz)
    var returns: [StripPlan]
    var returnBus: BusPlan?
    var inputs: [StripPlan]
    var inputsNote: String?          // placeholder after the inputs
    var bus: [MeterGroupPlan]        // meter row under the strips
}

struct OutputStripPlan {
    let title: String
    let levels: MAKey, mute: MAKey, link: MAKey
    let solo: MAKey?
    let pairInGroup: Int
    let monitorPair: Int?
    let rotary: (group: MARotaryMask.Group, bit: UInt64)?
    let meterSlots: (Int, Int)       // -1 = no meter (floor)
    let phones: Int?                 // 1 = MAHeadphoneSourceV2, 2 = MAHeadphoneSecondSource
    let features: StripFeatures
    let hideLastMon: Bool            // phones: the last source button is not available (FW1814 aux at 176.4/192 kHz)
    let pan: MAKey?                  // per-channel pan pair (aux output: MAAuxOutputPan2, cookie 0x5f5), nil = none
}

struct OutputPlan {
    var mainTitle: String
    var main: [OutputStripPlan]
    var digitalBus: BusPlan?
    var otherTitle: String
    var other: [OutputStripPlan]
    var bus: [MeterGroupPlan]?       // nil = no bus meter panel
}

struct MeterItem {
    let label: String
    let slots: [Int]
    let group: String
    let enabled: Bool
}

struct SyncOption {
    let title: String
    let sync: MAClockSource.Sync
    let enabled: (ChannelLayout) -> Bool
}

struct HardwarePlan {
    let sync: [SyncOption]
    let digitalPorts: Bool           // the FW1814's optical/coaxial/ADAT card
    let abSwitchTitle: String        // radio on the a/b card ("Headphone out 1 A/B")
    let abSheetName: String          // sheet title / heading ("Headphone Out 1")
}

/// One model. The tabs, windows and demo ask it; nothing model-specific lives outside Sources/Models/<Model>/.
protocol DeviceProfile: AnyObject {
    var productName: String { get }          // fallback when the registry has no MAActualProductName
    var windowTitle: String { get }
    var aboutPhoto: String { get }           // resource name
    var aboutTitle: String { get }
    var aboutCaption: String { get }
    var counts: TranslatorCounts { get }
    var defaultRates: [UInt32] { get }       // until CoreAudio lists the device's rates
    /// The official firmware build of the model, "YYYYMMDDhhmmss": the build the official v1.11.2 bundle ships for it. The official driver has no
    /// minimum: its helper re-flashes any unit whose date differs from this one (`isEqual:`, R113); this version does not update firmware.
    /// Compared with the MAFirmwareInfo date/time read from the unit.
    var minimumFirmware: String { get }
    var routeLabels: [String] { get }        // mixer "out" buttons = destination bits 0...n-1
    var phonesSources: [PhonesSource] { get }
    var hardware: HardwarePlan { get }
    func mixer(_ l: ChannelLayout) -> MixerPlan
    func output(_ l: ChannelLayout) -> OutputPlan
    func meters(_ l: ChannelLayout, narrow: Bool) -> [(String, [MeterItem])]
    func hardwareCard(_ ctx: PanelContext) -> HardwareCard?
}

extension DeviceProfile {
    var peaks: PeakMap { return PeakMap(counts: counts) }
    func phonesSource(mask: Int) -> PhonesSource? { return phonesSources.first { Int($0.mask) == mask } }
}

/// Model by the device object's registry class (the kext's per-model classes; the panel matches their generic base).
enum DeviceProfiles {
    static let all: [(cls: String, profile: DeviceProfile)] = [
        ("com_openaudio_FW1814Device", FW1814Profile.shared),
        ("com_openaudio_FWAudiophileDevice", AudiophileProfile.shared),
    ]
    static func profile(forClass cls: String) -> DeviceProfile? { return all.first { $0.cls == cls }?.profile }
    static func profile(named n: String) -> DeviceProfile? {
        switch n.lowercased() {
        case "1814", "fw1814": return FW1814Profile.shared
        case "audiophile", "ap": return AudiophileProfile.shared
        default: return nil
        }
    }
}

/// A model-specific card on the hardware tab (the FW1814's digital-port card). The tab places it; the model owns its controls and rules.
protocol HardwareCard: AnyObject {
    var card: Card { get }
    var onChange: (() -> Void)? { get set }      // a control changed the device: the tab redraws everything
    func layout(width: CGFloat, height: CGFloat)
    func refresh()
}

extension DeviceProfile {
    func hardwareCard(_ ctx: PanelContext) -> HardwareCard? { return nil }
}
