// PN.3/PN.5 -- the device state the panel's layout depends on: sample rate and the MAClockSource bits (generic; every model has a rate and a
// clock word). What a model builds from it (strips, greying) is in its profile (Sources/Models/<Model>/); the FW1814's rules are in
// Models/FW1814/FW1814Layout.swift. Pure logic, no AppKit, so it is unit-testable.
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
}
