// m_audio_b_FWBaseProp::free
// Entry: 00008b58
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::free() */

void __thiscall m_audio_b_FWBaseProp::free(m_audio_b_FWBaseProp *this)

{
  if (*(long **)(this + 0x18) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x18) + 0x28))();
  }
  if (*(long **)(this + 0x10) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x10) + 0x28))();
  }
                    /* WARNING: Could not recover jumptable at 0x00008b92. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300d8 + 0xa0))(this);
  return;
}
