// m_audio_b_FWHSCI::Resume
// Entry: 0001d424
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::Resume() */

void __thiscall m_audio_b_FWHSCI::Resume(m_audio_b_FWHSCI *this)

{
  int iVar1;
  
  iVar1 = *(int *)(this + 0x1c);
  *(int *)(this + 0x1c) = iVar1 + -1;
  if ((iVar1 + -1 == 0) && (*(int *)(this + 0x18) != 0)) {
    (**(code **)(**(long **)(this + 0x30) + 0x150))();
                    /* WARNING: Could not recover jumptable at 0x0001d46f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(long **)(this + 0x30) + 0x1d0))(*(long **)(this + 0x30),0x32);
    return;
  }
  return;
}
