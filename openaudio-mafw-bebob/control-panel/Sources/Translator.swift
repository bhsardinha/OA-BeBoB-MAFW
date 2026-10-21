// AP-5.1 -- the generic translator of the panel (official CXDevice): per-model counts and the index rules that follow from them, plus the
// headphone-source record. Pure logic (no AppKit) so the host tests use it. Sources: A2-P13 §1, §4-§6 (FW1814), A2-PNAP §1-§2, §6 (Audiophile).
import Foundation

/// Translator counts: CXDevice vtable [+0x18] outputs, [+0x1c] mixer channels, [+0x5c] inputs, [+0x60] headphone channels
/// (FW1814 20 / 32 / 18 / 4, A2-P13 §1; Audiophile 10 / 10 / 4 / 2, PNAP §1). Every index rule below follows from them.
struct TranslatorCounts: Equatable {
    let outputs: Int, mixer: Int, inputs: Int, headphone: Int
    /// Output channel ids: 1...main = MAOutput, then the headphone channels, then the two aux-output channels (ChooseOutputBusAndChannelIndex).
    var mainOutputs: Int { return outputs - headphone - 2 }
}

/// The shared meter page `uint32 peak[52]` = [inputs | outputs | SW returns] (A2-P13 §6 for the 1814: 18 + 20 + 14; PNAP §6 for the Audiophile:
/// inputs 0-3, outputs 4-13, SW returns from 14). 1-based channel ids, as the reference's GetMixer/OutputPairChannelMap.
struct PeakMap: Equatable {
    let counts: TranslatorCounts
    func input(_ ch: Int) -> Int { return ch - 1 }
    func output(_ id: Int) -> Int { return counts.inputs + id - 1 }
    func swReturn(_ k: Int) -> Int { return counts.inputs + counts.outputs + k - 1 }
    func phones(_ ch: Int) -> Int { return output(counts.mainOutputs + ch) }
    func auxOut(_ ch: Int) -> Int { return output(counts.outputs - 2 + ch) }
}

/// Aux-send element of a mixer channel: the mixer channel id (inputs first, SW returns after; cookie 0x5ea, A2-P13 §4, PNAP §3).
extension TranslatorCounts {
    func auxSendElement(input ch: Int) -> Int { return ch }
    func auxSendElement(swReturn ch: Int) -> Int { return inputs + ch }
}

/// One headphone source as the kext takes it (a mask, MAHeadphoneSourceV2 / MAMomentarySwitchRoutings) and as the panel names it.
struct PhonesSource: Equatable {
    let mask: UInt32
    let button: String       // strip "monitor" button
    let name: String         // A/B sheet radio
    let short: String        // a/b note text
}

/// Peak page -> dB exactly as `CXDevice::GetHWSetting` cookie 0x5e9/0x5f3 (A2-P9 §7.5, constants read from the panel binary 2026-10-16):
/// raw 0 -> -64; else ceil(20 * log10(raw * 2^-15)) (data at 0xf8b40 = 2^-15, 0xf8b48 = 20.0). raw 32768 (full-scale 16-bit peak) = 0 dB.
func peakToDB(_ raw: UInt32) -> Int {
    if raw == 0 { return -64 }
    let v = ceil(20.0 * log10(Double(raw) * 0.000030517578125))
    return Int(max(-64.0, v))     // the reference does not clamp non-zero values; clamping only affects values below -64 dB
}
