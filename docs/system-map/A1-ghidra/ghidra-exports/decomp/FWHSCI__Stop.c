// m_audio_b_FWHSCI::Stop
// Entry: 0001d8fe
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::Stop() */

void __thiscall m_audio_b_FWHSCI::Stop(m_audio_b_FWHSCI *this)

{
  int iVar1;
  char cVar2;
  
  if ((*(int *)(this + 0x18) != 0) && (this[0x48] != (m_audio_b_FWHSCI)0x0)) {
    cVar2 = (**(code **)(**(long **)(this + 0x30) + 0x160))();
    if ((cVar2 != '\0') &&
       (iVar1 = *(int *)(this + 0x18), *(int *)(this + 0x18) = iVar1 + -1, iVar1 + -1 == 0)) {
                    /* WARNING: Could not recover jumptable at 0x0001d947. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(**(long **)(this + 0x30) + 0x158))();
      return;
    }
  }
  return;
}
