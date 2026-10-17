// com_m_audio_FW1814Device::ResumeReceptionOfControlPackets
// Entry: 0000dba6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ResumeReceptionOfControlPackets() */

void __thiscall
com_m_audio_FW1814Device::ResumeReceptionOfControlPackets(com_m_audio_FW1814Device *this)

{
  if (*(m_audio_b_FWHSCI **)(this + 0x1270) == (m_audio_b_FWHSCI *)0x0) {
    return;
  }
  m_audio_b_FWHSCI::Resume(*(m_audio_b_FWHSCI **)(this + 0x1270));
  return;
}
