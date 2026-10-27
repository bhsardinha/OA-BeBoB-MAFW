// FW410Status -- decode of the FireWire 410's 76-byte status packet (module models/FW410), kernel-free for the host test.
// Reference: com_m_audio_FW410Device::ReadHSCIData @0x15198 (decompile), GetControlPacketSize 0x4c. Live: capture 105 (quadlets 4-13 = the
// hardware peaks of outputs 1-10 at 96 kHz, 4-11 = outputs 1-8 at 192 kHz).
// Packet: quadlets 0-3 = hardware input peaks (analog 1/2, S/PDIF L/R) -> meter slots 0-3; quadlets 4-17 = hardware output peaks (outputs
// 1-10, then two more pairs) -> slots 14-27; slots 4-13 = SW returns (host peaks, _SamplePeaks10); quadlet 18 = status word (host order).
#ifndef OA_FW410STATUS_H
#define OA_FW410STATUS_H

#include <stdint.h>

enum { kFW410HwInPeaks = 4, kFW410HwOutPeaks = 14, kFW410FirstHwOutSlot = 14, kFW410StatusQuadlet = 18, kFW410SWReturnSlots = 10 };

struct FW410Status {
    bool    locked;     // (s >> 24) & 1
    uint8_t fdf;        // (s >> 16) & 7
    uint8_t encoder;    // (s >> 8) & 0xff -> TRotaryControl::RotaryValueChanged (1 = +1, 2 = -1, else no change; FWRotaryDelta)
};

static inline FW410Status FW410DecodeStatus(uint32_t s)
{
    FW410Status st;
    st.locked = ((s >> 24) & 1) != 0;
    st.fdf = (uint8_t)((s >> 16) & 7);
    st.encoder = (uint8_t)((s >> 8) & 0xff);
    return st;
}

#endif
