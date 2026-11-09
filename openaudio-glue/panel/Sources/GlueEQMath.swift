// OpenAudio Glue app - the EQ's response for the graph, with the same formulas as dsp/GlueEQ.cpp (RBJ Audio-EQ-Cookbook: low shelf
// and high shelf with slope 1, peaking EQ with proportional Q = 0.5 + 0.1 * |gain dB|), so the curve shows what the filter does.
// Host-tested against values computed by the C++ (Tests/GlueTests.swift). Pure logic.
import Foundation

enum GlueEQMath {
    struct Range { let minHz: Double, maxHz: Double, defaultHz: Double }
    static let ranges = [Range(minHz: 30, maxHz: 600, defaultHz: 100),       // low shelf      (kEQBandRanges)
                         Range(minHz: 60, maxHz: 2000, defaultHz: 400),      // low-mid
                         Range(minHz: 500, maxHz: 12000, defaultHz: 2500),   // high-mid
                         Range(minHz: 1500, maxHz: 16000, defaultHz: 8000)]  // high shelf
    static let bandNames = ["Low", "Low-mid", "High-mid", "High"]
    static let maxGainDb = 12.0
    static let shelfSlope = 1.0

    static func proportionalQ(_ gainDb: Double) -> Double { return 0.5 + 0.1 * abs(gainDb) }

    static func clamp(band: Int, freq: Double, gain: Double, rate: Double) -> (Double, Double) {
        var f = min(max(freq, ranges[band].minHz), ranges[band].maxHz)
        if f > 0.45 * rate { f = 0.45 * rate }
        return (f, min(max(gain, -maxGainDb), maxGainDb))
    }

    /// Normalised biquad coefficients (b0, b1, b2, a1, a2) of one band.
    static func coefficients(band: Int, freq: Double, gain: Double, rate: Double) -> (Double, Double, Double, Double, Double) {
        let (hz, db) = clamp(band: band, freq: freq, gain: gain, rate: rate)
        let A = pow(10.0, db / 40.0)
        let w0 = 2.0 * Double.pi * hz / rate
        let cw = cos(w0), sw = sin(w0)
        var b0: Double, b1: Double, b2: Double, a0: Double, a1: Double, a2: Double
        if band == 0 || band == 3 {
            let alpha = sw / 2.0 * sqrt((A + 1.0 / A) * (1.0 / shelfSlope - 1.0) + 2.0)
            let k = 2.0 * sqrt(A) * alpha
            if band == 0 {
                b0 = A * ((A + 1) - (A - 1) * cw + k)
                b1 = 2 * A * ((A - 1) - (A + 1) * cw)
                b2 = A * ((A + 1) - (A - 1) * cw - k)
                a0 = (A + 1) + (A - 1) * cw + k
                a1 = -2 * ((A - 1) + (A + 1) * cw)
                a2 = (A + 1) + (A - 1) * cw - k
            } else {
                b0 = A * ((A + 1) + (A - 1) * cw + k)
                b1 = -2 * A * ((A - 1) + (A + 1) * cw)
                b2 = A * ((A + 1) + (A - 1) * cw - k)
                a0 = (A + 1) - (A - 1) * cw + k
                a1 = 2 * ((A - 1) - (A + 1) * cw)
                a2 = (A + 1) - (A - 1) * cw - k
            }
        } else {
            let alpha = sw / (2.0 * proportionalQ(db))
            b0 = 1 + alpha * A
            b1 = -2 * cw
            b2 = 1 - alpha * A
            a0 = 1 + alpha / A
            a1 = -2 * cw
            a2 = 1 - alpha / A
        }
        return (b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0)
    }

    /// |H| in dB of one band at `at` Hz.
    static func bandMagnitudeDb(band: Int, freq: Double, gain: Double, rate: Double, at: Double) -> Double {
        let (b0, b1, b2, a1, a2) = coefficients(band: band, freq: freq, gain: gain, rate: rate)
        let w = 2.0 * Double.pi * at / rate
        let nr = b0 + b1 * cos(w) + b2 * cos(2 * w), ni = -(b1 * sin(w) + b2 * sin(2 * w))
        let dr = 1.0 + a1 * cos(w) + a2 * cos(2 * w), di = -(a1 * sin(w) + a2 * sin(2 * w))
        return 10.0 * log10((nr * nr + ni * ni) / (dr * dr + di * di))
    }

    /// The whole EQ's response (bands at 0 dB are skipped, as the filter skips them).
    static func magnitudeDb(bands: [GlueBand], rate: Double, at: Double) -> Double {
        var sum = 0.0
        for (i, b) in bands.enumerated() where b.gain != 0 { sum += bandMagnitudeDb(band: i, freq: b.freq, gain: b.gain, rate: rate, at: at) }
        return sum
    }
}
