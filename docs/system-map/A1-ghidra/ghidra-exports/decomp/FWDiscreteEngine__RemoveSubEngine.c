// m_audio_b_FWDiscreteEngine::RemoveSubEngine
// Entry: 000186bc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::RemoveSubEngine(m_audio_b_FWPairEngine*) */

void m_audio_b_FWDiscreteEngine::RemoveSubEngine(m_audio_b_FWPairEngine *param_1)

{
  if (*(long **)(param_1 + 0x118) == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000186d9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(param_1 + 0x118) + 0x1d8))();
  return;
}
