// OpenAudio Glue app - the settings model: the same plist the driver reads and writes as the Glue device's 'glst' property
// (openaudio-glue/driver/src/GlueSettings.h). Option names and counts mirror dsp/Glue.h and dsp/GlueComp.h. Pure logic (host-tested).
import Foundation

enum GlueOptions {
    static let stageNames = ["Transformer", "Tube", "Desk", "Tape"]
    static let levelNames = ["Off", "Soft", "Medium", "High", "Full"]                          // glue::Level
    static let deskNames = ["Off", "Neve M", "Neve H", "API M", "API H", "SSL M", "SSL H"]     // glue::DeskOption
    static let deskLongNames = ["Off", "Neve Medium", "Neve High", "API Medium", "API High", "SSL Medium", "SSL High"]
    static let sourceNames = ["General", "Drums", "Vocals", "Heavy guitar", "Bass", "Synth"]   // glue::CompSource
    static let deskStage = 2
    static func options(_ stage: Int) -> [String] { return stage == deskStage ? deskNames : levelNames }
    static let maxChannels = 32                                                                  // kGlueTapMaxChannels
    static let thresholdRange: ClosedRange<Double> = -40...0                                    // CompDesign::kMinThresholdDb .. off
}

struct GlueBand: Equatable {
    var freq: Double
    var gain: Double
}

struct GlueChannel: Equatable {
    var on = true
    var threshold = 0.0                        // dB, 0 = compressor off
    var source = 0
    var bands: [GlueBand] = GlueEQMath.ranges.map { GlueBand(freq: $0.defaultHz, gain: 0) }
    var link = false                           // on channel 1, 3, 5 ...: the next channel follows this one (stereo link)
}

struct GlueSettings: Equatable {
    var bypass = false
    var stages = [0, 0, 0, 0]
    var drive = 0.0                            // dB, 0...18: the global Drive (more into every colour stage, level-compensated)
    var channels: [GlueChannel] = Array(repeating: GlueChannel(), count: GlueOptions.maxChannels)

    init() {}

    /// Merge a plist from the driver (missing keys keep the defaults), as SettingsFromPlist does.
    init(plist: [String: Any]) {
        if let b = plist["bypass"] as? NSNumber { bypass = b.doubleValue != 0 }
        if let d = plist["drive"] as? NSNumber { drive = min(max(d.doubleValue, 0), 18) }
        if let st = plist["stages"] as? [NSNumber] {
            for (k, n) in st.enumerated() where k < 4 {
                let o = n.intValue
                stages[k] = (o >= 0 && o < GlueOptions.options(k).count) ? o : 0
            }
        }
        if let ch = plist["channels"] as? [[String: Any]] {
            for (c, e) in ch.enumerated() where c < GlueOptions.maxChannels {
                var g = channels[c]
                if let v = e["on"] as? NSNumber { g.on = v.doubleValue != 0 }
                if let v = e["link"] as? NSNumber { g.link = v.doubleValue != 0 && c % 2 == 0 }
                if let v = e["thr"] as? NSNumber { g.threshold = min(0, v.doubleValue) }
                if let v = e["src"] as? NSNumber { let s = v.intValue; g.source = (s >= 0 && s < GlueOptions.sourceNames.count) ? s : 0 }
                if let eq = e["eq"] as? [NSNumber] {
                    for b in 0..<4 {
                        if 2 * b < eq.count { g.bands[b].freq = eq[2 * b].doubleValue }
                        if 2 * b + 1 < eq.count { g.bands[b].gain = eq[2 * b + 1].doubleValue }
                    }
                }
                channels[c] = g
            }
        }
    }

    /// The plist for the first `count` channels, as SettingsToPlist writes it.
    func plist(channels count: Int) -> [String: Any] {
        var list: [[String: Any]] = []
        for c in 0..<min(count, channels.count) {
            let g = channels[c]
            var eq: [Double] = []
            for b in g.bands { eq.append(b.freq); eq.append(b.gain) }
            list.append(["on": g.on ? 1 : 0, "thr": g.threshold, "src": g.source, "eq": eq, "link": g.link ? 1 : 0])
        }
        return ["version": 1, "bypass": bypass ? 1 : 0, "stages": stages, "drive": drive, "channels": list]
    }
}

/// Routing shown on a card: the channel's number in its Glue device and, with the Dual Glue aggregate, its number there (unit B's
/// inputs follow all of unit A's; the plain inputs follow every Glue input, GLUE_PLAN_v1.md "Dual").
enum GlueRouting {
    static func dualInput(unitIndex: Int, channel: Int, channelsA: Int) -> Int {
        return (unitIndex == 0 ? 0 : channelsA) + channel + 1
    }
    static func rawCopyInput(unitIndex: Int, channel: Int, channelsA: Int, channelsB: Int) -> Int {
        return channelsA + channelsB + dualInput(unitIndex: unitIndex, channel: channel, channelsA: channelsA)
    }
}

// MARK: - stereo link

extension GlueSettings {
    /// Channel c follows c - 1 (c odd, 0-based) when c - 1 is linked.
    func isFollower(_ c: Int) -> Bool { return c % 2 == 1 && c > 0 && channels[c - 1].link }

    /// After a change to channel c: a linked leader's settings are copied to its follower (the driver does the same).
    mutating func propagateLink(from c: Int) {
        guard c % 2 == 0, c + 1 < channels.count, channels[c].link else { return }
        var f = channels[c]
        f.link = false
        channels[c + 1] = f
    }
}

// MARK: - name templates (user, 2026-10-28)

struct GlueTemplateEntry {
    let name: String
    let source: Int                // GlueOptions.sourceNames: 0 General, 1 Drums, 2 Vocals, 3 Heavy guitar, 4 Bass, 5 Synth
    let on: Bool
    let link: Bool                 // on the first of a stereo pair (channel 1, 3, 5 ...)
    init(_ name: String, _ source: Int, link: Bool = false, on: Bool = true) { self.name = name; self.source = source; self.on = on; self.link = link }
}

enum GlueTemplates {
    // One unit, 16 inputs (user): the kit on the 8 analog inputs, then 2 bass, 3 guitars and 3 vocals.
    static let band16: [GlueTemplateEntry] = [
        GlueTemplateEntry("Kick", 1), GlueTemplateEntry("Snare Top", 1), GlueTemplateEntry("Snare Bottom", 1), GlueTemplateEntry("Hi-Hat", 1),
        GlueTemplateEntry("Tom 1", 1), GlueTemplateEntry("Floor Tom", 1), GlueTemplateEntry("OH L", 1, link: true), GlueTemplateEntry("OH R", 1),
        GlueTemplateEntry("Bass DI", 4, link: true), GlueTemplateEntry("Bass Amp", 4),
        GlueTemplateEntry("Guitar L", 3, link: true), GlueTemplateEntry("Guitar R", 3), GlueTemplateEntry("Guitar Lead", 3),
        GlueTemplateEntry("Lead Vocal", 2), GlueTemplateEntry("Backing Vocal 1", 2, link: true), GlueTemplateEntry("Backing Vocal 2", 2),
    ]
    // Dual, 32 inputs: the 16 most important sources on the 16 analog inputs (A 1-8: the kit close mics; B 1-8: overheads, bass,
    // guitars, lead and first backing vocal), the rest on the ADAT inputs (A 9-16, B 9-16).
    static let dualA: [GlueTemplateEntry] = [
        GlueTemplateEntry("Kick In", 1, link: true), GlueTemplateEntry("Kick Out", 1), GlueTemplateEntry("Snare Top", 1, link: true), GlueTemplateEntry("Snare Bottom", 1),
        GlueTemplateEntry("Hi-Hat", 1), GlueTemplateEntry("Tom 1", 1), GlueTemplateEntry("Tom 2", 1), GlueTemplateEntry("Floor Tom", 1),
        GlueTemplateEntry("Room L", 1, link: true), GlueTemplateEntry("Room R", 1), GlueTemplateEntry("Ride", 1), GlueTemplateEntry("Guitar 3", 3),
        GlueTemplateEntry("Acoustic L", 0, link: true), GlueTemplateEntry("Acoustic R", 0),
        GlueTemplateEntry("Backing Vocal 2", 2, link: true), GlueTemplateEntry("Backing Vocal 3", 2),
    ]
    static let dualB: [GlueTemplateEntry] = [
        GlueTemplateEntry("OH L", 1, link: true), GlueTemplateEntry("OH R", 1), GlueTemplateEntry("Bass DI", 4, link: true), GlueTemplateEntry("Bass Amp", 4),
        GlueTemplateEntry("Guitar 1", 3, link: true), GlueTemplateEntry("Guitar 2", 3), GlueTemplateEntry("Lead Vocal", 2), GlueTemplateEntry("Backing Vocal 1", 2),
        GlueTemplateEntry("Keys L", 5, link: true), GlueTemplateEntry("Keys R", 5), GlueTemplateEntry("Synth L", 5, link: true), GlueTemplateEntry("Synth R", 5),
        GlueTemplateEntry("Percussion", 1), GlueTemplateEntry("Lead Vocal Double", 2),
        GlueTemplateEntry("Click", 0, on: false), GlueTemplateEntry("Talkback", 0, on: false),
    ]

    /// Applies names (returned) and, with `settings`, the sources, on/off and stereo links to `s` (thresholds and EQ are kept).
    static func apply(_ t: [GlueTemplateEntry], to s: inout GlueSettings, channels: Int, settings: Bool) -> [String] {
        var names: [String] = []
        for c in 0..<min(channels, t.count) {
            names.append(t[c].name)
            if settings {
                s.channels[c].source = t[c].source
                s.channels[c].on = t[c].on
                s.channels[c].link = c % 2 == 0 && t[c].link
            }
        }
        if settings { for c in stride(from: 0, to: channels, by: 2) { s.propagateLink(from: c) } }
        return names
    }
}
