// m_audio_b_FWDiscreteEngine::addTimer
// Entry: 00018706
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::addTimer() */

void __thiscall m_audio_b_FWDiscreteEngine::addTimer(m_audio_b_FWDiscreteEngine *this)

{
  code *UNRECOVERED_JUMPTABLE;
  undefined8 uVar1;
  
  if (*(long **)(this + 0xb8) == (long *)0x0) {
    return;
  }
  UNRECOVERED_JUMPTABLE = *(code **)(**(long **)(this + 0xb8) + 0x9e0);
  uVar1 = (**(code **)(*(long *)this + 0xaa0))(this);
                    /* WARNING: Could not recover jumptable at 0x0001875d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(undefined8 *)(this + 0xb8),this,PTR_timerCallback_00030020,uVar1);
  return;
}
