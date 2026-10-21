// Per-rate CIP constants. Source: Q2 "Per-rate dispatch" (dispatch @0x272f6 -> SetupCIP @0x27102), verified against
// the cmpl immediates 0x2730b..0x2738b. FDF is the enum 0..6 (ChangeSampleRateToFDFValue @0x52e8), not the hex rate.
// Unknown rates fall through to the 44.1 kHz row ($0xac44 default at 0x2738b).
#ifndef OA_FWRATES_H
#define OA_FWRATES_H

#include <IOKit/IOTypes.h>

struct FWRateInfo {
    UInt32 rateHz;
    UInt8  fdf;            // FDF index 0..6
    UInt8  sytInterval;    // data blocks per packet: 8 / 16 / 32
    UInt16 sytIncrement;
};

static const FWRateInfo kFWRates[7] = {
    {  32000, 0,  8, 0x1800 },
    {  44100, 1,  8, 0x116A },
    {  48000, 2,  8, 0x1000 },
    {  88200, 3, 16, 0x116A },
    {  96000, 4, 16, 0x1000 },
    { 176400, 5, 32, 0x116A },
    { 192000, 6, 32, 0x1000 },
};

static inline const FWRateInfo &FWRateLookup(UInt32 rateHz)
{
    for (unsigned i = 0; i < 7; i++)
        if (kFWRates[i].rateHz == rateHz) return kFWRates[i];
    return kFWRates[1];    // ref default: 44.1 kHz
}

// TX packets per bank P = floor(65,536,000 / (rate*4)) + 2 (SetupCIP @0x27113..0x27133, Q-DCL).
static inline UInt32 FWTxPacketsPerBank(UInt32 rateHz) { return 0x3E80000u / (rateHz * 4u) + 2u; }


// ---------------------------------------------------------------------------------------------------------------------
// M-D: per-rate device parameters, generic part (A1 Q10, Q-P4 Part A; decompiles FWAudioDevice__GetSafetyOffsetForFDF.c,
// FWAudioDevice__GetBandwidthBytesAtSampleRate.c). Each model's own rate list, DeviceChannels rows and latency table live in its module
// (models/<Model>/<Model>Tables.h), as the reference keeps them per model class (base/model split, plan v4 AP-2.5).
// ---------------------------------------------------------------------------------------------------------------------

// IOAudioSampleRate-shaped entry (whole, fraction); the lists are zero-terminated (FWAudioDevice::FetchIndexIntoSampleRateArray).
struct FWSampleRateEntry { UInt32 whole; UInt32 fraction; };

// m_audio_b_FWAudioDevice::DeviceChannels, 0x80 bytes. Field meanings from the readers: SetIOFormat @0x42aa copies
// out -> engine[+0x2e0], outStream -> [+0x2ec], in -> [+0x2e4], inStream -> [+0x2f0], clip offsets -> [+0x320..+0x330];
// SetStreamFormat uses +0xc / +0x38 for the output stream and +0x4 / +0x30 for the input stream. The four 4-word groups at
// +0x40 are copied by GetDeviceChannelsForSampleRate (groups 0,1 from the INPUT table, 2,3 from the OUTPUT table); their
// meaning is NOT decoded (probably per-channel-pair signal types, GetSignalTypeForChannelPair) -- carried verbatim.
struct FWDeviceChannels {
    UInt32 in;                // +0x00
    UInt32 inStream;          // +0x04
    UInt32 out;               // +0x08
    UInt32 outStream;         // +0x0c
    UInt32 midi;              // +0x10  MIDI quadlets per data block (m)
    UInt32 pad14;             // +0x14
    UInt64 clipOffset[3];     // +0x18 +0x20 +0x28  (0 for the FW1814)
    const FWSampleRateEntry *inRates;    // +0x30
    const FWSampleRateEntry *outRates;   // +0x38
    UInt32 group[5][4];       // +0x40: {isInput, firstPair, lastPair, signalType 0 analog / 1 S/PDIF / 2 ADAT / 3 end-of-list} x4 + terminator
};

// ref FWRev1AudioDevice::GetRoundTripLatencyForFDF @0x136da (asm 0x136da-0x1370a): FDF 1-2 -> 0xba, 3 -> 0x106, 4-6 -> 0x124, else 0
static inline UInt32 FWRev1RoundTripLatencyForFDF(UInt32 fdf)
{
    if (fdf - 4 < 3) return 0x124;
    if (fdf == 3) return 0x106;
    if (fdf - 1 <= 1) return 0xba;
    return 0;
}
// ref FWAudioDevice::GetSafetyOffsetForFDF @0x1d58 (frames): feeds setSampleOffset in ResetLatency. Default 0x2c.
static inline UInt32 FWSafetyOffsetForFDF(UInt32 fdf)
{
    switch (fdf) { case 2: return 0x30; case 3: return 0x58; case 4: return 0x60; case 5: return 0xb0; case 6: return 0xc0; default: return 0x2c; }
}
// ref FWAudioDevice::GetBandwidthBytesAtSampleRate @0x1ee2: (in + out + 2*m) * (SYT * 4) + 16 bytes per cycle, SYT = the
// engine's CURRENT SYT interval ([engine+0x224]) or 8 when there is no engine (0x20 / 4).
static inline UInt32 FWBandwidthBytesPerCycle(const FWDeviceChannels &ch, UInt32 engineSytInterval)
{
    const UInt32 syt4 = engineSytInterval ? ((engineSytInterval & 0xff) << 2) : 0x20;
    return (ch.out + ch.in + ch.midi * 2) * syt4 + 0x10;
}
// ref FWBaseEngine::ChangeSampleRateToFDFValue @0x52e8 / PerformSampleRateChangeCore @0x479e: FDF index <-> rate.
static inline UInt32 FWRateForFdfIndex(UInt32 fdf, bool *valid)
{
    if (valid) *valid = fdf < 7;
    return fdf < 7 ? kFWRates[fdf].rateHz : 0;
}

#endif
