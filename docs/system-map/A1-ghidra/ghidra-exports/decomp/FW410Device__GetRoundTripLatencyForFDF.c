// com_m_audio_FW410Device::GetRoundTripLatencyForFDF
// Entry: 000145a6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::GetRoundTripLatencyForFDF(FDFValue) */

undefined8 __thiscall
com_m_audio_FW410Device::GetRoundTripLatencyForFDF(undefined8 param_1,undefined4 param_2)

{
  switch(param_2) {
  case 2:
    return 0xa0;
  case 3:
    return 0xfe;
  case 4:
    return 0xf5;
  case 5:
    return 0x1b8;
  case 6:
    return 0x1e0;
  default:
    return 0x96;
  }
}
