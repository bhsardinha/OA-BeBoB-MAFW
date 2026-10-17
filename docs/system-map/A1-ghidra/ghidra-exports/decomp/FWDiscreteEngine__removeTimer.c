// m_audio_b_FWDiscreteEngine::removeTimer
// Entry: 00018760
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::removeTimer() */

void m_audio_b_FWDiscreteEngine::removeTimer(void)

{
  long *plVar1;
  long in_RDI;
  
  plVar1 = *(long **)(in_RDI + 0xb8);
  if (plVar1 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00018783. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x9e8))(plVar1);
  return;
}
