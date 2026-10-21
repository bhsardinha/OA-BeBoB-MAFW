#include "FW1814Peaks.h"

static const UInt8 kPeakMap4[]   = { 0, 1, 2, 3 };
static const UInt8 kPeakMap6[]   = { 0, 1, 2, 3, 4, 5 };
static const UInt8 kPeakMap8[]   = { 0, 1, 2, 3, 6, 7, 8, 9 };
static const UInt8 kPeakMap12[]  = { 0, 1, 2, 3, 6, 7, 8, 9, 10, 11, 12, 13 };
static void Peaks4All(const SInt32 *s, SInt32 *d, UInt32 n)  { FWPeaksMax(s, d, n, 4, kPeakMap4); }
static void PeaksSPDIF6(const SInt32 *s, SInt32 *d, UInt32 n) { FWPeaksMax(s, d, n, 6, kPeakMap6); }
static void PeaksADAT8(const SInt32 *s, SInt32 *d, UInt32 n)  { FWPeaksMax(s, d, n, 8, kPeakMap8); }
static void PeaksADAT12(const SInt32 *s, SInt32 *d, UInt32 n) { FWPeaksMax(s, d, n, 12, kPeakMap12); }

FWPeakProc FW1814PeakProcForChannels(UInt32 outChannels)
{
    switch (outChannels) {
        case 6:  return PeaksSPDIF6;      // ref FW1814Device::SwitchDigitalSignals @0xe5dc
        case 8:  return PeaksADAT8;
        case 12: return PeaksADAT12;
        default: return Peaks4All;
    }
}
