// PN.3/PN.5/PN.6 -- the panel's view of one interface: a cache of the registry properties, typed accessors, and batched writes.
// Mirrors the reference CXDevice (A2-P3): local property cache + dirty-key set + one IORegistryEntrySetCFProperties per commit, the
// sample rate pushed 1 s after the last change (CXDevice::SendNewSampleRateTimer, P7-fup §3.1). Value domains and index rules come from
// A2-P13 (reference translator decompile). No AppKit here, so the write rules are unit-testable with a mock transport.
import Foundation

protocol PropertyTransport: AnyObject {
    func copyProperties() -> [String: Any]
    @discardableResult func setProperties(_ properties: [String: Any]) -> kern_return_t
}

/// The sample rate as the reference panel handles it (A2-P8 §6, Correction 2026-10-17): the panel never writes `MASampleRate`; 1 s after the last
/// pick (CXDevice::SendNewSampleRateTimer @0x6c8c8) it calls CoreAudio `setNominalSampleRate:` on the device's audio object (CXDevice+0x60), and the
/// displayed rate follows CoreAudio. The lock colour comes from `MARateLocked` (kext). A write through setProperties applies the rate to the engine
/// without completing the kext's pending state, so the device is never re-programmed and `MARateLocked` stays 0 (capture 80, 79).
protocol RateBackend: AnyObject {
    var nominalRate: UInt32? { get }
    /// `kAudioDevicePropertyAvailableNominalSampleRates` (the reference reads it through `-supportedSampleRates`, cookies 0x603/0x604, A2-P8 §10); empty when unknown.
    var availableRates: [UInt32] { get }
    @discardableResult func setNominalRate(_ hz: UInt32) -> Bool
    var onRateChanged: (() -> Void)? { get set }
    /// `kAudioDevicePropertyDeviceIsRunningSomewhere`: some client runs the device's IO (the status menu greys the rates then, README known issue 2)
    var isRunningSomewhere: Bool { get }
}
extension RateBackend { var isRunningSomewhere: Bool { return false } }

final class DeviceModel {
    private let transport: PropertyTransport
    private(set) var cache: [String: Any] = [:]
    private var dirty: [String: Any] = [:]
    private let rateBackend: RateBackend?
    private var rateTimer: Timer?
    private var rateSettleTimer: Timer?
    private var pendingRate: UInt32?

    /// Called with the keys whose value changed in the kernel (not for our own pending writes).
    var onKernelChange: ((Set<String>) -> Void)?
    /// Called after a local write so the views can redraw immediately.
    var onLocalChange: ((Set<String>) -> Void)?

    /// Rates to offer until CoreAudio lists the device's own (the model's kext table; DeviceProfile.defaultRates).
    private let fallbackRates: [UInt32]

    init(transport: PropertyTransport, rate: RateBackend? = nil, fallbackRates: [UInt32] = []) {
        self.transport = transport
        self.fallbackRates = fallbackRates
        self.rateBackend = rate
        rate?.onRateChanged = { [weak self] in self?.halRateChanged() }
        refresh()
    }

    // MARK: reading

    /// Re-read the full dictionary and diff it against the cache, as `ReceivedPropertiesChangedMessage` does (A2-P2). Keys with a
    /// pending local write keep the local value until the commit.
    func refresh() {
        let fresh = transport.copyProperties()
        var changed = Set<String>()
        for (k, v) in fresh where k.hasPrefix("MA") {
            if dirty[k] != nil { continue }
            if cache[k] == nil || !DeviceModel.same(cache[k]!, v) { changed.insert(k) }
            cache[k] = v
        }
        if !changed.isEmpty { onKernelChange?(changed) }
    }

    private static func same(_ a: Any, _ b: Any) -> Bool { return (a as AnyObject).isEqual(b) }

    func ints(_ key: MAKey) -> [Int] {
        guard let arr = cache[key.name] as? [Any] else { return [] }
        return arr.map { DeviceModel.int($0) }
    }

    static func int(_ v: Any) -> Int {
        if let n = v as? NSNumber { return Int(Int32(truncatingIfNeeded: n.int64Value)) }     // levels are signed 32-bit (capture 62)
        return 0
    }

    func number(_ key: MAKey) -> UInt64? {
        return (cache[key.name] as? NSNumber)?.uint64Value
    }

    /// 1-based element access, matching the panel's index convention (A2-P13 §1).
    func element(_ key: MAKey, _ index: Int) -> Int {
        let a = ints(key)
        return index >= 1 && index <= a.count ? a[index - 1] : 0
    }

    var clockSource: MAClockSource {
        return MAClockSource(raw: UInt32(truncatingIfNeeded: number(MAKeys.clockSource) ?? UInt64(MAClockSource.factoryDefault.raw)))
    }
    /// What the panel shows: the user's pick until it has been applied, then CoreAudio's nominal rate (reference: cookie 0x601 follows the HAL), then
    /// the kext's `MASampleRate` when there is no CoreAudio backend (demo, tests).
    var sampleRate: UInt32 { return pendingRate ?? rateBackend?.nominalRate ?? UInt32(truncatingIfNeeded: number(MAKeys.sampleRate) ?? 44100) }
    /// Rates the Hardware tab steps through: CoreAudio's list for this device (reference: cookies 0x603/0x604 proxy `-supportedSampleRates`, A2-P8 §10).
    /// Without a CoreAudio backend, or before the HAL device exists, the rates the model's kext table offers.
    var supportedRates: [UInt32] {
        if let r = rateBackend?.availableRates, !r.isEmpty { return r }
        return fallbackRates
    }
    var rateLocked: Bool { return (number(MAKeys.rateLocked) ?? 0) != 0 }
    var isStreaming: Bool { return rateBackend?.isRunningSomewhere ?? false }
    var layout: ChannelLayout { return ChannelLayout(rate: sampleRate, clock: clockSource) }
    var rotaryMask: UInt64 { return number(MAKeys.rotaryControl) ?? MARotaryMask.factoryDefault }

    // MARK: writing

    private func queue(_ key: MAKey, _ value: Any) {
        cache[key.name] = value
        dirty[key.name] = value
        onLocalChange?([key.name])
        scheduleLiveCommit()
    }

    /// Write one element (1-based) of an array key. The whole array is sent, as the reference does (SetChannelLevelProperty).
    func setElement(_ key: MAKey, _ index: Int, _ value: Int) {
        var a = ints(key)
        guard index >= 1 && index <= a.count else { return }
        a[index - 1] = value
        queue(key, a.map { NSNumber(value: Int32(truncatingIfNeeded: $0)) })
    }

    func setNumber(_ key: MAKey, _ value: UInt64) {
        queue(key, NSNumber(value: value))
    }

    /// Flush every pending key in one registry write (UI commit boundary: mouse-up, popup selection).
    /// Live updates while a control is being dragged (user request 2026-10-16): pending writes are flushed every 40 ms instead of only at
    /// mouse-up. The reference panel batches until the commit boundary (A2-P3); this is a deliberate change. Explicit commit() still flushes at once.
    private var liveTimer: Timer?
    private func scheduleLiveCommit() {
        if liveTimer != nil { return }
        let t = Timer(timeInterval: 0.04, repeats: false) { [weak self] _ in
            self?.liveTimer = nil
            self?.commit()
        }
        liveTimer = t
        RunLoop.main.add(t, forMode: .commonModes)
    }

    func commit() {
        guard !dirty.isEmpty else { return }
        let batch = dirty
        dirty.removeAll()
        let kr = transport.setProperties(batch)
        if kr != KERN_SUCCESS {
            writeFailure = (kr, (writeFailure?.keys ?? []) + batch.keys.sorted())
            NotificationCenter.default.post(name: DeviceModel.writeFailed, object: self)
        }
    }

    /// A write the driver refused (IORegistryEntrySetCFProperties result and the keys of the refused batches) since `clearWriteFailure`.
    /// The Advanced windows show it instead of "applied" (ADVANCED_TODO_v2 §2: the chip trusted the cache, not the driver).
    private(set) var writeFailure: (code: kern_return_t, keys: [String])?
    func clearWriteFailure() { writeFailure = nil }
    static let writeFailed = Notification.Name("DeviceModelWriteFailed")

    // MARK: headphone safety (user request 2026-10-16)

    /// Highest phones level the panel allows at start-up: -86 half-dB steps = -43 dB, 15 % of the fader travel from the bottom
    /// (Fader.curve: 0.85 of the track from the top ~ 42.9 dB). The device powers up with 0 dB, which is far too loud for headphones.
    /// A clone addition (the reference panel leaves the level as it finds it).
    static let phonesStartupCeiling = -86

    /// Called once when the panel opens a real interface: every phones level above the ceiling is lowered to it and written at once, before any
    /// audio can reach the headphones. Levels already below the ceiling are never raised.
    func limitPhonesLevelsAtStartup() {
        let a = ints(MAKeys.headphoneLevels)
        guard !a.isEmpty, a.contains(where: { $0 > DeviceModel.phonesStartupCeiling }) else { return }
        queue(MAKeys.headphoneLevels, a.map { NSNumber(value: Int32(truncatingIfNeeded: min($0, DeviceModel.phonesStartupCeiling))) })
        commit()
    }

    // Correction (2026-10-17): the panel no longer saves/restores its own "last settings". The reference's HELPER owns persistence (A4 H11:
    // per-GUID settings under com.m-audio.device-settings.firewire, committed 4 s after the last change, replayed on attach and at login; "the Panel
    // need not write preferences"). The clone's helper does exactly that (verified 2026-10-17: a kext reload with the panel closed restored the levels).
    // Only the headphone safety cap above stays in the panel.

    // MARK: levels, pairs, link (A2-P13 §1, CFW1814DeviceSettings)

    // Balance is kept apart from the faders (user request 2026-10-16). The device has no balance key: the reference panel writes balance into
    // the two channel levels and reads it back as their difference (CFW1814DeviceSettings cookie 0x5f6, CalculateOutputPairBalanceLevel), so a
    // balance move dragged the faders and a single-fader move showed up as balance. Here the panel keeps the balance of every pair itself and
    // shows the fader positions; the level written to the device is fader minus the balance attenuation of that side (same wire values as the
    // reference for the same fader + balance). The balance is panel state (not stored in the device), default centre.
    private var balances: [String: Int] = [:]
    private func balanceKey(_ key: MAKey, _ pair: Int) -> String { return key.name + "#" + String(pair) }
    private func attenuation(_ key: MAKey, channel: Int) -> Int {
        let b = balances[balanceKey(key, (channel + 1) / 2)] ?? 0
        return channel % 2 == 1 ? max(0, b) : max(0, -b)                    // positive balance (right) lowers the left channel
    }

    /// Monitor keys (plan v7 AP-5.4): levels that follow the monitor law, whose bottom is off (-255) instead of the console floor (-129).
    /// Phones only since R127 (user, 2026-10-21): outputs and aux out are back on the console law and the official knob law.
    static let monitorKeys: Set<String> = [MAKeys.headphoneLevels.name]
    static func law(_ key: MAKey) -> FaderLaw { return monitorKeys.contains(key.name) ? .monitor : .console }
    static func floor(_ key: MAKey) -> Int { return law(key) == .monitor ? FaderLaw.off : MARange.levelMin }

    /// Fader position of one channel as the user set it (device level plus the balance attenuation of that side).
    func faderLevel(_ key: MAKey, channel: Int) -> Int {
        let e = element(key, channel)
        if e <= DeviceModel.floor(key) { return e }                          // off stays off whatever the balance
        return min(MARange.levelMax, e + attenuation(key, channel: channel))
    }

    /// Fader of one channel. With `linked` the partner of the pair follows (both faders get the value).
    func setLevel(_ key: MAKey, channel: Int, value: Int, linked: Bool) {
        let lo = DeviceModel.floor(key)
        let v = min(MARange.levelMax, max(lo, value))
        func level(_ ch: Int) -> Int { return v <= lo ? lo : max(lo, v - attenuation(key, channel: ch)) }
        setElement(key, channel, level(channel))
        if linked {
            let p = DeviceModel.partner(of: channel)
            setElement(key, p, level(p))
        }
    }

    /// Channel ids of a pair p: (2p-1, 2p).
    static func channels(ofPair p: Int) -> (Int, Int) { return (2 * p - 1, 2 * p) }
    static func partner(of channel: Int) -> Int { return channel % 2 == 1 ? channel + 1 : channel - 1 }

    func balance(_ key: MAKey, pair: Int) -> Int { return balances[balanceKey(key, pair)] ?? 0 }

    /// Moves only the balance: the faders stay where they are; the side the balance points away from is attenuated by |balance| steps.
    func setBalance(_ key: MAKey, pair: Int, balance: Int) {
        let (l, r) = DeviceModel.channels(ofPair: pair)
        let fl = faderLevel(key, channel: l), fr = faderLevel(key, channel: r)
        balances[balanceKey(key, pair)] = balance
        let lo = DeviceModel.floor(key)
        setElement(key, l, fl <= lo ? fl : max(lo, fl - attenuation(key, channel: l)))
        setElement(key, r, fr <= lo ? fr : max(lo, fr - attenuation(key, channel: r)))
    }

    /// mute / solo / link are per-channel booleans; the panel writes both elements of a pair (SetPairBooleanProperty).
    func setPairFlag(_ key: MAKey, pair: Int, on: Bool) {
        let (l, r) = DeviceModel.channels(ofPair: pair)
        setElement(key, l, on ? 1 : 0)
        setElement(key, r, on ? 1 : 0)
    }
    func pairFlag(_ key: MAKey, pair: Int) -> Bool { return element(key, DeviceModel.channels(ofPair: pair).0) != 0 }

    // MARK: clock source (read-modify-write of MAClockSource, P-ClockUI-encoding §1)

    func updateClock(_ change: (inout MAClockSource) -> Void) {
        var c = clockSource
        change(&c)
        queue(MAKeys.clockSource, NSNumber(value: c.wireValue))
        commit()
    }

    // MARK: sample rate (deferred 1 s, A2-P8 §6)

    /// The user's pick: shown at once, applied 1 s after the last pick (coalesces quick radio changes), like CXDevice::SetHWSetting(0x601).
    func requestSampleRate(_ hz: UInt32) {
        pendingRate = hz
        rateSettleTimer?.invalidate()
        onLocalChange?([MAKeys.sampleRate.name])
        rateTimer?.invalidate()
        rateTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: false) { [weak self] _ in self?.sendRate() }
    }

    /// ref SendNewSampleRateTimer @0x6c8c8: CoreAudio `setNominalSampleRate:`. Without a CoreAudio backend (demo, tests) the rate is written as the
    /// `MASampleRate` property, the clone's earlier behaviour.
    func sendRate() {
        guard let hz = pendingRate else { return }
        guard let rb = rateBackend else {
            pendingRate = nil
            queue(MAKeys.sampleRate, NSNumber(value: hz))
            commit()
            return
        }
        let ok = rb.setNominalRate(hz)
        // The pick stays on screen until CoreAudio reports the new rate; if CoreAudio refuses (or never answers) the display falls back to the real rate.
        rateSettleTimer?.invalidate()
        rateSettleTimer = Timer.scheduledTimer(withTimeInterval: ok ? 8.0 : 1.0, repeats: false) { [weak self] _ in
            guard let me = self, me.pendingRate != nil else { return }
            me.pendingRate = nil
            me.onKernelChange?([MAKeys.sampleRate.name])
        }
    }

    /// CoreAudio reported a nominal-rate change (or the device appeared / left): drop the pick when it has been applied, and redraw.
    func halRateChanged() {
        if let p = pendingRate, rateBackend?.nominalRate == p { pendingRate = nil; rateSettleTimer?.invalidate() }
        onKernelChange?([MAKeys.sampleRate.name])
    }

    // MARK: headphone sources and a/b switch (A2-P13 §5)

    /// phones 1 = MAHeadphoneSourceV2, phones 2 = MAHeadphoneSecondSource; value = the model's source mask (DeviceProfile.phonesSources).
    func setPhonesSource(phones: Int, mask: UInt32) {
        setNumber(phones == 1 ? MAKeys.headphoneSourceV2 : MAKeys.headphoneSecondSource, UInt64(mask))
        commit()
    }

    /// Sheet "set source": entry 0 = A, entry 1 = B of MAMomentarySwitchRoutings (index = the kext's A/B mode; Correction 2026-10-16, see
    /// HardwareTab); the other entries are written back unchanged.
    func setABSources(a: UInt32, b: UInt32) {
        var cur = ints(MAKeys.momentarySwitchRoutings)
        while cur.count < 4 { cur.append(cur.count + 1) }
        cur[0] = Int(a); cur[1] = Int(b)
        queue(MAKeys.momentarySwitchRoutings, cur.map { NSNumber(value: $0) })
        commit()
    }

    /// a/b function radios: cookie 0x608 value 3 -> MAMomentarySwitchSource 1 (direct monitoring), else 0 (headphone A/B).
    func setSwitchFunction(directMonitoring: Bool) {
        setNumber(MAKeys.momentarySwitchSource, directMonitoring ? 1 : 0)
        commit()
    }

    // MARK: output main/aux toggle (MAMonitorAuxOnOutput: 3 elements = output pair 1, pair 2 and the S/PDIF pair, kext (output.Count() >> 1) + 1; A2-P13 §4 Correction)

    func setMonitorAux(pair: Int, aux: Bool) {
        setElement(MAKeys.monitorAuxOnOutput, pair, aux ? 1 : 0)
        commit()
    }
    func monitorAux(pair: Int) -> Bool { return element(MAKeys.monitorAuxOnOutput, pair) != 0 }

    // MARK: rotary encoder assignment (MARotaryControlV2, A2-P13 §2)

    func setRotaryPreset(_ mask: UInt64) { setNumber(MAKeys.rotaryControl, mask); commit() }
    func setRotaryBit(_ bit: UInt64, on: Bool) {
        setNumber(MAKeys.rotaryControl, on ? (rotaryMask | bit) : (rotaryMask & ~bit)); commit()
    }

    /// Mixer keys saved in a preset and written back by load (everything the sidebar `settings` buttons cover; no clock, rate or firmware).
    static let presetKeys: [MAKey] = [
        MAKeys.swReturnLevels, MAKeys.swReturnMute, MAKeys.swReturnSolo, MAKeys.swReturnLink,
        MAKeys.outputLevels, MAKeys.outputMute, MAKeys.outputSolo, MAKeys.outputLink,
        MAKeys.inputLevels, MAKeys.inputMute, MAKeys.inputSolo, MAKeys.inputLink,
        MAKeys.headphoneLevels, MAKeys.headphoneMute, MAKeys.headphoneLink,
        MAKeys.auxSendLevels, MAKeys.auxOutputLevels, MAKeys.auxOutputMute, MAKeys.auxOutputLink, MAKeys.auxOutputPan2,
        MAKeys.monitorAuxOnOutput, MAKeys.inputPan2, MAKeys.inputRoutingV2, MAKeys.swReturnRoutingV2,
        MAKeys.globalDim, MAKeys.globalMute, MAKeys.rotaryControl,
        MAKeys.momentarySwitchRoutings, MAKeys.headphoneSourceV2, MAKeys.headphoneSecondSource, MAKeys.momentarySwitchSource,
    ]

    func presetSnapshot() -> [String: Any] {
        var d: [String: Any] = [:]
        for k in DeviceModel.presetKeys { if let v = cache[k.name] { d[k.name] = v } }
        return d
    }

    /// Writes a preset back in one batch.
    func restore(_ values: [String: Any]) {
        for k in DeviceModel.presetKeys { if let v = values[k.name] { queue(k, v) } }
        commit()
    }

    /// Headphone safety (user, 2026-10-20): the factory phones level is 0 dB (official reset, capture 92), so the start-up ceiling is written
    /// right after the reset. The kext runs the reset inside the same setProperties (ResetMixer), so this second write lands after it.
    func resetToFactory() {
        setNumber(MAKeys.resetToFactory, 1); commit()
        let n = ints(MAKeys.headphoneLevels).count
        guard n > 0 else { return }
        queue(MAKeys.headphoneLevels, Array(repeating: NSNumber(value: Int32(DeviceModel.phonesStartupCeiling)), count: n))
        commit()
    }
}
