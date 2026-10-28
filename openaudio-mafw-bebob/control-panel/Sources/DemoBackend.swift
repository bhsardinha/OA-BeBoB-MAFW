// Debug aid: `OpenAudio\ FW1814 --demo [--spdif] [--rate N]` runs the real UI against an in-memory device with synthetic meters, so layouts,
// greying rules and the write paths can be looked at without an interface (no kext, no hardware; the airliner rule is about hardware).
// Not part of the shipping behaviour: the app only uses it when the flag is given. The level-controller knob is simulated here the way the
// kext is described to apply it: one level step per detent on every channel selected in MARotaryControlV2, clamped to -129...0.
import Foundation

final class DemoBackend: PanelBackend {
    let entryID: UInt64
    var onPropertiesChanged: (() -> Void)?
    private var props: [String: Any]
    private var phase = 0.0
    private var env: [Double] = [], seed: UInt32 = 0x1814
    private let guid: String

    let profile: DeviceProfile

    init(id: UInt64, rate: UInt32, adat: Bool, external: Bool = false, guid: String = "0xD6C04000DE001", profile: DeviceProfile = FW1814Profile.shared) {
        entryID = id
        self.profile = profile
        // the first demo Audiophile shows the bench unit's GUID (capture 89 / 92)
        self.guid = (profile === AudiophileProfile.shared && id == 1) ? "0xD6C0300211D54" : guid
        func zeros(_ n: Int, _ v: Int = 0) -> [NSNumber] { return [NSNumber](repeating: NSNumber(value: v), count: n) }
        var clock = MAClockSource.factoryDefault
        clock.sync = .internalUnmuted; clock.adatIn = adat; clock.adatOut = adat
        if external { clock.sync = .externalDigital; clock.spdifCoaxial = true }   // `--external`: External - digital, coaxial S/PDIF
        var pans: [NSNumber] = []
        for i in 0..<18 { pans.append(NSNumber(value: i % 2 == 0 ? -255 : 255)) }
        var info = [UInt8](repeating: 0, count: 0x50)
        for (i, ch) in "bridgeCo".utf8.enumerated() { info[i] = ch }
        info[0x10...0x17] = [0x04, 0x6c, 0x0d, 0x00, 0xfb, 0xb9, 0x74, 0x00]; info[0x1B] = 0x83; info[0x1F] = 1
        for (i, ch) in "20070713".utf8.enumerated() { info[0x20 + i] = ch }
        for (i, ch) in "080440".utf8.enumerated() { info[0x28 + i] = ch }
        for (i, ch) in "20040330".utf8.enumerated() { info[0x40 + i] = ch }
        for (i, ch) in "025909".utf8.enumerated() { info[0x48 + i] = ch }
        // A few non-default values so the snapshots show the controls in different states.
        let inLevels = [0, 0, -12, -12, -20, -20, -6, -6, 0, 0, -30, -30, -10, -10, 0, 0, -16, -16].map { NSNumber(value: $0) }
        var aux = zeros(22, -129); aux[0] = NSNumber(value: -24); aux[1] = NSNumber(value: -24); aux[4] = NSNumber(value: -40)
        props = [
            "MASWReturnLevels": [0, 0, -8, -8].map { NSNumber(value: $0) }, "MASWReturnMute": zeros(4), "MASWReturnSolo": zeros(4), "MASWReturnLink": zeros(4, 1),
            "MAOutputLevels": zeros(4), "MAOutputMute": zeros(4), "MAOutputSolo": zeros(4), "MAOutputLink": zeros(4, 1),
            "MAInputLevels": inLevels, "MAInputMute": [0, 0, 0, 0, 1, 1].map { NSNumber(value: $0) } + zeros(12),
            "MAInputSolo": zeros(18), "MAInputLink": zeros(18, 1),
            "MAHeadphoneLevels": [-20, -20, -12, -12].map { NSNumber(value: $0) }, "MAHeadphoneMute": zeros(4), "MAHeadphoneLink": zeros(4, 1), "MAMonitorKnobStep": NSNumber(value: 3),
            "MAAuxSendLevels": aux, "MAAuxOutputLevels": zeros(2), "MAAuxOutputMute": zeros(2), "MAAuxOutputLink": zeros(2, 1),
            "MAAuxOutputPan2": [NSNumber(value: -255), NSNumber(value: 255)], "MAMonitorAuxOnOutput": zeros(3),
            "MAInputPan2": pans, "MAInputRoutingV2": [1, 1, 1, 1, 0, 1, 1, 2, 2].map { NSNumber(value: $0) }, "MASWReturnRoutingV2": [NSNumber(value: 1), NSNumber(value: 2)],
            "MAGlobalDim": NSNumber(value: 0), "MAGlobalMute": NSNumber(value: 0),
            "MARotaryControlV2": NSNumber(value: MARotaryMask.factoryDefault),
            // Seeded as the A/B sheet writes A = out 1/2, B = out 3/4 (entry 0 kept). The kext's own default {1,2,3,4} has no valid mask in
            // entry 2, which the panel shows as "-" (A2-P13 §5 open item 4).
            "MAMomentarySwitchRoutings": [1, 2, 3, 4].map { NSNumber(value: $0) }, "MAHeadphoneSourceV2": NSNumber(value: 1),
            "MAHeadphoneSecondSource": NSNumber(value: 2), "MAMomentarySwitchSource": NSNumber(value: 0),
            "MAClockSource": NSNumber(value: clock.wireValue), "MASampleRate": NSNumber(value: rate), "MARateLocked": NSNumber(value: 1),
            "MAActualProductName": "FireWire 1814 (demo)", "MAFirmwareInfo": Data(info),
        ]
        if profile === AudiophileProfile.shared { props = DemoBackend.audiophileProps(rate: rate, info: DemoBackend.audiophileInfoBlock) }
        if profile === FW410Profile.shared { props = DemoBackend.fw410Props(rate: rate, info: info) }
        if props["MAEnableStereoPairEngines"] == nil { props["MAEnableStereoPairEngines"] = NSNumber(value: 0) }   // every driver publishes it
    }

    /// FireWire 410 demo state (plan FW410 v2 F4.1): the kext's group sizes (SW returns 10, outputs 10, inputs 4, phones 2, aux sends 14, aux
    /// out 2, monitor switches 6, routings 5 / 2; kext models/FW410) with the routing the SL rig's 410 had (capture 103: SW return n -> out n,
    /// inputs unrouted, phones on 1/2) and a few changes so the snapshots show states (phones on 1/2 + S/PDIF: multi-select).
    private static func fw410Props(rate: UInt32, info: [UInt8]) -> [String: Any] {
        func n(_ v: [Int]) -> [NSNumber] { return v.map { NSNumber(value: $0) } }
        func zeros(_ c: Int, _ v: Int = 0) -> [NSNumber] { return [NSNumber](repeating: NSNumber(value: v), count: c) }
        var aux = zeros(14, -129); aux[0] = NSNumber(value: -24); aux[4] = NSNumber(value: -40)
        var clock = MAClockSource(raw: 0x4000_0004)                                    // internal, coaxial S/PDIF in (capture 103)
        clock.sync = .internalUnmuted
        return [
            "MASWReturnLevels": n([0, 0, -8, -8, 0, 0, -16, -16, 0, 0]), "MASWReturnMute": zeros(10), "MASWReturnSolo": zeros(10), "MASWReturnLink": zeros(10, 1),
            "MAOutputLevels": zeros(10, -12), "MAOutputMute": zeros(10), "MAOutputSolo": zeros(10), "MAOutputLink": zeros(10, 1),
            "MAInputLevels": n([0, 0, -12, -12]), "MAInputMute": zeros(4), "MAInputSolo": zeros(4), "MAInputLink": zeros(4, 1),
            "MAHeadphoneLevels": n([-20, -20]), "MAHeadphoneMute": zeros(2), "MAHeadphoneLink": zeros(2, 1), "MAMonitorKnobStep": NSNumber(value: 3),
            "MAAuxSendLevels": aux, "MAAuxOutputLevels": zeros(2), "MAAuxOutputMute": zeros(2), "MAAuxOutputLink": zeros(2, 1),
            "MAAuxOutputPan2": n([-255, 255]), "MAMonitorAuxOnOutput": n([0, 0, 0, 1, 0, 0]),
            "MAInputPan2": n([-255, 255, -255, 255]), "MAInputRoutingV2": n([1, 0]), "MASWReturnRoutingV2": n([1, 2, 4, 8, 16]),
            "MAGlobalDim": NSNumber(value: 0), "MAGlobalMute": NSNumber(value: 0),
            "MARotaryControlV2": NSNumber(value: MARotaryMask.factoryDefault),
            "MAHeadphoneSourceV2": NSNumber(value: 17), "MAEnableStereoPairEngines": NSNumber(value: 0),
            "MAClockSource": NSNumber(value: clock.wireValue), "MASampleRate": NSNumber(value: rate), "MARateLocked": NSNumber(value: 1),
            "MAActualProductName": "FireWire 410 (demo)", "MAFirmwareInfo": Data(info),
        ]
    }

    /// FireWire Audiophile demo state (AP-5.2): the kext's group sizes (SW returns 6, outputs 6, inputs 4, phones 2, aux sends 10, aux out 2,
    /// monitor switches 3, routings 2 / 3; kext models/FWAudiophile) with factory values (capture 92) and a few changes so the snapshots show states.
    /// The bench Audiophile's MAFirmwareInfo as the official driver read it from the unit (capture 89, SL rig 2026-10-19): firmware 2007-05-04
    /// 10:22:12, bootloader 2003-10-06 10:00:41, hardware model 13 version 1, GUID 000d6c0300211d54.
    static let audiophileInfoBlock: [UInt8] = {
        let hex = "627269646765436f0100000000000000036c0d00541d21000d000000010000003230303730353034313032323132000060000100ffffff00000008200000180032303033313030363130303034310000"
        return stride(from: 0, to: hex.count, by: 2).map { i -> UInt8 in
            let a = hex.index(hex.startIndex, offsetBy: i); return UInt8(hex[a..<hex.index(a, offsetBy: 2)], radix: 16)! }
    }()

    private static func audiophileProps(rate: UInt32, info: [UInt8]) -> [String: Any] {
        func n(_ v: [Int]) -> [NSNumber] { return v.map { NSNumber(value: $0) } }
        func zeros(_ c: Int, _ v: Int = 0) -> [NSNumber] { return [NSNumber](repeating: NSNumber(value: v), count: c) }
        var aux = zeros(10, -129); aux[0] = NSNumber(value: -24); aux[1] = NSNumber(value: -24); aux[4] = NSNumber(value: -40)
        return [
            "MASWReturnLevels": n([0, 0, -8, -8, -16, -16]), "MASWReturnMute": zeros(6), "MASWReturnSolo": zeros(6), "MASWReturnLink": zeros(6, 1),
            "MAOutputLevels": zeros(6), "MAOutputMute": zeros(6), "MAOutputSolo": zeros(6), "MAOutputLink": zeros(6, 1),
            "MAInputLevels": n([0, 0, -12, -12]), "MAInputMute": zeros(4), "MAInputSolo": zeros(4), "MAInputLink": zeros(4, 1),
            "MAHeadphoneLevels": n([-20, -20]), "MAHeadphoneMute": zeros(2), "MAHeadphoneLink": zeros(2, 1), "MAMonitorKnobStep": NSNumber(value: 3),
            "MAAuxSendLevels": aux, "MAAuxOutputLevels": zeros(2), "MAAuxOutputMute": zeros(2), "MAAuxOutputLink": zeros(2, 1),
            "MAAuxOutputPan2": n([-255, 255]), "MAMonitorAuxOnOutput": zeros(3),
            "MAInputPan2": n([-255, 255, -255, 255]), "MAInputRoutingV2": n([1, 4]), "MASWReturnRoutingV2": n([1, 2, 4]),
            "MAGlobalDim": NSNumber(value: 0), "MAGlobalMute": NSNumber(value: 0),
            "MARotaryControlV2": NSNumber(value: MARotaryMask.factoryDefault),
            "MAMomentarySwitchRoutings": n([1, 2, 3, 4]), "MAHeadphoneSourceV2": NSNumber(value: 1), "MAMomentarySwitchSource": NSNumber(value: 0),
            "MAClockSource": NSNumber(value: MAClockSource(raw: 0x4000_0000).wireValue), "MASampleRate": NSNumber(value: min(rate, 96000)), "MARateLocked": NSNumber(value: 1),
            "MAActualProductName": "FireWire Audiophile (demo)", "MAFirmwareInfo": Data(info),
        ]
    }

    var info: FWDeviceInfo { return FWDeviceInfo(entryID: entryID, guid: guid, name: "\(profile.productName) (demo)") }
    /// MAExternalSyncRate (RATE_FOLLOW_PLAN_v1): a source at the demo's own rate while the sync source is external, else none
    var externalSyncRate: UInt32? {
        let c = MAClockSource(raw: UInt32(truncatingIfNeeded: (props[MAKeys.clockSource.name] as? NSNumber)?.uint64Value ?? 0))
        return c.sync == .externalDigital ? (props[MAKeys.sampleRate.name] as? NSNumber)?.uint32Value ?? 0 : 0
    }
    func copyProperties() -> [String: Any] { return props }
    func setProperties(_ properties: [String: Any]) -> kern_return_t {
        for (k, v) in properties {
            if k == MAKeys.virtualRotateKnob.name, let n = v as? NSNumber {
                rotate(Int(Int32(truncatingIfNeeded: n.int64Value)))      // write-only trigger: not stored
                continue
            }
            props[k] = v
        }
        return 0
    }

    /// Encoder simulation: +-1 level step per detent on each selected channel, then a "properties changed" notification as the kext sends.
    private func rotate(_ detents: Int) {
        let mask = (props[MAKeys.rotaryControl.name] as? NSNumber)?.uint64Value ?? MARotaryMask.factoryDefault
        for t in RotaryTargets.channels(mask: mask) {
            guard var a = props[t.key.name] as? [NSNumber] else { continue }
            for c in t.channels where c >= 1 && c <= a.count {
                let v = Int(Int32(truncatingIfNeeded: a[c - 1].int64Value)) + detents
                a[c - 1] = NSNumber(value: min(MARange.levelMax, max(MARange.levelMin, v)))
            }
            props[t.key.name] = a
        }
        // Through the run loop (not the main dispatch queue), so it also arrives while the snapshot code runs inside a dispatched block.
        RunLoop.main.perform(inModes: [.commonModes]) { [weak self] in self?.onPropertiesChanged?() }
    }

    /// `--fixed-meters` (test aid, plan v4 AP-5.1): the synthetic meters stand still so snapshots are reproducible and two builds compare byte for byte.
    static var fixedMeters = false
    func readPeaks() -> [UInt32]? {
        if !DemoBackend.fixedMeters { return livePeaks() }
        phase = 1.0
        return (0..<FWDevice.peakCount).map { i in
            let a = 0.5 + 0.5 * sin(phase * (0.8 + Double(i % 7) * 0.12) + Double(i) * 0.9)
            let top = 14000.0 + Double(i % 5) * 4600.0                 // some channels touch full scale (clip LED)
            return UInt32(max(0, a * a * top))
        }
    }

    /// Peaks shaped like real material (user, 2026-10-28: a sine creeping one step a tick is not what audio does): per channel, hits that
    /// jump up and decay (drums), a level that wanders around a mean with phrase gaps (voice, bass), occasional near-full-scale peaks, and
    /// silent channels. One call per meter tick (30 Hz).
    private func livePeaks() -> [UInt32] {
        func rnd() -> Double { seed = seed &* 1664525 &+ 1013904223; return Double(seed >> 8) / Double(1 << 24) }
        if env.count != FWDevice.peakCount { env = [Double](repeating: -64, count: FWDevice.peakCount) }
        for i in 0..<env.count {
            switch i % 6 {
            case 0: env[i] = -64                                                     // nothing connected
            case 1, 2: env[i] = rnd() < 0.07 ? -2 - 10 * rnd() : max(-64, env[i] - 6)     // drum hits, ~2 per second, peaks falling 180 dB/s
            case 3, 4:                                                               // sustained, with gaps
                if rnd() < 0.01 { env[i] = env[i] > -40 ? -64 : -12 }
                else if env[i] > -40 { env[i] = min(-4, max(-20, env[i] + (rnd() - 0.5) * 2.5)) }
            default: env[i] = rnd() < 0.02 ? -0.5 * rnd() : -26 + 3 * rnd()            // a bed with rare one-tick transients near 0 dBFS
            }
        }
        return env.map { $0 <= -64 ? 0 : UInt32(32768 * pow(10, $0 / 20)) }
    }
}

// Debug aid: `--snapshot DIR` writes a PNG of every tab and both mini-meter windows, then quits (screencapture is not available over SSH).
// `--knob N` first turns the on-screen level-controller knob by N detents through the normal write path; `--ab A,B` (source masks 1, 2, 4)
// first writes the A/B sources as the sheet's Set does.
import AppKit

extension PanelWindowController {
    private func writePNG(_ view: NSView, _ path: String) {
        // The Advanced and Dual windows draw their static views into one root layer and their meters into GPU framebuffers (MeterField):
        // cacheDisplay renders neither, so those windows are captured as WindowServer shows them (content area only).
        if view.canDrawSubviewsIntoLayer, let w = view.window, w.isVisible,
           let img = CGWindowListCreateImage(.null, .optionIncludingWindow, CGWindowID(w.windowNumber), [.boundsIgnoreFraming, .bestResolution]) {
            let scale = CGFloat(img.width) / w.frame.width
            let content = w.contentRect(forFrameRect: w.frame)
            let crop = CGRect(x: 0, y: (w.frame.maxY - content.maxY) * scale, width: content.width * scale, height: content.height * scale)
            if let c = img.cropping(to: crop), let data = NSBitmapImageRep(cgImage: c).representation(using: .png, properties: [:]) {
                try? data.write(to: URL(fileURLWithPath: path)); return
            }
        }
        guard let rep = view.bitmapImageRepForCachingDisplay(in: view.bounds) else { return }
        view.cacheDisplay(in: view.bounds, to: rep)
        if let data = rep.representation(using: .png, properties: [:]) { try? data.write(to: URL(fileURLWithPath: path)) }
    }

    /// `--demo --advanced --snapshot DIR`: the Advanced window from a fresh store (first opening -> DAW), then every mode; the switch to
    /// Console also writes the question it asks (its levels rise). Outside 44.1 / 48 kHz (`--rate 96000`) the one PNG is the lock-out.
    func snapshotAdvanced(to dir: String, prefix: String) {
        try? FileManager.default.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        if device is DemoBackend { try? FileManager.default.removeItem(at: advancedStoreFolder) }
        printScene(prefix, "before")
        openAdvanced()
        guard let a = advanced else { return }
        a.ignoresOcclusion = true
        let demo = device is DemoBackend
        // demo loops for the Hybrid snapshot (a channel the optical mode lacks drops its loop, as on a unit)
        if demo { a.store.loops = [] }
        if demo { a.store.loops += [HybridLoop(name: "Vocal comp", device: "1176", sends: ["out.l3"], returns: ["in.a5"]),
                         HybridLoop(name: "Reamp", device: "reamp box \u{2192} amp \u{2192} SM57", sends: ["out.l4"], returns: ["in.a1"]),
                         HybridLoop(name: "Plate reverb", device: "Lexicon via ADAT", sends: ["out.adat1", "out.adat2"], returns: ["in.adat1", "in.adat2"])] }
        for _ in 0..<12 { RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05)) }
        if let v = a.window.contentView { writePNG(v, "\(dir)/\(prefix)-advanced-open.png") }
        guard a.available else { return }
        // demo outputs start at 0 dB; lower them so the Console switch has levels to raise (and its question is captured)
        if demo { ctx.model.restore([MAKeys.outputLevels.name: [-40, -40, -24, -24].map { NSNumber(value: Int32($0)) }]) }
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.2))
        printScene(prefix, "open (\(a.mode.rawValue))")
        // a real unit ends in Monitor: the scene kept at the first opening, i.e. its mixer as it was before the run
        for m in demo ? [AdvancedMode.monitor, .console, .hybrid, .daw] : [.console, .hybrid, .monitor] {
            let plan = AdvancedSwitch.plan(from: a.mode, to: m, current: ctx.model.cache, stored: a.store.scene(m))
            if !plan.rises.isEmpty {
                let alert = AdvancedWindowController.riseAlert(plan)
                alert.layout()
                if let v = alert.window.contentView { writePNG(v, "\(dir)/\(prefix)-advanced-ask-\(m.rawValue).png") }
                print("\(prefix) advanced \(a.mode.rawValue) -> \(m.rawValue): \(plan.rises.count) rises")
            }
            a.request(m, ask: false)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: demo ? 0.4 : 1.2))
            if let v = a.window.contentView { writePNG(v, "\(dir)/\(prefix)-advanced-\(m.rawValue).png") }
            printScene(prefix, m.rawValue)
        }
    }

    /// `--demo --devices 2 --dual --snapshot DIR`: the Dual window in each of its modes, and its setup sheet.
    func snapshotDual(_ d: AdvancedDualWindowController, to dir: String, prefix: String) {
        try? FileManager.default.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        if device is DemoBackend { try? FileManager.default.removeItem(at: advancedStoreFolder) }      // reproducible: a fresh demo store
        d.writeSnapshots(dir: dir, prefix: prefix) { [weak self] v, path in self?.writePNG(v, path) }
    }

    /// Bench aid: the mixer keys the modes write, read back from the driver (registry), one line per key.
    private func printScene(_ prefix: String, _ what: String) {
        ctx.model.refresh()
        let props = (device as PropertyTransport).copyProperties()
        let keys = [MAKeys.inputRoutingV2, MAKeys.swReturnRoutingV2, MAKeys.swReturnLevels, MAKeys.outputLevels, MAKeys.outputMute,
                    MAKeys.monitorAuxOnOutput, MAKeys.headphoneSourceV2, MAKeys.headphoneSecondSource, MAKeys.rotaryControl, MAKeys.inputLevels]
        print("\(prefix) [\(what)] driver: " + keys.map { k -> String in
            let v = props[k.name]
            if let a = v as? [Any] { return k.name + "=" + a.map { "\(DeviceModel.int($0))" }.joined(separator: ",") }
            if let n = v as? NSNumber { return k.name + "=" + String(format: "0x%llx", n.uint64Value) }
            return k.name + "=-"
        }.joined(separator: "  "))
    }

    func snapshotAll(to dir: String, prefix: String, knob: Int = 0, ab: (UInt32, UInt32)? = nil) {
        try? FileManager.default.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        show()
        if let s = ab { ctx.model.setABSources(a: s.0, b: s.1) }            // the write the A/B sheet's Set makes
        if knob != 0 {
            turnLevelKnob(knob)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.3))
        }
        for _ in 0..<20 { RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05)) }     // let the meters and peak holds settle
        func logFrame(_ what: String, _ w: NSWindow?) {
            guard let f = w?.frame else { return }
            print(String(format: "%@ %@ frame %.0f,%.0f %.0fx%.0f centre %.0f,%.0f", prefix, what, f.minX, f.minY, f.width, f.height, f.midX, f.midY))
        }
        let sc = screenCenter()
        print(String(format: "%@ screen centre %.0f,%.0f", prefix, sc.x, sc.y))
        logFrame("full", window)
        if let v = window.contentView { writePNG(v, "\(dir)/\(prefix)-launch.png") }     // state right after opening, before any tab click
        for name in ["mixer", "output", "hardware", "about"] {
            selectTab(name)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.4))
            if let v = window.contentView { writePNG(v, "\(dir)/\(prefix)-\(name).png") }
            logFrame(name, window)
            if name == "hardware", let panel = hardwareTab()?.presentABSheet() {
                RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.6))
                if let v = panel.contentView { writePNG(v, "\(dir)/\(prefix)-hardware-ab-sheet.png") }
                window.endSheet(panel)
                RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.3))
            }
        }
        for (m, n) in [(1, "wide"), (2, "narrow")] {
            setMode(m)
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.6))
            logFrame(n, miniWindow(narrow: m == 2))
            if let w = (m == 1 ? miniWindow(narrow: false) : miniWindow(narrow: true)), let v = w.contentView { writePNG(v, "\(dir)/\(prefix)-meters-\(n).png") }
        }
        setMode(0)
        logFrame("full again", window)
    }
}
