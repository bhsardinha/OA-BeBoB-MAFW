// FW1814MixerRegs -- the FW1814 mixer / control register map (module models/FW1814) (A1 Q15f + Q-ME §2/§4). All offsets are byte offsets in the vendor register
// window at FireWire address 0xFFC7:0x00700000 written with CustomMixerWriteQuads (ref FWAVCAudioDevice @0x22c7a); values are
// big-endian int16 pairs (level = property level * 0x80). Groups confirmed against FFADO special_avdevice.h:55-69 (wire truth only).
#ifndef OA_FW1814MIXERREGS_H
#define OA_FW1814MIXERREGS_H

#include <libkern/OSTypes.h>

enum {
    kFWMixRegSWReturn      = 0x00,     // 4 ch  (2 quadlets)   ref group +0x1668
    kFWMixRegOutput        = 0x08,     // 4 ch  (2 quadlets)   ref group +0x1318
    kFWMixRegInput         = 0x10,     // 18 ch (9 quadlets)   ref group +0x13e0
    kFWMixRegAuxOutput     = 0x34,     // 2 ch  (1 quadlet)    ref group +0x1b00
    kFWMixRegHeadphone     = 0x38,     // 4 ch  (2 quadlets)   ref group +0x1730
    kFWMixRegPan           = 0x40,     // 9 quadlets, two 16-bit pan words each
    kFWMixRegAuxSend       = 0x64,     // 22 ch (11 quadlets), written rotated (CustomMixerWriteWithRotatedAuxChannels)
    kFWMixRegInputRouting  = 0x90,     // 1 quadlet (shadow +0x1308)
    kFWMixRegHeadphoneSel  = 0x98,     // 1 quadlet (shadow +0x1310)
    kFWMixRegAuxMonitor    = 0x9c      // 1 quadlet (shadow +0x1314)
};

#include "FWRegisterWindows.h"

#endif
