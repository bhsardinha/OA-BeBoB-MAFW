// FWAudiophileTables -- the FireWire Audiophile's per-rate data (module models/FWAudiophile). Reference: com_m_audio_FWAudiophileDevice
// GetDeviceChannelsForSampleRate @0xafb4 (one table for every rate: sDeviceChannels @0x4aaa0), GetSupportedInput/OutputSampleRates
// (kSupportedSampleRatesArray @0x2f720), GetRoundTripLatencyForFDF (asm 0xafde-0xb00c). Bytes read from the v1.11.2 x86_64 slice
// (A1 Q-AP0 §2.4, §6; R85; re-read 2026-10-20). Row layout: FWDeviceChannels in base/FWRates.h.
#ifndef OA_FWAUDIOPHILETABLES_H
#define OA_FWAUDIOPHILETABLES_H

#include "FWRates.h"

// @0x2f720: 44.1, 48, 88.2, 96 kHz, zero-terminated (no 32 kHz, no 176.4/192 kHz)
static const FWSampleRateEntry kFWAudiophileRates[5] = { { 44100, 0 }, { 48000, 0 }, { 88200, 0 }, { 96000, 0 }, { 0, 0 } };

// @0x4aaa0: in 4 / inStream 4 / out 6 / outStream 6 / MIDI 1, clip offsets 0, rates @0x2f720 both ways; groups {isInput, first pair, last pair,
// signal type 0 analog / 1 S/PDIF / 3 end}: input pair 1 analog, input pair 2 S/PDIF, output pairs 1-2 analog, output pair 3 S/PDIF.
static const FWDeviceChannels kFWAudiophileChannels = {
    4, 4, 6, 6, 1, 0, { 0, 0, 0 }, kFWAudiophileRates, kFWAudiophileRates,
    { { 1, 1, 1, 0 }, { 1, 2, 2, 1 }, { 0, 1, 2, 0 }, { 0, 3, 3, 1 }, { 0, 0, 0, 3 } }
};

// GetRoundTripLatencyForFDF (frames): FDF 2 -> 0xc4, 3 -> 0x14a, 4 -> 0x162, anything else -> 0xb6
static inline UInt32 FWAudiophileRoundTripLatencyForFDF(UInt32 fdf)
{
    switch (fdf) { case 2: return 0xc4; case 3: return 0x14a; case 4: return 0x162; default: return 0xb6; }
}

#endif
