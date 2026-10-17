// com_m_audio_FW1814Device::GetRoundTripLatencyForFDF
// Entry: 0000cf30
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2c)

/* com_m_audio_FW1814Device::GetRoundTripLatencyForFDF(FDFValue) */

undefined8 __thiscall
com_m_audio_FW1814Device::GetRoundTripLatencyForFDF(undefined8 param_1,undefined4 param_2)

{
  switch(param_2) {
  case 2:
    return 0xe1;
  case 3:
    return 0x166;
  case 4:
    return 0x17a;
  case 5:
    return 0x282;
  case 6:
    return 0x2ba;
  default:
    return 0xd2;
  }
}
