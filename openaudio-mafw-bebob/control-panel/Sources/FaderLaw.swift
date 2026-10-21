// How a fader maps a level (half-dB steps, 0 = unity) to its travel. Kept free of AppKit for the logic tests.
//   .console  the official panel's curve: most of the travel near unity, for mixing (positions approximated from the screenshots; 64.5 dB
//             stands for the bottom, -129 steps)
//   .monitor  plan v7 AP-5.4 (user, 2026-10-20; a clone law, not the reference): dB-linear over 72 dB, level = -72 dB x (1 - p) with p = 0 at
//             the bottom, centre -36 dB; the very bottom is off (-255). Used by the output-tab strips (outputs, aux out, phones).
import Foundation
import CoreGraphics

enum FaderLaw {
    case console, monitor

    // (dB below 0, fraction of the track from the top)
    static let consoleCurve: [(db: Double, frac: CGFloat)] = [(0, 0), (2, 0.26), (5, 0.42), (10, 0.55), (20, 0.69), (30, 0.76), (50, 0.90), (64.5, 1)]
    static let consoleLabels = [0, 2, 5, 10, 20, 30, 50]
    static let monitorSpan = 144                       // half-dB steps (72 dB)
    static let off = -255                              // level domain minimum (MaKeys MARange.minimum)

    /// Fraction of the track from the top.
    func fraction(forValue v: Int) -> CGFloat {
        switch self {
        case .console:
            let c = FaderLaw.consoleCurve
            let db = min(64.5, max(0, -Double(v) / 2))
            for i in 1..<c.count where db <= c[i].db {
                let a = c[i - 1], b = c[i]
                return a.frac + (b.frac - a.frac) * CGFloat((db - a.db) / (b.db - a.db))
            }
            return 1
        case .monitor:
            return v < -FaderLaw.monitorSpan ? 1 : CGFloat(min(0, v)) / CGFloat(-FaderLaw.monitorSpan)
        }
    }

    func value(forFraction f: CGFloat) -> Int {
        let fr = min(1, max(0, f))
        switch self {
        case .console:
            let c = FaderLaw.consoleCurve
            for i in 1..<c.count where fr <= c[i].frac {
                let a = c[i - 1], b = c[i]
                let db = a.db + (b.db - a.db) * Double((fr - a.frac) / (b.frac - a.frac))
                return Int((-db * 2).rounded())
            }
            return MARange.levelMin
        case .monitor:
            return fr >= 1 ? FaderLaw.off : Int((-CGFloat(FaderLaw.monitorSpan) * fr).rounded())
        }
    }

    /// Scale labels in dB below 0 (monitor: uniform, every 6 dB).
    var labels: [Int] {
        switch self {
        case .console: return FaderLaw.consoleLabels
        case .monitor: return Array(stride(from: 0, through: FaderLaw.monitorSpan / 2, by: 6))
        }
    }
}
