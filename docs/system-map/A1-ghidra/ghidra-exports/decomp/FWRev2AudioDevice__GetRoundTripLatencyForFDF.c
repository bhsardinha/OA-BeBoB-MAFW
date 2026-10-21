// FWRev2AudioDevice::GetRoundTripLatencyForFDF
// Entry: 00012a54
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::GetRoundTripLatencyForFDF(FDFValue) */

undefined8 __thiscall
FWRev2AudioDevice::GetRoundTripLatencyForFDF(undefined8 param_1,undefined4 param_2)

{
  switch(param_2) {
  case 2:
    return 0xd2;
  case 3:
    return 0x1c9;
  case 4:
    return 0x181;
  case 5:
  case 6:
    return 0x18d;
  default:
    return 0xe2;
  }
}
