// m_audio_b_FWDCLOutputProgram::ResetPort
// Entry: 00025566
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLOutputProgram::ResetPort(unsigned int) */

void __thiscall
m_audio_b_FWDCLOutputProgram::ResetPort(m_audio_b_FWDCLOutputProgram *this,uint param_1)

{
  m_audio_b_FWDCLProgram::ResetPort((m_audio_b_FWDCLProgram *)this,true,param_1);
  return;
}
