// m_audio_b_FWAudioDevice::GetSafetyOffsetForFDF
// Entry: 00001d58
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWAudioDevice::GetSafetyOffsetForFDF(FDFValue) */

undefined8 __thiscall
m_audio_b_FWAudioDevice::GetSafetyOffsetForFDF(undefined8 param_1,undefined4 param_2)

{
  switch(param_2) {
  case 2:
    return 0x30;
  case 3:
    return 0x58;
  case 4:
    return 0x60;
  case 5:
    return 0xb0;
  case 6:
    return 0xc0;
  default:
    return 0x2c;
  }
}
