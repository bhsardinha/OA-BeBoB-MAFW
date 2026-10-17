// m_audio_b_FWDCLInputProgram::Connect
// Entry: 000257a4  Size: 17 bytes
// Prototype: undefined Connect(m_audio_b_FWDCLInputProgram * this, uint param_1)


/* m_audio_b_FWDCLInputProgram::Connect(unsigned int) */

void __thiscall m_audio_b_FWDCLInputProgram::Connect(m_audio_b_FWDCLInputProgram *this,uint param_1)

{
  m_audio_b_FWDCLProgram::CreateIsochChannelAndPorts
            ((m_audio_b_FWDCLProgram *)this,false,param_1,*(uint *)(this + 0x60));
  return;
}


