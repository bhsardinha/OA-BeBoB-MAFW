// m_audio_b_FWDCLOutputProgram::Stop
// Entry: 00025160
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLOutputProgram::Stop() */

void __thiscall m_audio_b_FWDCLOutputProgram::Stop(m_audio_b_FWDCLOutputProgram *this)

{
  if (this[0x8d] != (m_audio_b_FWDCLOutputProgram)0x0) {
    m_audio_b_FWDCLProgram::Stop((m_audio_b_FWDCLProgram *)this);
    **(int **)(this + 0x80) = **(int **)(this + 0x80) + -1;
  }
  return;
}
