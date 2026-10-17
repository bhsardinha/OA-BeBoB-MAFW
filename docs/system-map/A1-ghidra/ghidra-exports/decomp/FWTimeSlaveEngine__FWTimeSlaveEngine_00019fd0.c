// m_audio_b_FWTimeSlaveEngine::m_audio_b_FWTimeSlaveEngine
// Entry: 00019fd0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWTimeSlaveEngine::m_audio_b_FWTimeSlaveEngine() */

void __thiscall
m_audio_b_FWTimeSlaveEngine::m_audio_b_FWTimeSlaveEngine(m_audio_b_FWTimeSlaveEngine *this)

{
  IOAudioEngine::IOAudioEngine((IOAudioEngine *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWTimeSlaveEngine_00042380;
  OSMetaClass::instanceConstructed();
  return;
}
