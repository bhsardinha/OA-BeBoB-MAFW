// m_audio_b_FWDCLProgram::Disconnect
// Entry: 000250a2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLProgram::Disconnect() */

void __thiscall m_audio_b_FWDCLProgram::Disconnect(m_audio_b_FWDCLProgram *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x58);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x150))(plVar1);
    (**(code **)(**(long **)(this + 0x58) + 0x28))();
    *(undefined8 *)(this + 0x58) = 0;
    this[0x8c] = (m_audio_b_FWDCLProgram)0x0;
  }
  return;
}
