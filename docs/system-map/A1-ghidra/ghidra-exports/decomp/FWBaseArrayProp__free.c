// m_audio_b_FWBaseArrayProp::free
// Entry: 000084c8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* WARNING: Switch with 1 destination removed at 0x000084f2 */
/* m_audio_b_FWBaseArrayProp::free() */

void __thiscall m_audio_b_FWBaseArrayProp::free(m_audio_b_FWBaseArrayProp *this)

{
  if (*(void **)(this + 0x50) != (void *)0x0) {
    operator_delete__(*(void **)(this + 0x50));
  }
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
