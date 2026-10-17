// m_audio_b_FWDCLProgram::Start
// Entry: 000250e4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLProgram::Start() */

undefined8 __thiscall m_audio_b_FWDCLProgram::Start(m_audio_b_FWDCLProgram *this)

{
  undefined8 uVar1;
  
  if (this[0x8d] == (m_audio_b_FWDCLProgram)0x0) {
    uVar1 = (**(code **)(**(long **)(this + 0x58) + 0x158))();
    if ((int)uVar1 == 0) {
      this[0x8d] = (m_audio_b_FWDCLProgram)0x1;
    }
  }
  else {
    uVar1 = 0xe00002c2;
  }
  return uVar1;
}
