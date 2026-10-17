// m_audio_b_FWPairEngine::free
// Entry: 0001ab30
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* WARNING: Switch with 1 destination removed at 0x0001ab3c */
/* m_audio_b_FWPairEngine::free() */

void __thiscall m_audio_b_FWPairEngine::free(m_audio_b_FWPairEngine *this)

{
  *(undefined8 *)(this + 0xb0) = 0;
  if ((*(long *)(this + 0x110) == 0) && (*(long **)(this + 0x120) != (long *)0x0)) {
    (**(code **)(**(long **)(this + 0x120) + 0x28))();
  }
                    /* WARNING: Could not recover jumptable at 0x00019dcf. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300b0 + 0xa0))(this);
  return;
}
