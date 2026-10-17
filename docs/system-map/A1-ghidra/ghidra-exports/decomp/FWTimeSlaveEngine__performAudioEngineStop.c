// m_audio_b_FWTimeSlaveEngine::performAudioEngineStop
// Entry: 00019dee
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWTimeSlaveEngine::performAudioEngineStop() */

undefined8 __thiscall
m_audio_b_FWTimeSlaveEngine::performAudioEngineStop(m_audio_b_FWTimeSlaveEngine *this)

{
  (**(code **)(**(long **)(this + 0x118) + 0xa68))();
  return 0;
}
