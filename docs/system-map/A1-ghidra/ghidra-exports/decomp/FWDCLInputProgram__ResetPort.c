// m_audio_b_FWDCLInputProgram::ResetPort
// Entry: 00025578
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLInputProgram::ResetPort(unsigned int) */

void __thiscall
m_audio_b_FWDCLInputProgram::ResetPort(m_audio_b_FWDCLInputProgram *this,uint param_1)

{
  m_audio_b_FWDCLProgram::ResetPort((m_audio_b_FWDCLProgram *)this,false,param_1);
  return;
}
