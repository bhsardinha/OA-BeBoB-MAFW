// FW1814Tables -- the FW1814's per-rate data (module models/FW1814). Reference: FW1814Device::GetDeviceChannelsForSampleRate @0xc998 and
// GetRoundTripLatencyForFDF @0xcf30 (decompiles FW1814Device__*.c; A1 Q10, Q-P4 Part A). Tables read from the binary: rates @0x2f890,
// DeviceChannels @0x363c0 / 0x364a0 / 0x36580 / 0x4ab80. Layout of a row: FWDeviceChannels in base/FWRates.h.
#ifndef OA_FW1814TABLES_H
#define OA_FW1814TABLES_H

#include "FWRates.h"

// ref @0x2f890: the six rates the FW1814 offers (44.1 .. 192 kHz). 32 kHz is NOT offered (its CIP row exists only as the
// SetupCIP fall-through table).
static const FWSampleRateEntry kFW1814Rates[7] = {
    { 44100, 0 }, { 48000, 0 }, { 88200, 0 }, { 96000, 0 }, { 176400, 0 }, { 192000, 0 }, { 0, 0 }
};

// Group words are per table, two groups from the input side and two from the output side (see above).
#define FWDC_HEAD(i, o) { i, i, o, o, 1, 0, {0, 0, 0}, kFW1814Rates, kFW1814Rates,
static const FWDeviceChannels kFW1814SPDIF44_96 = FWDC_HEAD(10, 6)  { {1,1,4,0}, {1,5,5,1}, {0,1,2,0}, {0,3,3,1}, {0,0,0,3} } };   // @0x363c0
static const FWDeviceChannels kFW1814ADAT44_48  = FWDC_HEAD(16, 12) { {1,1,4,0}, {1,5,8,2}, {0,1,2,0}, {0,3,6,2}, {0,0,0,3} } };   // @0x364a0
static const FWDeviceChannels kFW1814ADAT88_96  = FWDC_HEAD(12, 8)  { {1,1,4,0}, {1,5,6,2}, {0,1,2,0}, {0,3,4,2}, {0,0,0,3} } };   // @0x36580
static const FWDeviceChannels kFW1814_176_192    = FWDC_HEAD(2, 4)   { {1,1,1,0}, {0,1,2,0}, {0,0,0,3}, {0,0,0,0}, {0,0,0,0} } };   // @0x4ab80
#undef FWDC_HEAD

// ref GetDeviceChannelsForSampleRate @0xc998. `mode` = the device mode byte [+0x144] (MAClockSource bits): bit 3 (0x08)
// selects the ADAT table for the INPUT fields, bit 4 (0x10) for the OUTPUT fields. rate >= 176.4 kHz ignores the mode.
static inline void FW1814ComposeDeviceChannels(UInt32 rateHz, UInt32 mode, FWDeviceChannels &o)
{
    if (rateHz == 176400 || rateHz == 192000) { o = kFW1814_176_192; return; }
    const FWDeviceChannels &adat = (rateHz == 88200 || rateHz == 96000) ? kFW1814ADAT88_96 : kFW1814ADAT44_48;
    const FWDeviceChannels &inT  = (mode & 0x08) ? adat : kFW1814SPDIF44_96;
    const FWDeviceChannels &outT = (mode & 0x10) ? adat : kFW1814SPDIF44_96;
    o = inT;                                                    // +0x00/+0x04, groups 0,1, midi, rates, clip offsets
    o.out = outT.out; o.outStream = outT.outStream;             // +0x08/+0x0c
    o.group[2][0] = outT.group[2][0]; o.group[2][1] = outT.group[2][1]; o.group[2][2] = outT.group[2][2]; o.group[2][3] = outT.group[2][3];
    o.group[3][0] = outT.group[3][0]; o.group[3][1] = outT.group[3][1]; o.group[3][2] = outT.group[3][2]; o.group[3][3] = outT.group[3][3];
}

// ref FW1814Device::GetRoundTripLatencyForFDF @0xcf30 (frames, Q10): FDF index -> value, default 210.
static inline UInt32 FW1814RoundTripLatencyForFDF(UInt32 fdf)
{
    switch (fdf) { case 2: return 0xe1; case 3: return 0x166; case 4: return 0x17a; case 5: return 0x282; case 6: return 0x2ba; default: return 0xd2; }
}
#endif
