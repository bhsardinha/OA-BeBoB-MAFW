// m_audio_b_FWDiscreteEngine::AddSubEngine
// Entry: 00018f0c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::AddSubEngine(m_audio_b_FWPairEngine*) */

void m_audio_b_FWDiscreteEngine::AddSubEngine(m_audio_b_FWPairEngine *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(param_1 + 0x118);
  if (plVar1 == (long *)0x0) {
    plVar1 = (long *)OSSet::withCapacity(4);
    *(long **)(param_1 + 0x118) = plVar1;
  }
  if (plVar1 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00018f54. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x1c0))(plVar1);
  return;
}
