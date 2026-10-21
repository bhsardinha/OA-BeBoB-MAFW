// Plan v8 AP-5.5 / AP-5.6 (user, 2026-10-21; panel-only clone features) -- the parts of the status menu and the big meter that are pure logic,
// kept free of AppKit for the logic tests: the status icon's state, which preset files fit a unit, and the big meter's dB scale.
import Foundation
import CoreGraphics

/// What the menu-bar icon shows for the selected unit (user: online / muted / no sync; no clip indicator).
struct StatusIconState: Equatable {
    var online: Bool          // a unit is selected
    var muted: Bool           // MAGlobalMute
    var noSync: Bool          // MARateLocked == 0

    static let offline = StatusIconState(online: false, muted: false, noSync: false)

    static func of(_ m: DeviceModel?) -> StatusIconState {
        guard let m = m else { return .offline }
        return StatusIconState(online: true, muted: (m.number(MAKeys.globalMute) ?? 0) != 0, noSync: !m.rateLocked)
    }
}

extension DeviceModel {
    /// The part of a preset file this unit can take: keys it knows, and arrays only with this unit's element count (a preset of another model
    /// is never written with the wrong shape). Empty when nothing fits.
    func fittingPreset(_ values: [String: Any]) -> [String: Any] {
        var out: [String: Any] = [:]
        for k in DeviceModel.presetKeys {
            guard let v = values[k.name] else { continue }
            if let a = v as? [Any] {
                let cur = ints(k)
                if cur.isEmpty || a.count != cur.count { continue }
            } else if number(k) == nil {
                continue
            }
            out[k.name] = v
        }
        return out
    }

    /// Main outputs (channels 1 ... mainOutputs of MAOutputLevels: outputs without phones and aux out) and their loudest fader level.
    func mainOutputChannels(_ counts: TranslatorCounts) -> [Int] {
        let n = min(counts.mainOutputs, ints(MAKeys.outputLevels).count)
        return n > 0 ? Array(1...n) : []
    }
    func loudest(_ key: MAKey, channels: [Int]) -> Int? {
        return channels.map { faderLevel(key, channel: $0) }.max()
    }
}

/// The big meter's scale (AP-5.6): dBFS -60 ... 0, dB-linear, marks every 3 dB, labels every 6 dB; raw peaks as the meter page holds them.
enum BigMeterScale {
    static let floorDB = -60.0
    static let markStep = 3, labelStep = 6

    /// Raw u32 peak -> dBFS (raw 32768 = 0 dBFS, the meter page's full scale: Translator.peakToDB); 0 -> -infinity.
    static func db(raw: UInt32) -> Double {
        return raw == 0 ? -Double.infinity : 20 * log10(Double(raw) / 32768.0)
    }
    /// Fraction of the bar length for a level (0 at -60 dBFS and below, 1 at 0 dBFS and above).
    static func fraction(_ db: Double) -> CGFloat {
        if !db.isFinite || db <= floorDB { return 0 }
        return CGFloat(min(1, (db - floorDB) / -floorDB))
    }
    static var marks: [Int] { return Array(stride(from: Int(floorDB), through: 0, by: markStep)) }
    /// LED segments (user, 2026-10-21: segmented like an old stereo deck): 80 over 60 dB = 0.75 dB each, so a 3 dB mark falls every 4.
    static let segments = 80
    /// Lit segments for a level: segment i (0-based) lights once the level is above its lower edge.
    static func litSegments(_ db: Double) -> Int { return Int((fraction(db) * CGFloat(segments)).rounded(.up)) }
    /// Colour zone of segment i: 0 = up to -18 dBFS, 1 = -18 ... -6, 2 = above -6.
    static func zone(segment i: Int) -> Int {
        func first(above db: Double) -> Int { return Int((fraction(db) * CGFloat(segments)).rounded()) }   // whole segments: no float edge
        return i >= first(above: -6) ? 2 : (i >= first(above: -18) ? 1 : 0)
    }
    static var labels: [Int] { return Array(stride(from: Int(floorDB), through: 0, by: labelStep)) }
}
