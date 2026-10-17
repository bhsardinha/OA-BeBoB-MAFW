// m_audio_FWb::TPTMFFunctor0<m_audio_b_FWHSCI*,void(m_audio_b_FWHSCI::*)(),void>::Call
// Entry: 0001d582
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_FWb::TPTMFFunctor0<m_audio_b_FWHSCI*, void (m_audio_b_FWHSCI::*)(), void>::Call() */

void m_audio_FWb::TPTMFFunctor0<m_audio_b_FWHSCI*,void(m_audio_b_FWHSCI::*)(),void>::Call(void)

{
  code *UNRECOVERED_JUMPTABLE;
  long in_RDI;
  
  UNRECOVERED_JUMPTABLE = *(code **)(in_RDI + 0x10);
  if (((ulong)UNRECOVERED_JUMPTABLE & 1) != 0) {
    UNRECOVERED_JUMPTABLE =
         *(code **)(UNRECOVERED_JUMPTABLE +
                   *(long *)(*(long *)(in_RDI + 8) + *(long *)(in_RDI + 0x18)) + -1);
  }
                    /* WARNING: Could not recover jumptable at 0x0001d5a7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(long *)(in_RDI + 8) + *(long *)(in_RDI + 0x18));
  return;
}
