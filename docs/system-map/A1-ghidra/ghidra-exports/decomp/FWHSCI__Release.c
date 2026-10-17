// m_audio_b_FWHSCI::Release
// Entry: 0001d8a4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::Release() */

void __thiscall m_audio_b_FWHSCI::Release(m_audio_b_FWHSCI *this)

{
  if (this[0x48] != (m_audio_b_FWHSCI)0x0) {
    (**(code **)(**(long **)(this + 0x30) + 0x158))();
  }
  if (*(long **)(this + 0xa0) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0xa0) + 0x28))();
    *(undefined8 *)(this + 0xa0) = 0;
  }
  operator_delete(*(void **)(this + 200));
  *(undefined8 *)(this + 200) = 0;
  return;
}
