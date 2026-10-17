// m_audio_b_FWHSCI::Pause
// Entry: 0001d3fc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::Pause() */

void __thiscall m_audio_b_FWHSCI::Pause(m_audio_b_FWHSCI *this)

{
  int iVar1;
  
  iVar1 = *(int *)(this + 0x1c);
  *(int *)(this + 0x1c) = iVar1 + 1;
  if ((iVar1 == 0) && (*(int *)(this + 0x18) != 0)) {
                    /* WARNING: Could not recover jumptable at 0x0001d41d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(long **)(this + 0x30) + 0x158))();
    return;
  }
  return;
}
