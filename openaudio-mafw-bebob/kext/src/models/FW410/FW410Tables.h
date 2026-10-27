// FW410Tables -- the FireWire 410's per-rate data (module models/FW410). Reference: com_m_audio_FW410Device GetDeviceChannelsForSampleRate
// (asm 0x14010/0x14017: rate > 96000 -> sDeviceChannels192 @0x4ad40, else sDeviceChannels @0x4ae20), GetSupportedInput/OutputSampleRates
// (kSupportedInputRatesArray @0x2fa10, both directions), GetRoundTripLatencyForFDF (decompile). Bytes read from the v1.11.2 x86_64 slice
// (A1 Q-410-0 §2, R135/R136). Live: capture 103 (4/10 at 44.1-96 kHz, 2/8 at 176.4/192 kHz), capture 105 (all 8 outputs stream at 192 kHz).
#ifndef OA_FW410TABLES_H
#define OA_FW410TABLES_H

#include "FWRates.h"

// @0x2fa10: 44.1, 48, 88.2, 96, 176.4, 192 kHz, zero-terminated (no 32 kHz, although the early manual lists it)
static const FWSampleRateEntry kFW410Rates[7] = { { 44100, 0 }, { 48000, 0 }, { 88200, 0 }, { 96000, 0 }, { 176400, 0 }, { 192000, 0 }, { 0, 0 } };

// 176.4/192 kHz channel maps (@0x2f9e0, @0x2f9f0; R136): input wire slot -> host channel, output host channel -> wire slot (wire = L1-L4,
// R1-R4, MIDI). Used by the base's ConvertSampleFramesFromInput / RemapSampleFramesForOutput instead of the model's procs.
static const UInt32 kFW410InputMap192[2]  = { 0, 1 };
static const UInt32 kFW410OutputMap192[8] = { 0, 4, 1, 5, 2, 6, 3, 7 };

// @0x4ae20 (44.1-96 kHz): in 4 / 4, out 10 / 10, MIDI 1, no maps; groups: input pair 1 analog, input pair 2 S/PDIF, output pairs 1-4 analog,
// output pair 5 S/PDIF.
static const FWDeviceChannels kFW410Channels = {
    4, 4, 10, 10, 1, 0, { 0, 0, 0 }, kFW410Rates, kFW410Rates,
    { { 1, 1, 1, 0 }, { 1, 2, 2, 1 }, { 0, 1, 4, 0 }, { 0, 5, 5, 1 }, { 0, 0, 0, 3 } }
};
// @0x4ad40 (176.4/192 kHz): in 2 / 2, out 8 / 8, MIDI 1, maps above, no copy list. The reference keeps the <=96 kHz group list here (S/PDIF
// pairs beyond the channel count); kept as read.
static const FWDeviceChannels kFW410Channels192 = {
    2, 2, 8, 8, 1, 0, { kFW410InputMap192, kFW410OutputMap192, 0 }, kFW410Rates, kFW410Rates,
    { { 1, 1, 1, 0 }, { 1, 2, 2, 1 }, { 0, 1, 4, 0 }, { 0, 5, 5, 1 }, { 0, 0, 0, 3 } }
};

// GetRoundTripLatencyForFDF (frames): 2 -> 0xa0, 3 -> 0xfe, 4 -> 0xf5, 5 -> 0x1b8, 6 -> 0x1e0, anything else -> 0x96
static inline UInt32 FW410RoundTripLatencyForFDF(UInt32 fdf)
{
    switch (fdf) { case 2: return 0xa0; case 3: return 0xfe; case 4: return 0xf5; case 5: return 0x1b8; case 6: return 0x1e0; default: return 0x96; }
}

#endif
