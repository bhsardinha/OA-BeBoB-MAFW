// FW1814Peaks (module models/FW1814).
// ref _Sample1814Peaks{4All,SPDIF6,ADAT8,ADAT12} (decomp _Sample1814Peaks*.c): output peak meters of the host stream, installed on the engine
// ([+0x308]) by FW1814Device::SwitchDigitalSignals by output-channel count (6, 8, 12, else 4All). Every 8th frame is sampled; the sample
// is truncated to its top 16 bits (bits 8..23 of the scaled int), made absolute with the 16-bit trick (so -32768 stays negative and
// never registers) and max-merged into dst[map[c]]. `dst` is cleared by the caller before each clip.
#ifndef OA_FW1814PEAKS_H
#define OA_FW1814PEAKS_H

#include "FWSamples.h"

FWPeakProc FW1814PeakProcForChannels(UInt32 outChannels);

#endif
