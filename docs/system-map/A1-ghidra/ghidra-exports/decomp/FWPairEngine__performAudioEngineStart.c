// m_audio_b_FWPairEngine::performAudioEngineStart
// Entry: 0001ae44
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::performAudioEngineStart() */

void __thiscall m_audio_b_FWPairEngine::performAudioEngineStart(m_audio_b_FWPairEngine *this)

{
  m_audio_b_FWDiscreteEngine::ActivateSubEngine
            (*(m_audio_b_FWDiscreteEngine **)(this + 0x118),(uint)(1L << ((byte)this[300] & 0x3f)));
  return;
}
