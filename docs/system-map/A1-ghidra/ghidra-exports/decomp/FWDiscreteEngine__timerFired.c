// m_audio_b_FWDiscreteEngine::timerFired
// Entry: 000186dc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::timerFired() */

void __thiscall m_audio_b_FWDiscreteEngine::timerFired(m_audio_b_FWDiscreteEngine *this)

{
  (**(code **)(*(long *)this + 0xaa8))(this);
                    /* WARNING: Could not recover jumptable at 0x00018700. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xab0))(this);
  return;
}
