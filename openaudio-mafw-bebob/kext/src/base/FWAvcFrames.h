// FWAvcFrames -- byte layouts of the Rev2 AV/C commands (FWRev2AudioDevice, A1 Q-AP0 §2.1-§2.2, R77), as pure functions with no kernel
// dependency so tests/host/avc_frames_test.cpp checks the same code the kext sends. Each function cites the reference instruction range.
#ifndef OA_FWAVCFRAMES_H
#define OA_FWAVCFRAMES_H

#include <stdint.h>
#include <string.h>
#ifdef KERNEL
#include <libkern/libkern.h>   // snprintf
#else
#include <stdio.h>
#endif

// ref FWRev2AudioDevice::AVCControlPlugSignalFormat @0x12e7e, switch on the FDF: AV/C sampling-frequency code (FFADO avc_definitions.h:115-125)
static inline uint8_t FWAvcRev2RateCode(uint32_t fdf)
{
    switch (fdf) {
    case 0: return 0x02;    // 32 kHz
    case 2: return 0x04;    // 48 kHz
    case 3: return 0x0A;    // 88.2 kHz
    case 4: return 0x05;    // 96 kHz
    case 5: return 0x06;    // 176.4 kHz
    case 6: return 0x07;    // 192 kHz
    default: return 0x03;   // 44.1 kHz (FDF 1 and anything else)
    }
}

enum { kFWAvcRev2ListLen = 14, kFWAvcRev2ReplyLen = 0x200 };

// ref asm 0x12f0b-0x12f7c: EXTENDED STREAM FORMAT INFORMATION STATUS, subfunction C1 (list), entry `idx`:
//   01 FF 2F C1 <dir'> 00 00 <plug> FF FF <idx> + 3 bytes, sent with length 14. dir' = (direction argument == 0) (sete %bl @0x12f01).
// Deviation: the reference leaves bytes 11-13 as uninitialised stack; the clone sends zeros (Q-AP0 §2.1 clone note, unverified on hardware).
static inline void FWAvcRev2FormatListFrame(uint8_t out[kFWAvcRev2ListLen], uint32_t direction, uint8_t plug, uint8_t idx)
{
    memset(out, 0, kFWAvcRev2ListLen);
    out[0] = 0x01; out[1] = 0xFF; out[2] = 0x2F; out[3] = 0xC1; out[4] = (uint8_t)(direction == 0);
    out[5] = 0x00; out[6] = 0x00; out[7] = plug; out[8] = 0xFF; out[9] = 0xFF; out[10] = idx;
}

// ref asm 0x12f7e-0x1302b: CONTROL, subfunction C0 (set), built from the matching list reply:
//   00 FF 2F C0 <dir'> 00 00 <plug> FF FF 90 40 <rate> 01 <reply[15]> <2*reply[15] bytes of reply[16..]> 00 00 00
// length = 2*reply[15] + 0x12, zero-filled, memmove(cmd+14, reply+15, 2*reply[15]+1). Returns the length (0 if `cap` is too small).
static inline uint32_t FWAvcRev2FormatSetFrame(uint8_t *out, uint32_t cap, uint32_t direction, uint8_t plug, uint8_t rate, const uint8_t *reply)
{
    const uint32_t n = reply[15];
    const uint32_t len = 2 * n + 0x12;
    if (len > cap) return 0;
    memset(out, 0, len);
    out[0] = 0x00; out[1] = 0xFF; out[2] = 0x2F; out[3] = 0xC0; out[4] = (uint8_t)(direction == 0);
    out[5] = 0x00; out[6] = 0x00; out[7] = plug; out[8] = 0xFF; out[9] = 0xFF;
    out[10] = 0x90; out[11] = 0x40; out[12] = rate; out[13] = 0x01;
    memmove(out + 14, reply + 15, 2 * n + 1);
    return len;
}

// ref FWRev2AudioDevice::GetAddressAndPlugForSyncType @0x12ae8 (decompile): the SIGNAL SOURCE source address / plug per sync type.
// Returns 0, or 0xE00002C7 (unsupported) for an unknown type.
static inline uint32_t FWAvcRev2SyncSource(uint32_t type, uint8_t plug, uint8_t *addr, uint8_t *srcPlug)
{
    switch (type) {
    case 0: *addr = 0x60; *srcPlug = (plug == 1) ? 1 : (uint8_t)(plug + 1); return 0;   // music subunit (internal clock)
    case 1: *addr = 0xFF; *srcPlug = 0x81; return 0;                                    // unit external plug 1
    case 2: *addr = 0xFF; *srcPlug = 0x00; return 0;                                    // unit isochronous plug 0
    case 3: *addr = 0xFF; *srcPlug = 0x83; return 0;                                    // unit external plug 3
    case 4: *addr = 0xFF; *srcPlug = 0x82; return 0;                                    // unit external plug 2 (Audiophile S/PDIF)
    default: return 0xE00002C7u;
    }
}

// ref FWRev2AudioDevice::AVCSetupSyncStream @0x12c60: SIGNAL SOURCE (0x1A) STATUS to the music subunit plug, then CONTROL with the source
//   STATUS  01 FF 1A 0F FF FE 60 <plug>
//   CONTROL 00 FF 1A 0F <addr> <srcPlug> 60 <plug>
static inline void FWAvcSignalSourceStatusFrame(uint8_t out[8], uint8_t plug)
{
    out[0] = 0x01; out[1] = 0xFF; out[2] = 0x1A; out[3] = 0x0F; out[4] = 0xFF; out[5] = 0xFE; out[6] = 0x60; out[7] = plug;
}
static inline void FWAvcSignalSourceControlFrame(uint8_t out[8], uint8_t addr, uint8_t srcPlug, uint8_t plug)
{
    out[0] = 0x00; out[1] = 0xFF; out[2] = 0x1A; out[3] = 0x0F; out[4] = addr; out[5] = srcPlug; out[6] = 0x60; out[7] = plug;
}

// ---- function-block frames (FWAVCAudioDevice, every AV/C model; A1 Q-ME §7b, Q-AP0c §1) -------------------------------------------------
// ref AVCControlFunctionBlock @0x22a9x (decomp FWAVCAudioDevice__AVCControlFunctionBlock.c): bytes 0-5 = 00 08 B8 <type> <fb> 10 (CONTROL, audio
// subunit 0x08, FUNCTION BLOCK, type 0x80 selector / 0x81 feature / 0x82 processing, attribute 0x10 current); the caller fills the rest.
static inline void FWAvcFunctionBlockHeader(uint8_t *f, uint8_t type, uint8_t fb)
{
    f[0] = 0x00; f[1] = 0x08; f[2] = 0xB8; f[3] = type; f[4] = fb; f[5] = 0x10;
}
// ref AVCSelectorCommand @0x23074: 00 08 B8 80 <fb> 10 02 <value> 01 00 00 00 (12 bytes)
static inline void FWAvcSelectorFrame(uint8_t f[12], uint8_t fb, uint8_t value)
{
    FWAvcFunctionBlockHeader(f, 0x80, fb); f[6] = 0x02; f[7] = value; f[8] = 0x01; f[9] = 0x00; f[10] = 0x00; f[11] = 0x00;
}
// ref AVCControlFeatureFunctionBlock @0x231a8: 00 08 B8 81 <fb> 10 02 <ch> <ctl> 02 <v hi> <v lo> (12 bytes)
static inline void FWAvcFeatureFrame(uint8_t f[12], uint8_t fb, uint8_t controlSelector, uint8_t channel, uint16_t value)
{
    FWAvcFunctionBlockHeader(f, 0x81, fb); f[6] = 0x02; f[7] = channel; f[8] = controlSelector; f[9] = 0x02;
    f[10] = (uint8_t)(value >> 8); f[11] = (uint8_t)value;
}
// ref AVCMixerConnectSingle @0x23586: 00 08 B8 82 <fb> 10 04 <plug> <in> <out> 03 02 <00 connect | 80 disconnect> 00 00 00 (16 bytes)
static inline void FWAvcMixerConnectFrame(uint8_t f[16], bool connect, uint8_t fb, uint8_t plug, uint8_t in, uint8_t out)
{
    FWAvcFunctionBlockHeader(f, 0x82, fb); f[6] = 0x04; f[7] = plug; f[8] = in; f[9] = out; f[10] = 0x03; f[11] = 0x02;
    f[12] = connect ? 0x00 : 0x80; f[13] = 0x00; f[14] = 0x00; f[15] = 0x00;
}
// ref AVCVendorCommand(a, b, n, data) @0x23970 (asm 0x23981-0x23ac3): 16-byte buffer, zero-filled, 00 FF 00 | 00 0D 6C (M-Audio company id)
// | a b n | n data bytes; sent with length 16 through vt[0x8b8] (synchronous). n >= 8 -> 0xE00002C2 (returns 0 here).
enum { kFWAvcVendorLen = 16 };
static inline uint32_t FWAvcVendorFrame(uint8_t f[kFWAvcVendorLen], uint8_t a, uint8_t b, uint32_t n, const uint8_t *data)
{
    if (n >= 8) return 0;
    memset(f, 0, kFWAvcVendorLen);
    f[0] = 0x00; f[1] = 0xFF; f[2] = 0x00; f[3] = 0x00; f[4] = 0x0D; f[5] = 0x6C; f[6] = a; f[7] = b; f[8] = (uint8_t)n;
    for (uint32_t i = 0; i < n; i++) f[9 + i] = data[i];
    return kFWAvcVendorLen;
}

// ---- EXTENDED PLUG INFO (AV/C PLUG INFO 0x02, subfunction 0xC0), STATUS, read-only. NOT in the reference driver: a clone diagnostic sent by
// Rev2 models at bring-up (plan v4 AP-3.10, R92) to log the channel order before audio. Never for the FW1814 (its firmware freezes on commands
// it does not know, snd-bebob bebob_maudio.c:30-33). Layout from FFADO (wire truth only): avc_extended_plug_info.cpp ExtendedPlugInfoCmd::serialize
// (subfunction, plug address, info type), avc_extended_cmd_generic.cpp UnitPlugAddress (type 0x00 PCR, id, reserved FF), direction 0x00 input /
// 0x01 output (avc_extended_cmd_generic.h:210-211), info types 0x03 channel position / 0x07 cluster info (avc_extended_plug_info.h:238,242),
// request data: channel position = number of clusters 0 (ctor :176-179); cluster info = index, port type 0xFF, string length 0xFF (ctor :429-434).
enum { kFWAvcPlugInfoPosLen = 11, kFWAvcPlugInfoClusterLen = 13 };
static inline void FWAvcExtPlugInfoHeader(uint8_t *f, uint8_t direction, uint8_t plug, uint8_t infoType)
{
    f[0] = 0x01; f[1] = 0xFF; f[2] = 0x02; f[3] = 0xC0; f[4] = direction; f[5] = 0x00; f[6] = 0x00; f[7] = plug; f[8] = 0xFF; f[9] = infoType;
}
static inline void FWAvcExtPlugInfoChannelPositionFrame(uint8_t f[kFWAvcPlugInfoPosLen], uint8_t direction, uint8_t plug)
{
    FWAvcExtPlugInfoHeader(f, direction, plug, 0x03); f[10] = 0x00;
}
static inline void FWAvcExtPlugInfoClusterFrame(uint8_t f[kFWAvcPlugInfoClusterLen], uint8_t direction, uint8_t plug, uint8_t cluster)
{
    FWAvcExtPlugInfoHeader(f, direction, plug, 0x07); f[10] = cluster; f[11] = 0xFF; f[12] = 0xFF;
}
// Summarise a channel-position response (bytes from offset 10: clusters, then per cluster: channels, {stream position, location} pairs) as
// "n clusters: [c] pos/loc ..." into `out`; returns the number of clusters (0 when the response is malformed or short).
static inline uint32_t FWAvcFormatChannelPosition(const uint8_t *r, uint32_t len, char *out, uint32_t cap)
{
    uint32_t o = 0, i = 10;
    if (cap == 0) return 0;
    out[0] = 0;
    if (len < 11) return 0;
    const uint32_t n = r[i++];
    o += (uint32_t)snprintf(out + o, cap - o, "%u clusters:", n);
    for (uint32_t c = 0; c < n && i < len && o < cap; c++) {
        const uint32_t ch = r[i++];
        o += (uint32_t)snprintf(out + o, cap > o ? cap - o : 0, " [%u]", c + 1);
        for (uint32_t k = 0; k < ch && i + 1 < len && o < cap; k++, i += 2)
            o += (uint32_t)snprintf(out + o, cap > o ? cap - o : 0, " %u/%u", r[i], r[i + 1]);
    }
    return n;
}

// ref FWRev2AudioDevice::UpdatePeakMeters (decomp FWRev2AudioDevice__UpdatePeakMeters.c): peak = ((q & 0xff00) * 0x100 + q * 0x1000000) >> 16 in 32-bit arithmetic,
// i.e. the low 16 bits of the host-order quadlet, byte-swapped
static inline uint32_t FWRev2PeakFromQuadlet(uint32_t q)
{
    return (uint32_t)((q & 0xff00u) * 0x100u + q * 0x1000000u) >> 16;
}

#endif
