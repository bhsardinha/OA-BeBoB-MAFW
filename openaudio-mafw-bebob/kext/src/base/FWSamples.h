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

// Output peak-meter proc installed on the engine ([+0x308]) by the model (FW1814: _Sample1814Peaks*, models/FW1814/FW1814Peaks.h).
// The reference's peak procs share one arithmetic (decomp _SamplePeaks6.c, _Sample1814Peaks*.c): every 8th frame, per channel
// |(int16)(sample >> 8)| (the 16-bit absolute trick, so -32768 never registers) max-merged into dst[map[c]]; `dst` is cleared by the caller.
void FWPeaksMax(const SInt32 *src, SInt32 *dst, UInt32 frames, UInt32 channels, const UInt8 *map);
typedef void (*FWPeakProc)(const SInt32 *src, SInt32 *dst, UInt32 frames);

// ---- LLRR family (Rev2 models; A1 Q-AP0d, R80). Generic reference routines, installed by the model's SetupAudioEngine. ----------------
// ref _ConvertLLRR4SkipMIDI1 @0x28eeb (asm 0x28eff-0x28fa6): `count` = samples (4 per frame); per frame 5 wire quadlets (slot 4 = MIDI,
// skipped), each unpacked like FWUnpackAM824; host frame = [w1, w3, w0, w2].
void FWConvertLLRR4SkipMIDI1(UInt32 count, const UInt32 *src, SInt32 *dst, void *arg);
// ref _RearrangeSampleFrameLLRR6 @0x2c0e5 (asm 0x2c100-0x2c2af): per frame wire[k] = pack(host[p[k]]), p = 0 2 4 1 3 5; host stride 6,
// wire stride 7 (the MIDI quadlet is not written). Clone guard: frames == 0 returns (the reference's decl/incq count would loop 2^32 times).
void FWRearrangeLLRR6(const SInt32 *src, UInt32 *dst, UInt32 frames);
// ref _SamplePeaks6 @0x27e17 (decompile): six channels, identity map
void FWSamplePeaks6(const SInt32 *src, SInt32 *dst, UInt32 frames);
// ref _RearrangeFrameLLRR10WithMIDI @0x2b951 (asm, mafw.asm 51942-52130): wire[k] = pack(host[p[k]]), p = 8 0 2 4 6 9 1 3 5 7 (S/PDIF L, L1-L4,
// S/PDIF R, R1-R4); host stride 10, wire stride 11 (the MIDI quadlet is not written). frames == 0 returns (the reference would loop 2^32 times).
void FWRearrangeLLRR10WithMIDI(const SInt32 *src, UInt32 *dst, UInt32 frames);
// ref _SamplePeaks10 @0x280d3 (decompile): ten channels, identity map
void FWSamplePeaks10(const SInt32 *src, SInt32 *dst, UInt32 frames);
// ---- channel-map routines (DeviceChannels +0x18/+0x20/+0x28; R136) ----
// ref ConvertSampleFramesFromInput @0x27bb6 (decompile): per frame, wire slot k -> dst[frame * inStream + map[k]] (skipped when map[k] >= inStream);
// wire stride in + (midi ? 1 : 0).
void FWConvertSampleFramesFromInput(const UInt32 *src, SInt32 *dst, UInt32 in, UInt32 inStream, UInt32 frames, const UInt32 *map, bool midi);
// ref RemapSampleFramesForOutput @0x27aad (asm): per frame, host channel h -> wire[map[h]] (skipped when map[h] >= out + midi), packed as
// FWPackAM824; then the copy list (pairs a, b; terminated by 0xFFFFFFFF) copies wire[a] to wire[b] inside the frame. Host stride outStream,
// wire stride out + midi. frames == 0 returns.
void FWRemapSampleFramesForOutput(const SInt32 *src, UInt32 *dst, UInt32 out, UInt32 outStream, UInt32 frames, const UInt32 *map, const UInt32 *copy, bool midi);

#endif
