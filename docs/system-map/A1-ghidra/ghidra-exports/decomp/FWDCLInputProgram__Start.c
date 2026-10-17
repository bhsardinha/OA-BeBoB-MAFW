// m_audio_b_FWDCLInputProgram::Start
// Entry: 0002500e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLInputProgram::Start() */

void __thiscall m_audio_b_FWDCLInputProgram::Start(m_audio_b_FWDCLInputProgram *this)

{
  *(undefined4 *)(this + 0x510) = 0;
  this[0x504] = (m_audio_b_FWDCLInputProgram)0x0;
  *(undefined4 *)(*(long *)(this + 0x80) + 0x88) = 0x3f;
  m_audio_b_FWDCLProgram::Start((m_audio_b_FWDCLProgram *)this);
  return;
}
