// com_m_audio_FW1814Device::AllocateAudioEngine
// Entry: 0000d78a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2c)

/* com_m_audio_FW1814Device::AllocateAudioEngine() */

com_m_audio_FWEngine2 * com_m_audio_FW1814Device::AllocateAudioEngine(void)

{
  com_m_audio_FWEngine2 *this;
  ulong in_RSI;
  
  this = OSObject::operator_new((OSObject *)&section_00000330.offset,in_RSI);
  if (this != (com_m_audio_FWEngine2 *)0x0) {
    com_m_audio_FWEngine2::com_m_audio_FWEngine2(this);
  }
  return this;
}
