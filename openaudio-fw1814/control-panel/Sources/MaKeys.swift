// PN.2 -- typed catalog of the MA* property keys and the MAClockSource composite codec (status: written, parse-checked, not run).
// Spec: docs/system-map/A2-panel/PN2-control-key-map.md (control -> key map, approved wireframe PN1-wireframe.html)
// Source of every row: docs/system-map/A1-ghidra/Q-MAMAP-key-crosswalk.md (addresses are of the reference kext v1.11.2).
// Keys are written through IORegistryEntrySetCFProperties on the device registry entry, not IOConnectCallMethod (B5 §10).
import Foundation

/// How a key's value is shaped in the registry (CF types).
enum MAValueKind {
    case levels(count: Int)        // CFArray of signed ints, half-dB steps in [-255, 0] (Q-MAMAP §1.4)
    case boolArray(count: Int)     // CFArray of 0/1
    case routing(count: Int)       // CFArray of bitmasks (see PN2 §4)
    case scalar32                  // CFNumber u32
    case mask64                    // CFNumber u64
    case string
    case dictionary
    case function                  // write-only trigger, value ignored
}

enum MAAccess { case readWrite, readOnly, writeOnly }

struct MAKey {
    let name: String
    let kind: MAValueKind
    let access: MAAccess
    let source: String             // Q-MAMAP row / function address
}

enum MAKeys {
    // Channel groups: SW return 4, Output 4, Input 18, Headphone 4, Aux output 2, Aux send 22 (Q-MAMAP §1).
    static let swReturnLevels = MAKey(name: "MASWReturnLevels", kind: .levels(count: 4), access: .readWrite, source: "Q-MAMAP row 1")
    static let swReturnMute   = MAKey(name: "MASWReturnMute",   kind: .boolArray(count: 4), access: .readWrite, source: "row 2")
    static let swReturnSolo   = MAKey(name: "MASWReturnSolo",   kind: .boolArray(count: 4), access: .readWrite, source: "row 3")
    static let swReturnLink   = MAKey(name: "MASWReturnLink",   kind: .boolArray(count: 4), access: .readWrite, source: "row 4")
    static let outputLevels   = MAKey(name: "MAOutputLevels",   kind: .levels(count: 4), access: .readWrite, source: "row 5")
    static let outputMute     = MAKey(name: "MAOutputMute",     kind: .boolArray(count: 4), access: .readWrite, source: "row 6")
    static let outputSolo     = MAKey(name: "MAOutputSolo",     kind: .boolArray(count: 4), access: .readWrite, source: "row 7")
    static let outputLink     = MAKey(name: "MAOutputLink",     kind: .boolArray(count: 4), access: .readWrite, source: "row 8")
    static let inputLevels    = MAKey(name: "MAInputLevels",    kind: .levels(count: 18), access: .readWrite, source: "row 9")
    static let inputMute      = MAKey(name: "MAInputMute",      kind: .boolArray(count: 18), access: .readWrite, source: "row 10")
    static let inputSolo      = MAKey(name: "MAInputSolo",      kind: .boolArray(count: 18), access: .readWrite, source: "row 11")
    static let inputLink      = MAKey(name: "MAInputLink",      kind: .boolArray(count: 18), access: .readWrite, source: "row 12")
    static let headphoneLevels = MAKey(name: "MAHeadphoneLevels", kind: .levels(count: 4), access: .readWrite, source: "row 13")
    static let headphoneMute  = MAKey(name: "MAHeadphoneMute",  kind: .boolArray(count: 4), access: .readWrite, source: "row 14")
    static let headphoneLink  = MAKey(name: "MAHeadphoneLink",  kind: .boolArray(count: 4), access: .readWrite, source: "row 15")
    static let auxSendLevels  = MAKey(name: "MAAuxSendLevels",  kind: .levels(count: 22), access: .readWrite, source: "row 16, default -255")
    static let auxOutputLevels = MAKey(name: "MAAuxOutputLevels", kind: .levels(count: 2), access: .readWrite, source: "row 17")
    static let auxOutputMute  = MAKey(name: "MAAuxOutputMute",  kind: .boolArray(count: 2), access: .readWrite, source: "row 18")
    static let auxOutputLink  = MAKey(name: "MAAuxOutputLink",  kind: .boolArray(count: 2), access: .readWrite, source: "row 19")
    static let auxOutputPan2  = MAKey(name: "MAAuxOutputPan2",  kind: .levels(count: 2), access: .readWrite, source: "row 21, AV/C feature selector 3")
    static let monitorAuxOnOutput = MAKey(name: "MAMonitorAuxOnOutput", kind: .boolArray(count: 3), access: .readWrite, source: "row 22, Reg 0x9c; 3 elements = (output.Count() >> 1) + 1: out pairs 1, 2 and the S/PDIF pair")
    static let inputPan2      = MAKey(name: "MAInputPan2",      kind: .levels(count: 18), access: .readWrite, source: "row 23, Reg 0x40+")
    static let inputRoutingV2 = MAKey(name: "MAInputRoutingV2", kind: .routing(count: 9), access: .readWrite, source: "row 24, Reg 0x90")
    static let swReturnRoutingV2 = MAKey(name: "MASWReturnRoutingV2", kind: .routing(count: 2), access: .readWrite, source: "row 25, AV/C mixer connect")
    static let directMonitoringActive = MAKey(name: "MADirectMonitoringActive", kind: .scalar32, access: .readWrite, source: "row 26 (kext ignores the value)")
    static let globalDim      = MAKey(name: "MAGlobalDim",      kind: .scalar32, access: .readWrite, source: "row 27")
    static let globalMute     = MAKey(name: "MAGlobalMute",     kind: .scalar32, access: .readWrite, source: "row 28")
    static let rotaryControl  = MAKey(name: "MARotaryControlV2", kind: .mask64, access: .readWrite, source: "row 29, default 0xFFFF00000000")
    static let virtualRotateKnob = MAKey(name: "MAMetaVirtualRotateDataKnob", kind: .function, access: .writeOnly, source: "row 30 (function property: the value is the signed detent count)")
    static let momentarySwitchRoutings = MAKey(name: "MAMomentarySwitchRoutings", kind: .routing(count: 4), access: .readWrite, source: "row 31, default {1,2,3,4}")
    static let headphoneSourceV2 = MAKey(name: "MAHeadphoneSourceV2", kind: .scalar32, access: .readWrite, source: "row 32, Reg 0x98")
    static let headphoneSecondSource = MAKey(name: "MAHeadphoneSecondSource", kind: .scalar32, access: .readWrite, source: "row 33")
    static let momentarySwitchSource = MAKey(name: "MAMomentarySwitchSource", kind: .scalar32, access: .readWrite, source: "row 34")
    static let momentarySwitchLatent = MAKey(name: "MAMomentarySwitchLatent", kind: .scalar32, access: .readOnly, source: "row 35 (front-panel A/B state)")
    static let enableStereoPairEngines = MAKey(name: "MAEnableStereoPairEngines", kind: .scalar32, access: .readWrite, source: "row 36")
    static let enableAggregateEngine = MAKey(name: "MAEnableAggregateEngine", kind: .scalar32, access: .readWrite, source: "row 37")
    static let resetToFactory = MAKey(name: "MAMetaResetToFactorySettings", kind: .function, access: .writeOnly, source: "row 38")
    static let deviceUserLabel = MAKey(name: "MADeviceUserLabel", kind: .string, access: .readWrite, source: "row 39")
    static let clockSource    = MAKey(name: "MAClockSource",    kind: .scalar32, access: .readWrite, source: "row 40, default 0x40000003")
    static let sampleRate     = MAKey(name: "MASampleRate",     kind: .scalar32, access: .readWrite, source: "row 41, fast path in setProperties")
    static let rateLocked     = MAKey(name: "MARateLocked",     kind: .scalar32, access: .readOnly, source: "row 42")
    static let allProperties  = MAKey(name: "MAMetaAllProperties", kind: .dictionary, access: .readOnly, source: "row 43")
    // Read-only descriptors (Q-MAMAP §2 footer).
    static let actualProductName = MAKey(name: "MAActualProductName", kind: .string, access: .readOnly, source: "FWMetaNub::start")
    static let iconName       = MAKey(name: "MAIconName", kind: .string, access: .readOnly, source: "descriptor")
    static let mainEngineUniqueID = MAKey(name: "MAMainEngineUniqueID", kind: .string, access: .readOnly, source: "createAudioEngine")
    static let maxBytesPerCycle = MAKey(name: "MAMaxBytesPerCycle", kind: .scalar32, access: .readOnly, source: "initHardware")
    static let bytesPerCycle  = MAKey(name: "MABytesPerCycle", kind: .scalar32, access: .readOnly, source: "FWEngine2::RecalculateBandwidth")
    static let midiInputPorts = MAKey(name: "MAMIDIInputPorts", kind: .dictionary, access: .readOnly, source: "SetMIDIPortProperties")
    static let midiOutputPorts = MAKey(name: "MAMIDIOutputPorts", kind: .dictionary, access: .readOnly, source: "SetMIDIPortProperties")
    static let deviceInitialized = MAKey(name: "MADeviceInitialized", kind: .scalar32, access: .readOnly, source: "FWUserClient::SetInitializedProperty")
    static let firmwareInfo   = MAKey(name: "MAFirmwareInfo", kind: .dictionary, access: .readOnly, source: "80 B descriptor (A1-Q7a); About tab")
    // Firmware write keys belong to the helper daemon, not the panel.
    static let firmwareHalt   = MAKey(name: "MAFirmwareHalt", kind: .function, access: .writeOnly, source: "FWMetaNub (helper only)")
    static let firmwareStart  = MAKey(name: "MAFirmwareStart", kind: .function, access: .writeOnly, source: "FWMetaNub (helper only)")
    static let firmwareUpgradeImage = MAKey(name: "MAFirmwareUpgradeImage", kind: .function, access: .writeOnly, source: "FWMetaNub (helper only)")

    static let all: [MAKey] = [
        swReturnLevels, swReturnMute, swReturnSolo, swReturnLink,
        outputLevels, outputMute, outputSolo, outputLink,
        inputLevels, inputMute, inputSolo, inputLink,
        headphoneLevels, headphoneMute, headphoneLink,
        auxSendLevels, auxOutputLevels, auxOutputMute, auxOutputLink, auxOutputPan2,
        monitorAuxOnOutput, inputPan2, inputRoutingV2, swReturnRoutingV2,
        directMonitoringActive, globalDim, globalMute, rotaryControl, virtualRotateKnob,
        momentarySwitchRoutings, headphoneSourceV2, headphoneSecondSource, momentarySwitchSource, momentarySwitchLatent,
        enableStereoPairEngines, enableAggregateEngine, resetToFactory, deviceUserLabel,
        clockSource, sampleRate, rateLocked, allProperties,
        actualProductName, iconName, mainEngineUniqueID, maxBytesPerCycle, bytesPerCycle,
        midiInputPorts, midiOutputPorts, deviceInitialized, firmwareInfo,
        firmwareHalt, firmwareStart, firmwareUpgradeImage,
    ]
}

/// Level domain shared by every *Levels key: half-dB steps, 0 = unity, -255 = minimum (Q-MAMAP §1.4, capture 62).
enum MALevel {
    static let minimum = -255
    static let unity = 0
    static func clamp(_ v: Int) -> Int { return Swift.max(minimum, Swift.min(unity, v)) }
}

/// MAClockSource is a composite bitfield (memory project_maclocksource_is_composite_bitfield; P-ClockUI-encoding §2/§4).
/// Dispatch by bitmask, never by value. Every write re-applies the gate (bit 30) and clears bit 31.
struct MAClockSource: Equatable {
    var raw: UInt32

    static let factoryDefault = MAClockSource(raw: 0x4000_0003)   // Q3b
    static let gate: UInt32 = 0x4000_0000                         // SetClockSourceInternal compares against 0x3fffffff (@0xe2ea)

    /// Sync source, bits [1:0]. Values from the v1.9 capture (B5 §10); v1.5.2 values are UNVERIFIED (P-ClockUI §2).
    enum Sync: UInt32 {
        case internalUnmuted = 0   // radio 0x1453
        case externalDigital = 1   // radio 0x1454
        case wordClock = 2         // radio 0x1455
        case internalMuted = 3     // radio 0x1452
    }

    var sync: Sync {
        get { return Sync(rawValue: raw & 0x3)! }
        set { raw = (raw & ~UInt32(0x3)) | newValue.rawValue }
    }
    /// Bit 2: S/PDIF input port, 1 = coaxial, 0 = optical (B5 §10; corrects BUILD-MANUAL §2.G).
    var spdifCoaxial: Bool {
        get { return raw & 0x04 != 0 }
        set { raw = newValue ? (raw | 0x04) : (raw & ~UInt32(0x04)) }
    }
    /// Bit 3: optical in is ADAT. Bit 4: optical out is ADAT. Bit 5: ADAT S/MUX.
    var adatIn: Bool {
        get { return raw & 0x08 != 0 }
        set { raw = newValue ? (raw | 0x08) : (raw & ~UInt32(0x08)) }
    }
    var adatOut: Bool {
        get { return raw & 0x10 != 0 }
        set { raw = newValue ? (raw | 0x10) : (raw & ~UInt32(0x10)) }
    }
    var smux: Bool {
        get { return raw & 0x20 != 0 }
        set { raw = newValue ? (raw | 0x20) : (raw & ~UInt32(0x20)) }
    }

    /// Value to write: gate set, bit 31 cleared (CXDevice::ChangeDigitalSourceAttribute @0x71fea).
    var wireValue: UInt32 { return (raw & 0x3fff_ffff) | MAClockSource.gate }
}

/// Panel-side index and range conventions (A2-P13, decompile of the reference panel translator `CXDevice`).
enum MAIndex {
    /// Mixer channel id 1...32: 1...18 inputs, 19...32 SW return 1...14 (ChooseMixerBusAndChannelIndex @0x6de56).
    static func mixerChannel(_ id: Int) -> (group: String, element: Int)? {
        if id >= 1 && id <= 18 { return ("Input", id) }
        if id >= 19 && id <= 32 { return ("SWReturn", id - 18) }
        return nil
    }
    /// Output channel id 1...20: 1...14 output, 15...18 headphone, 19...20 aux output (ChooseOutputBusAndChannelIndex @0x6dcee).
    static func outputChannel(_ id: Int) -> (group: String, element: Int)? {
        if id >= 1 && id <= 14 { return ("Output", id) }
        if id >= 15 && id <= 18 { return ("Headphone", id - 14) }
        if id >= 19 && id <= 20 { return ("AuxOutput", id - 18) }
        return nil
    }
    /// Output pair 1...10: 1...7 output, 8 = phones 1, 9 = phones 2, 10 = aux output. Pair p = channels 2p-1, 2p.
    static let phones1Pair = 8, phones2Pair = 9, auxOutputPair = 10
}

/// MARotaryControlV2 bit layout (A2-P13 §2; preset table read at panel `8CXDevice`+0x10).
enum MARotaryMask {
    static let swReturn: UInt64 = 0x0000_0000_0000_FFFF      // preset 1, pair p -> bit p-1
    static let output: UInt64   = 0x0000_FFFF_0000_0000      // preset 2, pair p -> bit 31+p
    static let input: UInt64    = 0x0000_0000_FFFF_0000      // preset 3, pair p -> bit 15+p
    static let headphone: UInt64 = 0x0300_0000_0000_0000     // preset 4, bits 56, 57
    static let auxOutput: UInt64 = 0x0004_0000_0000_0000     // preset 5, bit 50
    static let factoryDefault: UInt64 = 0x0000_FFFF_0000_0000
    static func inputPairBit(_ pair: Int) -> UInt64 { return 1 << UInt64(15 + pair) }
    static func swReturnPairBit(_ pair: Int) -> UInt64 { return 1 << UInt64(pair - 1) }
    static func outputPairBit(_ pair: Int) -> UInt64 { return 1 << UInt64(31 + pair) }

    /// The encoder group the mask currently addresses (the sidebar radio). Priority output > input > sw return > aux > headphone, so the
    /// factory default (output only) reads as "output". A mask that mixes groups reports the first match.
    enum Group { case swReturn, output, input, aux, headphone }
    static func group(of mask: UInt64) -> Group? {
        if mask & output != 0 { return .output }
        if mask & input != 0 { return .input }
        if mask & swReturn != 0 { return .swReturn }
        if mask & auxOutput != 0 { return .aux }
        if mask & headphone != 0 { return .headphone }
        return nil
    }
    /// Preset written by the sidebar radios (cookie 0x5e5 table, A2-P13 §2).
    static func preset(for g: Group) -> UInt64 {
        switch g {
        case .swReturn: return swReturn
        case .output: return output
        case .input: return input
        case .aux: return auxOutput
        case .headphone: return headphone
        }
    }
}

/// Panel value ranges (CXDevice::GetHWSettingRange @0x6e200). Display = value * 0.5 for pan and levels.
enum MARange {
    static let levelMin = -129, levelMax = 0          // aux send, mixer and output levels, 0.5 dB per step
    static let panMin = -255, panMax = 255            // input pan, aux-out pan, display L127.5 ... R127.5
    static let peakFloorDB = -64                      // peak meters, dB
}

/// Headphone source masks as the kext receives them (translate(v) = v - (v == 4); mask = 1 << (translate - 1)).
enum MAHeadphoneSource: UInt32 { case out12 = 1, out34 = 2, aux = 4 }
