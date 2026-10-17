// m_audio_b_FWPropertyDict::release
// Entry: 000096bc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::release() */

void __thiscall m_audio_b_FWPropertyDict::release(m_audio_b_FWPropertyDict *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x68);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x148))(plVar1,this);
  }
                    /* WARNING: Could not recover jumptable at 0x000096f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300d8 + 0x38))(this);
  return;
}
