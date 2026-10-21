#include "FWSamples.h"

// floor() for a float already clamped into the SInt32 range.
static inline SInt32 FloorToInt(float v)
{
    SInt32 i = (SInt32)v;                    // truncates toward zero
    if ((float)i > v) i--;
    return i;
}

void FWFloat32ToInt24Scaled(const float *src, SInt32 *dst, UInt32 n)
{
    for (UInt32 i = 0; i < n; i++) {
        float v = src[i] * 8388608.0f + 0.5f;
        if (v < -2147483648.0f) v = -2147483648.0f;      // ref minps/maxps constants at 0x2ff20 / 0x2ff30
        if (v > 2147483520.0f)  v = 2147483520.0f;
        dst[i] = FloorToInt(v);
    }
}

void FWInt24ScaledToFloat32(const SInt32 *src, float *dst, UInt32 n)
{
    for (UInt32 i = 0; i < n; i++)
        dst[i] = (float)src[i] * 1.1920928955078125e-07f;   // 2^-23, ref constant at 0x2fe80
}

template <UInt32 N>
static void RearrangeLRLR(const SInt32 *src, UInt32 *dst, UInt32 frames)
{
    for (UInt32 f = 0; f < frames; f++) {
        for (UInt32 c = 0; c < N; c++) dst[c] = FWPackAM824(src[c]);
        src += N;
        dst += N + 1;                                    // the MIDI quadlet of the frame is skipped, not written
    }
}

FWRearrangeProc FWRearrangeProcForChannels(UInt32 channels)
{
    switch (channels) {
        case 4:  return RearrangeLRLR<4>;                // ref _RearrangeSampleFrameLRLR4
        case 6:  return RearrangeLRLR<6>;                // ref ...LRLR6SkipMIDI
        case 8:  return RearrangeLRLR<8>;
        case 10: return RearrangeLRLR<10>;
        case 12: return RearrangeLRLR<12>;
        case 14: return RearrangeLRLR<14>;
        default: return 0;
    }
}

void FWConvertInputOneStereoPairSkipMIDI(UInt32 count, const UInt32 *src, SInt32 *dst, void *)
{
    for (UInt32 i = 0; i < count; i += 2) {
        dst[0] = FWUnpackAM824(src[0]);
        dst[1] = FWUnpackAM824(src[1]);
        src += 3;
        dst += 2;
    }
}

void FWPeaksMax(const SInt32 *src, SInt32 *dst, UInt32 frames, UInt32 channels, const UInt8 *map)
{
    for (UInt32 g = 0; g < (frames >> 3); g++) {
        for (UInt32 c = 0; c < channels; c++) {
            const SInt16 x = (SInt16)((UInt32)src[c] >> 8);
            const SInt16 sign = (SInt16)(x >> 15);
            const SInt16 a = (SInt16)((SInt16)(x + sign) ^ sign);
            const SInt16 old = (SInt16)dst[map[c]];
            dst[map[c]] = (a < old) ? (SInt32)old : (SInt32)a;
        }
        src += channels * 8;
    }
}

// ---- LLRR family ---------------------------------------------------------------------------------------------------------------------

void FWConvertLLRR4SkipMIDI1(UInt32 count, const UInt32 *src, SInt32 *dst, void *)
{
    for (UInt32 i = 0; i < count; i += 4) {                                  // ref: edi -= 4 per frame until 0
        const SInt32 w0 = FWUnpackAM824(src[0]), w1 = FWUnpackAM824(src[1]), w2 = FWUnpackAM824(src[2]), w3 = FWUnpackAM824(src[3]);
        dst[0] = w1; dst[1] = w3; dst[2] = w0; dst[3] = w2;                  // stores at 0x28f87..0x28f91
        src += 5;
        dst += 4;
    }
}

void FWRearrangeLLRR6(const SInt32 *src, UInt32 *dst, UInt32 frames)
{
    static const UInt8 p[6] = { 0, 2, 4, 1, 3, 5 };
    for (UInt32 f = 0; f < frames; f++) {
        for (UInt32 k = 0; k < 6; k++) dst[k] = FWPackAM824(src[p[k]]);
        src += 6;
        dst += 7;
    }
}

static const UInt8 kIdentity6[6] = { 0, 1, 2, 3, 4, 5 };
void FWSamplePeaks6(const SInt32 *src, SInt32 *dst, UInt32 frames) { FWPeaksMax(src, dst, frames, 6, kIdentity6); }

