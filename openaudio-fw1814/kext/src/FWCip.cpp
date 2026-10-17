#include "FWCip.h"

static inline UInt32 bswap32(UInt32 v) { return __builtin_bswap32(v); }

void FWCipState::Setup(UInt32 rateHz, UInt32 sytInt, UInt32 sytInc, UInt32 fdfIndex, UInt32 localNodeId,
                       UInt32 audioQ, UInt32 midiQ)
{
    UInt32 quotient = 0x3E80000u / (rateHz * 4u);
    lowWater = quotient - 2;
    perBank = quotient + 2;
    dbc = 0;
    sytInterval = sytInt;
    sytIncrement = sytInc;
    fdf = fdfIndex;
    rate = rateHz;
    hdr1 = ((localNodeId & 0x3f) << 24) | ((midiQ + audioQ) * 0x10000u);
    hdr2 = 0x90000000;

    UInt32 num, den, c, d;
    switch (fdfIndex) {
        case 0:  cadA = 1;   cadB = 2;     fracC = 1;     fracD = 1;     break;
        case 1: case 3: case 5:
                 cadA = 199; cadB = 0x280; fracC = 0x153; fracD = 0x1b9; break;
        case 2: case 4: case 6:
                 cadA = 1;   cadB = 4;     fracC = 1;     fracD = 1;     break;
        default: break;                        // ref: keeps the previous constants
    }
    num = cadA; den = cadB; c = fracC; d = fracD; (void)d;
    cadAcc = num;                              // ref [+0x4bc] = iVar4 (the A constant)
    fracAcc = c;                               // ref [+0x4b8] = uVar1 (the C constant)
    UInt32 div = (den - num) * sytInterval;
    UInt32 a = (den << 11) / div;
    q11 = ((den << 11) % div) ? a + 1 : a;
    UInt32 b = (den << 12) / div;
    q12 = ((den << 12) % div) ? b + 1 : b;
}

void FWCipState::CalcIsochPacketHeaders(UInt32 *out, UInt32 syt, UInt32 dbcByte, bool sync) const
{
    out[0] = bswap32(dbcByte | hdr1);
    if (!sync) {
        out[1] = (hdr2 >> 24) - 0x100;         // 0xFFFFFF90: FDF 0xFF, SYT 0xFFFF = NODATA
        return;
    }
    UInt32 x = syt | hdr2;
    out[1] = (((fdf << 16) | hdr2) >> 8 & 0xff00) + ((x & 0xff00) << 8) + (x << 24) + (hdr2 >> 24);
}
