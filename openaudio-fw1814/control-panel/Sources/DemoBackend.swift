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

    init(id: UInt64, rate: UInt32, adat: Bool, guid: String = "0xD6C040074B9FB") {
        entryID = id
        self.guid = guid
        func zeros(_ n: Int, _ v: Int = 0) -> [NSNumber] { return [NSNumber](repeating: NSNumber(value: v), count: n) }
        var clock = MAClockSource.factoryDefault
        clock.sync = .internalUnmuted; clock.adatIn = adat; clock.adatOut = adat
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
            "MAHeadphoneLevels": [-20, -20, -12, -12].map { NSNumber(value: $0) }, "MAHeadphoneMute": zeros(4), "MAHeadphoneLink": zeros(4, 1),
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
    }

    var info: FWDeviceInfo { return FWDeviceInfo(entryID: entryID, guid: guid, name: "FireWire 1814 (demo)") }
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

    func readPeaks() -> [UInt32]? {
        phase += 0.12
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

    func snapshotAll(to dir: String, prefix: String, knob: Int = 0, ab: (MAHeadphoneSource, MAHeadphoneSource)? = nil) {
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
