// FWSamples -- the sample converters of the reference (A1 Q-MD2; decomp _Float32ToNativeInt32_X86.c, _NativeInt32ToFloat32_X86.c,
// _RearrangeSampleFrameLRLR*.c, _ConvertInputFramesOneStereoPairSkipMIDI.c). The reference writes these in SSE assembly; the clone
// writes the same arithmetic in C. Wire format of a data quadlet (AM824 MBLA, big-endian on the wire, kept in wire order in the packet
// pool): byte0 = label 0x40, byte1..3 = the 24-bit sample, most significant byte first.
#ifndef OA_FWSAMPLES_H
#define OA_FWSAMPLES_H

#include <libkern/OSTypes.h>

// ref _Float32ToNativeInt32_X86 @0x2d13d: v = x * 2^23 + 0.5, clamp to [-2^31, 2147483520.0], convert. The reference sets MXCSR
// rounding to "toward -infinity" around the SIMD loop (stmxcsr/ldmxcsr at 0x2d14e..0x2d16a), i.e. floor(v). The result is a 24-bit
// sample scaled so that full scale = +/-0x800000 (the rearrange step clamps to 24 bits).
void FWFloat32ToInt24Scaled(const float *src, SInt32 *dst, UInt32 n);

// ref _NativeInt32ToFloat32_X86 @0x2c8e0: x * 2^-23.
void FWInt24ScaledToFloat32(const SInt32 *src, float *dst, UInt32 n);

// ref _RearrangeSampleFrameLRLR{4,6SkipMIDI,8SkipMIDI,10SkipMIDI,12SkipMIDI,14SkipMIDI}: per frame, N samples (source stride N) are
// clamped to 24 bits and written as AM824 quadlets into N consecutive destination quadlets; the destination frame stride is N+1
// (the MIDI quadlet at the end of each frame is left untouched). Channel order is unchanged (checked for all six against the decompiles).
typedef void (*FWRearrangeProc)(const SInt32 *src, UInt32 *dst, UInt32 frames);
FWRearrangeProc FWRearrangeProcForChannels(UInt32 channels);        // NULL for any count the reference has no proc for

// Input converters ([+0x310] in the reference): (count = channels * frames, source quadlets, destination samples, per-proc argument).
typedef void (*FWInputConvertProc)(UInt32 count, const UInt32 *src, SInt32 *dst, void *arg);

// One AM824 quadlet in wire order (as the little-endian host reads it) -> signed 24-bit sample.
static inline SInt32 FWUnpackAM824(UInt32 x)
{
    return (SInt32)(((x & 0xff00u) << 16) | (x & 0xff0000u) | ((x >> 24) << 8)) >> 8;
}
// Signed 24-bit sample (clamped) -> one AM824 data quadlet in wire order (label 0x40).
static inline UInt32 FWPackAM824(SInt32 v)
{
    if (v > 0x7fffff) v = 0x7fffff;
    if (v < -0x800000) v = -0x800000;
    UInt32 u = (UInt32)v;
    return (u << 24) + ((u & 0xff00u) << 8) + ((u >> 8) & 0xff00u) + 0x40u;
}

// ref _ConvertInputFramesOneStereoPairSkipMIDI @0x2911b: 2 channels per frame, source stride 3 (the third quadlet is MIDI).
void FWConvertInputOneStereoPairSkipMIDI(UInt32 count, const UInt32 *src, SInt32 *dst, void *arg);

// ref _Sample1814Peaks{4All,SPDIF6,ADAT8,ADAT12} (decomp _Sample1814Peaks*.c): output peak meters of the host stream, installed on the engine
// ([+0x308]) by FW1814Device::SwitchDigitalSignals by output-channel count (6, 8, 12, else 4All). Every 8th frame is sampled; the sample
// is truncated to its top 16 bits (bits 8..23 of the scaled int), made absolute with the 16-bit trick (so -32768 stays negative and
// never registers) and max-merged into dst[map[c]]. `dst` is cleared by the caller before each clip.
typedef void (*FWPeakProc)(const SInt32 *src, SInt32 *dst, UInt32 frames);
FWPeakProc FWPeakProcForChannels(UInt32 outChannels);

#endif
