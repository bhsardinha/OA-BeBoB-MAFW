// com_m_audio_FW1814Device::SourceMaskToHeadphoneSelector
// Entry: 0000cf8c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SourceMaskToHeadphoneSelector(unsigned int) */

undefined8 __thiscall
com_m_audio_FW1814Device::SourceMaskToHeadphoneSelector(com_m_audio_FW1814Device *this,uint param_1)

{
  if ((param_1 != 8) && (param_1 != 4)) {
    if (param_1 == 2) {
      return 1;
    }
    return 0;
  }
  return 3;
}
