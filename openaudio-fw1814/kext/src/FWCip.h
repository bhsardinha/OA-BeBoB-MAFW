// CIP / SYT state for one DCL program. Mirrors the CIP block of m_audio_b_FWDCLProgram (offsets +0x498..+0x4e0):
// SetupCIP @0x27102 (A1 Q2, Q-DCL) and CalcIsochPacketHeaders @0x251a0 (Ghidra FWDCLProgram__CalcIsochPacketHeaders.c).
#ifndef OA_FWCIP_H
#define OA_FWCIP_H

#include <IOKit/IOTypes.h>

struct FWCipState
{
    UInt32 hdr1;          // [+0x498] ((localNodeId & 0x3f) << 24) | ((audioQ + midiQ) << 16); DBC is OR-ed in per packet
    UInt32 hdr2;          // [+0x49c] 0x90000000 (CIP Q1: EOH=1, FMT=0x10)
    UInt32 rate;          // [+0x4a0] sample rate in Hz (NOT a channel count; Q-DCL correction)
    UInt32 fdf;           // [+0x4a4] FDF/SFC index 0..6
    UInt32 sytIncrement;  // [+0x4a8] 24.576 MHz ticks per packet (0x1000 / 0x116A / 0x1800)
    UInt32 sytInterval;   // [+0x4ac] data blocks per packet (8/16/32)
    UInt32 fracC;         // [+0x4b0] fractional SYT spreader numerator
    UInt32 fracD;         // [+0x4b4] fractional SYT spreader denominator
    UInt32 fracAcc;       // [+0x4b8] running accumulator for the spreader
    UInt32 cadAcc;        // [+0x4bc] running accumulator for the data/NODATA cadence
    UInt32 cadA;          // [+0x4c0] cadence numerator
    UInt32 cadB;          // [+0x4c4] cadence denominator
    UInt32 q11;           // [+0x4c8] ceil((B << 11) / ((B - A) * sytInterval))
    UInt32 q12;           // [+0x4cc] ceil((B << 12) / ((B - A) * sytInterval)): presentation offset in cycles
    UInt8  dbc;           // [+0x4d0] data block counter, +sytInterval after EVERY packet (data or NODATA)
    UInt32 sytBase;       // [+0x4d4] running presentation time (FW cycle-time format)
    UInt32 sytLast;       // [+0x4d8] last queued input SYT consumed in external-sync mode
    UInt32 lowWater;      // [+0x4dc] floor(65,536,000 / (rate*4)) - 2
    UInt32 perBank;       // [+0x4e0] P = floor(65,536,000 / (rate*4)) + 2

    // ref SetupCIP(rate, sytInterval, sytIncrement, fdf). audioQ/midiQ are the program's [+0x64]/[+0x68].
    void Setup(UInt32 rateHz, UInt32 sytInt, UInt32 sytInc, UInt32 fdfIndex, UInt32 localNodeId, UInt32 audioQ, UInt32 midiQ);
    // ref CalcIsochPacketHeaders: out[0] = bswap32(dbc | hdr1); out[1] = NODATA 0xFFFFFF90 or {0x90, fdf, SYT hi, SYT lo}.
    void CalcIsochPacketHeaders(UInt32 *out, UInt32 syt, UInt32 dbcByte, bool sync) const;
};

#endif
