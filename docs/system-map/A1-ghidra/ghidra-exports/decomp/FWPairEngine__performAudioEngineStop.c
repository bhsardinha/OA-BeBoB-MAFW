// m_audio_b_FWPairEngine::performAudioEngineStop
// Entry: 0001ae22
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::performAudioEngineStop() */

undefined8 __thiscall m_audio_b_FWPairEngine::performAudioEngineStop(m_audio_b_FWPairEngine *this)

{
  m_audio_b_FWDiscreteEngine::DeactivateSubEngine
            (*(m_audio_b_FWDiscreteEngine **)(this + 0x118),(uint)(1L << ((byte)this[300] & 0x3f)));
  return 0;
}
