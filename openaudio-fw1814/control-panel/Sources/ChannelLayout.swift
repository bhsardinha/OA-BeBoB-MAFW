// PN.3/PN.5 -- panel state derived from the device, not from widget history: which strips exist and which controls are greyed for a given
// (sample rate, optical in, optical out, S/MUX). Pure logic, no AppKit, so it is unit-testable on the dev Mac.
// Rules: docs/system-map/A2-panel/P12-panel-screens-v19.md (user statements 2026-10-16) and PN1-wireframe.html (approved).
// Device truth: kext/src/FWRates.h FWComposeDeviceChannels: 176.4/192 kHz = 2 in / 4 out analog; 88.2/96 kHz with ADAT = 4 ADAT channels per
// direction whatever the S/MUX bit says; S/MUX (MAClockSource bit 5) is not an input of that table. The reference panel instead greys
// ADAT 5-8 only when the S/MUX box is on and ignores 192 kHz (P12): this layout deliberately follows the device (user decision 2026-10-16).
import Foundation

enum RateClass {
    case base      // 44.1 / 48 kHz
    case double    // 88.2 / 96 kHz
    case quad      // 176.4 / 192 kHz

    init(hz: UInt32) {
        switch hz {
        case 88200, 96000: self = .double
        case 176400, 192000: self = .quad
        default: self = .base          // includes the 44.1 default of FWRateLookup
        }
    }
}

/// One mixer input strip (a stereo pair).
struct InputStrip: Equatable {
    enum Kind { case analog, spdif, adat }
    let title: String            // "analog 1/2 in", "adat 5/6 in", "spdif in"
    let kind: Kind
    /// Mixer pair id 1...9 (A2-P13 §1). Analog 1...4; S/PDIF = 5 and ADAT = 6...9 are INFERRED from the routing bit layout (Q-MAMAP row 24).
    let pair: Int
    let enabled: Bool            // false = greyed (ADAT channels 5-8 at 88.2/96 kHz)
}

struct MeterLabel: Equatable {
    let title: String
    let enabled: Bool
}

struct ChannelLayout: Equatable {
    let rate: UInt32
    let adatIn: Bool
    let adatOut: Bool
    let smuxStored: Bool         // the stored MAClockSource bit 5

    init(rate: UInt32, adatIn: Bool, adatOut: Bool, smuxStored: Bool) {
        self.rate = rate; self.adatIn = adatIn; self.adatOut = adatOut; self.smuxStored = smuxStored
    }

    init(rate: UInt32, clock: MAClockSource) {
        self.init(rate: rate, adatIn: clock.adatIn, adatOut: clock.adatOut, smuxStored: clock.smux)
    }

    var rateClass: RateClass { return RateClass(hz: rate) }

    /// At 176.4/192 kHz every digital port is off (2 in / 4 out analog).
    var digitalAvailable: Bool { return rateClass != .quad }

    // MARK: hardware tab
    var portCardEnabled: Bool { return digitalAvailable }
    /// spdif input port (optical / coaxial) is greyed when input = ADAT (user, P12 item 2) and at 192 kHz.
    var spdifPortEnabled: Bool { return digitalAvailable && !adatIn }
    var externalDigitalSyncEnabled: Bool { return digitalAvailable }
    /// S/MUX applies only to ADAT. It is user-set at 44.1/48 kHz, automatic at 88.2/96 kHz, and greyed (keeping its state) with output
    /// S/PDIF or at 192 kHz (user, 2026-10-16; the output-S/PDIF rule disagrees with P5 and is flagged in P12).
    var smuxEnabled: Bool { return rateClass == .base && adatOut }
    var smuxShownChecked: Bool { return smuxStored || (rateClass == .double && (adatIn || adatOut)) }
    /// The value the panel should write for bit 5: forced on at 88.2/96 kHz with ADAT, otherwise what the user set.
    var smuxWanted: Bool { return smuxShownChecked }

    // MARK: strips
    /// ADAT channels 5-8 are not available at 88.2/96 kHz (4 channels per direction).
    private var adatHalf: Bool { return rateClass == .double }

    var inputStrips: [InputStrip] {
        var s: [InputStrip] = [
            InputStrip(title: "analog 1/2 in", kind: .analog, pair: 1, enabled: true)
        ]
        if rateClass == .quad { return s }               // 192 kHz: analog 1/2 only (2 in)
        s += [
            InputStrip(title: "analog 3/4 in", kind: .analog, pair: 2, enabled: true),
            InputStrip(title: "analog 5/6 in", kind: .analog, pair: 3, enabled: true),
            InputStrip(title: "analog 7/8 in", kind: .analog, pair: 4, enabled: true),
        ]
        if adatIn {
            s += [
                InputStrip(title: "adat 1/2 in", kind: .adat, pair: 6, enabled: true),
                InputStrip(title: "adat 3/4 in", kind: .adat, pair: 7, enabled: true),
                InputStrip(title: "adat 5/6 in", kind: .adat, pair: 8, enabled: !adatHalf),
                InputStrip(title: "adat 7/8 in", kind: .adat, pair: 9, enabled: !adatHalf),
            ]
        } else {
            s.append(InputStrip(title: "spdif in", kind: .spdif, pair: 5, enabled: true))
        }
        return s
    }

    enum ReturnBus { case none, adat, spdif }
    /// The bus strip after the two sw rtn strips (mixer) / after 3/4 out (output): follows optical OUT; gone at 192 kHz.
    var returnBus: ReturnBus {
        if !digitalAvailable { return .none }
        return adatOut ? .adat : .spdif
    }
    /// ADAT bus channels 5-8 greyed at 88.2/96 kHz.
    var adatBusHalfGreyed: Bool { return adatOut && adatHalf }
    /// Output pair of the digital output strip: S/PDIF = 3; ADAT = 4...7 (A2-P13 §1, order consistent with the kext peak maps).
    static let spdifOutputPair = 3

    /// Labels of the "input bus" meter row on the output tab, with greyed flags.
    var inputBusMeters: [MeterLabel] {
        var m: [MeterLabel] = [MeterLabel(title: "1/2", enabled: true)]
        if rateClass == .quad { return m }
        m += [MeterLabel(title: "3/4", enabled: true), MeterLabel(title: "5/6", enabled: true), MeterLabel(title: "7/8", enabled: true)]
        if adatIn {
            m += [MeterLabel(title: "adt 1/2", enabled: true), MeterLabel(title: "adt 3/4", enabled: true),
                  MeterLabel(title: "adt 5/6", enabled: !adatHalf), MeterLabel(title: "adt 7/8", enabled: !adatHalf)]
        } else {
            m.append(MeterLabel(title: "spdif", enabled: true))
        }
        return m
    }

    /// Output strip pairs on the output tab: 1/2 and 3/4 always; digital strip when available.
    var hasDigitalOutputStrip: Bool { return digitalAvailable }
}
