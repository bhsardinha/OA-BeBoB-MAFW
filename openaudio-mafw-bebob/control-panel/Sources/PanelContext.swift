// PN.3/PN.4 -- state shared by the tabs of one interface window: the device model, the latest meter dB values, and the helpers that
// translate model state into strip state. The tabs call `rebuild()` when the ChannelLayout changes and `refresh()` after any model change.
import AppKit

final class PanelContext {
    let model: DeviceModel
    let device: PanelBackend?
    /// The model (official panel: the CXDevice subclass + its settings views), from the device's registry class (DeviceProfiles).
    let profile: DeviceProfile
    /// "Follow external clock rate" for this interface (RATE_FOLLOW_PLAN_v1), ticked by the panel controller
    let follower = RateFollower()
    private(set) var meterDB = [Int](repeating: -64, count: FWDevice.peakCount)

    init(model: DeviceModel, device: PanelBackend?, profile: DeviceProfile) { self.model = model; self.device = device; self.profile = profile }

    var peaks: PeakMap { return profile.peaks }

    var layout: ChannelLayout { return model.layout }

    /// Pull one snapshot of the shared page and convert it (reference cookie 0x5e9/0x5f3 path, ~30 Hz).
    func pollMeters() {
        guard let raw = device?.readPeaks() else { return }
        for i in 0..<raw.count { meterDB[i] = peakToDB(raw[i]) }
    }
    func db(_ slot: Int) -> Int { return slot >= 0 && slot < meterDB.count ? meterDB[slot] : -64 }

    // MARK: encoder (ctrl) state, A2-P13 §2
    var rotaryGroup: MARotaryMask.Group? { return MARotaryMask.group(of: model.rotaryMask) }
    func ctrlState(group: MARotaryMask.Group, bit: UInt64) -> (enabled: Bool, on: Bool) {
        let enabled = rotaryGroup == group
        return (enabled, enabled && (model.rotaryMask & bit) != 0)
    }

    // MARK: routing masks (cookie 0x5f1: bit i = destination i, 0 = out 1/2, 1 = out 3/4)
    func setRoute(_ key: MAKey, element: Int, dest: Int, on: Bool) {
        let cur = model.element(key, element)
        let bit = 1 << dest
        let mask = on ? (cur | bit) : (cur & ~bit)
        model.setElement(key, element, mask)
        model.commit()
    }
}

/// The level channels a MARotaryControlV2 mask addresses (A2-P13 §2 bit layout): bits 0-15 SW-return pairs, 16-31 input pairs, 32-47 output
/// pairs, bit 50 the aux output pair, bits 56-57 headphone pairs 1-2; pair p = channels 2p-1, 2p (1-based). Used by the level-controller
/// ring and the demo backend; the kext applies the encoder steps itself.
enum RotaryTargets {
    static func channels(mask: UInt64) -> [(key: MAKey, channels: [Int])] {
        func pairs(_ lo: Int, _ hi: Int) -> [Int] { return (lo...hi).filter { mask & (UInt64(1) << UInt64($0)) != 0 }.map { $0 - lo + 1 } }
        func chans(_ ps: [Int]) -> [Int] { return ps.flatMap { [2 * $0 - 1, 2 * $0] } }
        var out: [(key: MAKey, channels: [Int])] = []
        let sw = pairs(0, 15), inp = pairs(16, 31), outp = pairs(32, 47), hp = pairs(56, 57)
        if !sw.isEmpty { out.append((MAKeys.swReturnLevels, chans(sw))) }
        if !inp.isEmpty { out.append((MAKeys.inputLevels, chans(inp))) }
        if !outp.isEmpty { out.append((MAKeys.outputLevels, chans(outp))) }
        if mask & (UInt64(1) << 50) != 0 { out.append((MAKeys.auxOutputLevels, [1, 2])) }
        if !hp.isEmpty { out.append((MAKeys.headphoneLevels, chans(hp))) }
        return out
    }
}

/// Protocol for the tab views so the window controller can drive them uniformly.
protocol PanelTab: AnyObject {
    func rebuild()
    func refresh()
    func updateMeters()
    /// Size the tab's content needs (the window fits itself to it when the tab is shown).
    var preferredSize: NSSize { get }
}
