// FWRev2AudioDevice::AllocateAudioEngine
// Entry: 0001a27c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::AllocateAudioEngine() */

com_m_audio_FWEngine2 * FWRev2AudioDevice::AllocateAudioEngine(void)

{
  com_m_audio_FWEngine2 *this;
  ulong in_RSI;
  
  this = OSObject::operator_new((OSObject *)&section_00000330.offset,in_RSI);
  if (this != (com_m_audio_FWEngine2 *)0x0) {
    com_m_audio_FWEngine2::com_m_audio_FWEngine2(this);
  }
  return this;
}
