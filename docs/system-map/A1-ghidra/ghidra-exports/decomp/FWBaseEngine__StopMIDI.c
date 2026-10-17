// m_audio_b_FWBaseEngine::StopMIDI
// Entry: 000040f6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_b_FWBaseEngine::StopMIDI() */

undefined8 __thiscall m_audio_b_FWBaseEngine::StopMIDI(m_audio_b_FWBaseEngine *this)

{
  if (this[0x298] != (m_audio_b_FWBaseEngine)0x0) {
    this[0x298] = (m_audio_b_FWBaseEngine)0x0;
    (**(code **)(*(long *)this + 0xc20))();
  }
  return 0;
}
