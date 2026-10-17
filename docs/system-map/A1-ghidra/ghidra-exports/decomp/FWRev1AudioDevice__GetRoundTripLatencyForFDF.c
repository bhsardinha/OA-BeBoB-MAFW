// FWRev1AudioDevice::GetRoundTripLatencyForFDF
// Entry: 000136da
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* FWRev1AudioDevice::GetRoundTripLatencyForFDF(FDFValue) */

undefined8 __thiscall FWRev1AudioDevice::GetRoundTripLatencyForFDF(undefined8 param_1,int param_2)

{
  if (param_2 - 4U < 3) {
    return 0x124;
  }
  if (param_2 != 3) {
    if (param_2 - 1U < 2) {
      return 0xba;
    }
    return 0;
  }
  return 0x106;
}
