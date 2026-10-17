// m_audio_b_FWDiscreteEngine::free
// Entry: 00018824
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::free() */

void __thiscall m_audio_b_FWDiscreteEngine::free(m_audio_b_FWDiscreteEngine *this)

{
  if ((*(long *)(this + 0x110) == 0) && (*(long **)(this + 0x120) != (long *)0x0)) {
    (**(code **)(**(long **)(this + 0x120) + 0x28))();
    *(undefined8 *)(this + 0x120) = 0;
    *(undefined8 *)(this + 0xb0) = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x00018872. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300b0 + 0xa0))(this);
  return;
}
