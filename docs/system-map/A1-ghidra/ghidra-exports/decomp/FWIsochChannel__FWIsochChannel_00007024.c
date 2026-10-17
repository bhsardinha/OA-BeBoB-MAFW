// m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel
// Entry: 00007024
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel() */

void __thiscall m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel(m_audio_b_FWIsochChannel *this)

{
  IOFWIsochChannel::IOFWIsochChannel((IOFWIsochChannel *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWIsochChannel_00033840;
  OSMetaClass::instanceConstructed();
  return;
}
