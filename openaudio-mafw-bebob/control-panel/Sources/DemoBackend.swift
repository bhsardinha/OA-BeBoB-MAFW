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
    private let guid: String

    let profile: DeviceProfile

    init(id: UInt64, rate: UInt32, adat: Bool, external: Bool = false, guid: String = "0xD6C040074B9FB", profile: DeviceProfile = FW1814Profile.shared) {
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
        if !DemoBackend.fixedMeters { phase += 0.12 } else { phase = 1.0 }
        return (0..<FWDevice.peakCount).map { i in
            let a = 0.5 + 0.5 * sin(phase * (0.8 + Double(i % 7) * 0.12) + Double(i) * 0.9)
            let top = 14000.0 + Double(i % 5) * 4600.0                 // some channels touch full scale (clip LED)
            return UInt32(max(0, a * a * top))
        }
    }
}

// Debug aid: `--snapshot DIR` writes a PNG of every tab and both mini-meter windows, then quits (screencapture is not available over SSH).
// `--knob N` first turns the on-screen level-controller knob by N detents through the normal write path; `--ab A,B` (source masks 1, 2, 4)
// first writes the A/B sources as the sheet's Set does.
import AppKit

extension PanelWindowController {
    private func writePNG(_ view: NSView, _ path: String) {
        guard let rep = view.bitmapImageRepForCachingDisplay(in: view.bounds) else { return }
        view.cacheDisplay(in: view.bounds, to: rep)
        if let data = rep.representation(using: .png, properties: [:]) { try? data.write(to: URL(fileURLWithPath: path)) }
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
