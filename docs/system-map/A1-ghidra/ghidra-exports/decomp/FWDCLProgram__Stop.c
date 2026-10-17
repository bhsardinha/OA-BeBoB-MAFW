// m_audio_b_FWDCLProgram::Stop
// Entry: 00025120
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLProgram::Stop() */

void __thiscall m_audio_b_FWDCLProgram::Stop(m_audio_b_FWDCLProgram *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x58);
  if ((plVar1 != (long *)0x0) && (this[0x8d] != (m_audio_b_FWDCLProgram)0x0)) {
    (**(code **)(*plVar1 + 0x160))(plVar1);
    this[0x8d] = (m_audio_b_FWDCLProgram)0x0;
  }
  return;
}
