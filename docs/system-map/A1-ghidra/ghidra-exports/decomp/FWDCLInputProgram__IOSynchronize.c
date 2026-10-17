// m_audio_b_FWDCLInputProgram::IOSynchronize
// Entry: 000257b6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLInputProgram::IOSynchronize() */

undefined8 __thiscall m_audio_b_FWDCLInputProgram::IOSynchronize(m_audio_b_FWDCLInputProgram *this)

{
  undefined8 uVar1;
  
  if ((*(long *)(this + 0x58) != 0) && (*(long *)(*(long *)(this + 0x58) + 0x28) != 0)) {
    uVar1 = IOFWLocalIsochPort::synchronizeWithIO();
    return uVar1;
  }
  return 0xe00002bc;
}
