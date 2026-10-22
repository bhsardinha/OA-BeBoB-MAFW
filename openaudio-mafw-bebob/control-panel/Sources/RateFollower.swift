// RATE_FOLLOW_PLAN_v1 (user, 2026-10-22; a clone feature: the reference detects the source's rate but never follows it, capture 98) -- the
// decision logic of "Follow external clock rate", free of AppKit and CoreAudio for the logic tests. The panel controller ticks it every 0.25 s
// and applies the rate it returns through the panel's CoreAudio path (DeviceModel.requestSampleRate, PN.8).
// What the hardware gives (capture 98, FW1814 + POD X3): the kext's MAExternalSyncRate is the source's exact rate while the source is in the
// interface's current speed range (44.1/48 or 88.2/96), 0 outside it; a rate change shows transitional states for up to ~6 s.
import Foundation

struct RateFollowInput {
    var enabled: Bool            // PanelPrefs.followExternalRate
    var external: Bool           // sync source = External - digital (word clock is not followed: unverified)
    var detected: UInt32?        // MAExternalSyncRate (0 = no source seen); nil = the driver does not publish it
    var current: UInt32?         // CoreAudio nominal rate; nil = no HAL device yet
    var available: [UInt32]      // CoreAudio's rates for the device
    var busy: Bool               // kAudioDevicePropertyDeviceIsRunningSomewhere
}

enum RateFollowStatus: Equatable {
    case off                     // setting off, another sync source, no key, no HAL device, or above 96 kHz
    case following               // the source runs at the interface's rate
    case waiting(UInt32)         // the source is at another rate and a client runs the device: follows when playback stops
    case unsupported(UInt32)     // the source's rate is not one the device offers (e.g. 32 kHz)
    case switching(UInt32)       // the follower asked for this rate
    case searching               // trying the other speed range
    case noSource                // nothing seen
}

final class RateFollower {
    static let stableFor: TimeInterval = 1.5         // a reading must hold this long (the 96 -> 88.2 -> 48 steps of a source last ~0.8 s each)
    static let holdOff: TimeInterval = 2             // readings right after a change are ignored
    static let noSourceAfter: TimeInterval = 8       // a lock takes up to ~6 s after a rate change (capture 98, official driver)
    static let quietBeforeTry: TimeInterval = 4      // nothing seen this long on a settled interface: try the other range
    static let changeTimeout: TimeInterval = 12      // CoreAudio did not apply the rate (1 s panel deferral + the change itself)
    static let maxRate: UInt32 = 96000               // digital I/O is off above 96 kHz

    private(set) var status: RateFollowStatus = .off
    private var active = false
    private var seen: UInt32 = 0                     // last detected value
    private var seenSince: TimeInterval = 0
    private var lastChange: TimeInterval = -Double.infinity
    private var target: UInt32?                      // rate asked for, until CoreAudio reports it
    private var probeOrigin: UInt32?                 // rate to go back to while the other speed range is tried
    private var tried = false                        // the one try of this episode was made

    /// The line the panel and the status menu show under the rate, when there is something to say.
    var note: String? {
        switch status {
        case .waiting(let hz): return "Source at \(RateFollower.khz(hz)) kHz - follows when playback stops"
        case .unsupported(let hz): return "Source at \(RateFollower.khz(hz)) kHz - not offered by the interface"
        case .switching(let hz): return "Following the source: \(RateFollower.khz(hz)) kHz"
        case .searching: return "Looking for the source's rate\u{2026}"
        case .off, .following, .noSource: return nil
        }
    }

    static func khz(_ hz: UInt32) -> String {
        return hz % 1000 == 0 ? String(hz / 1000) : String(format: "%.1f", Double(hz) / 1000)
    }

    /// One tick. Returns the rate to set, if any.
    func tick(_ i: RateFollowInput, now: TimeInterval) -> UInt32? {
        guard i.enabled, i.external, let det = i.detected, let cur = i.current, cur <= RateFollower.maxRate else {
            // leaving (setting off, sync source changed, interface gone) re-arms the try for the next time
            active = false; target = nil; probeOrigin = nil; tried = false
            status = .off
            return nil
        }
        if !active { active = true; seen = det; seenSince = now }
        if det != seen { seen = det; seenSince = now }

        if let t = target {
            if cur == t || now - lastChange > RateFollower.changeTimeout { target = nil } else { return nil }
        }
        // a reading counts once it has held for stableFor, measured from the end of the hold-off after a change
        let since = max(seenSince, lastChange + RateFollower.holdOff)
        let held = now - since

        if seen != 0 {
            guard held >= RateFollower.stableFor else { return nil }
            probeOrigin = nil; tried = false                       // a source re-arms the try
            if seen == cur { status = .following; return nil }
            guard seen <= RateFollower.maxRate, i.available.isEmpty || i.available.contains(seen) else { status = .unsupported(seen); return nil }
            if i.busy { status = .waiting(seen); return nil }
            status = .switching(seen)
            return change(to: seen, now: now)
        }

        // nothing seen: after a change the lock may still be coming (noSourceAfter); a source lost on a settled interface is gone (quietBeforeTry)
        let settledLoss = seenSince >= lastChange + RateFollower.holdOff + RateFollower.noSourceAfter
        guard held >= (settledLoss ? RateFollower.quietBeforeTry : RateFollower.noSourceAfter) else {
            if probeOrigin == nil { status = .noSource }
            return nil
        }
        if let o = probeOrigin {                                   // the other range showed nothing either: back to where it started
            probeOrigin = nil; tried = true
            status = .noSource
            return (o != cur && !i.busy) ? change(to: o, now: now) : nil
        }
        if tried || i.busy { status = .noSource; return nil }
        guard let other = RateFollower.otherRange(cur, i.available) else { tried = true; status = .noSource; return nil }
        probeOrigin = cur
        status = .searching
        return change(to: other, now: now)
    }

    private func change(to hz: UInt32, now: TimeInterval) -> UInt32 {
        target = hz; lastChange = now
        return hz
    }

    /// The rate tried in the other speed range: 96 (else 88.2) from 44.1/48, 48 (else 44.1) from 88.2/96.
    static func otherRange(_ cur: UInt32, _ available: [UInt32]) -> UInt32? {
        let order: [UInt32] = cur <= 48000 ? [96000, 88200] : [48000, 44100]
        return order.first { available.isEmpty || available.contains($0) }
    }
}
