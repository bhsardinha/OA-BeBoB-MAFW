// m_audio_b_FWIsochChannel::free
// Entry: 00006dca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::free() */

void __thiscall m_audio_b_FWIsochChannel::free(m_audio_b_FWIsochChannel *this)

{
  if (*(long **)(this + 0x28) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x28) + 0x28))();
  }
                    /* WARNING: Could not recover jumptable at 0x00006df5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300c8 + 0xa0))(this);
  return;
}
