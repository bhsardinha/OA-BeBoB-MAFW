// FWAudiophileStatus -- decode of the Audiophile's 60-byte status packet (module models/FWAudiophile), kernel-free for the host test.
// Reference: com_m_audio_FWAudiophileDevice::ReadHSCIData @0xba92 (asm 0xba92-0xbb99 = decompile; A1 Q-AP0b §2), TRotaryControl::
// RotaryValueChanged @0xc2ba (nibble 1 -> +step, 2 -> -step, else no change; step = 1, ctor asm 0xbee0/0xbeea). Live check: capture 93
// (w2 = FDF index 1..4, 0xFF unlocked; w3 = lock).
// Packet: quadlets 0-13 = hardware peaks (analog in 1-2, S/PDIF in 1-2, analog out 1-4, S/PDIF out 1-2, headphone L/R, aux L/R);
// quadlet 14 = status word, read as a host-order (little-endian) UInt32 s = w3<<24 | w2<<16 | w1<<8 | w0 of the wire bytes w0..w3.
#ifndef OA_FWAUDIOPHILESTATUS_H
#define OA_FWAUDIOPHILESTATUS_H

#include <stdint.h>

enum { kFWAudiophileHwPeaks = 14, kFWAudiophileStatusQuadlet = 14 };

struct FWAudiophileStatus {
    bool    locked;         // s > 0xFFFFFF (seta), i.e. w3 != 0
    uint8_t fdf;            // (s >> 16) & 0xFF = w2
    int32_t levelDelta;     // w1 low nibble: encoder 1 = Level Controller -> FWSettingsRotaryControl::UserRotatedHardwareKnob
    int32_t headphoneDelta; // w1 high nibble: encoder 2 = headphone -> SetLevelsFromKnob(bit 56, "MAHeadphoneLevels", 0x38, headphone group)
    bool    abPressed;      // (s << 24) > 0x0FFFFFFF, i.e. w0 bits 4-7 != 0 -> FWSettingsABSwitch::MomentarySwitchPressed
};

static inline int32_t FWRotaryDelta(uint32_t nibble) { return nibble == 1 ? 1 : nibble == 2 ? -1 : 0; }

static inline FWAudiophileStatus FWAudiophileDecodeStatus(uint32_t s)
{
    FWAudiophileStatus st;
    st.locked = s > 0xFFFFFFu;
    st.fdf = (uint8_t)((s >> 16) & 0xFF);
    st.levelDelta = FWRotaryDelta((s >> 8) & 0xF);
    st.headphoneDelta = FWRotaryDelta((s >> 12) & 0xF);
    st.abPressed = (uint32_t)(s << 24) > 0x0FFFFFFFu;
    return st;
}

#endif
