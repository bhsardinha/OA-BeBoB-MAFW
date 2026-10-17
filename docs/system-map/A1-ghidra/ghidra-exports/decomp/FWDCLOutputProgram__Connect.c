// m_audio_b_FWDCLOutputProgram::Connect
// Entry: 00025790  Size: 20 bytes
// Prototype: undefined Connect(m_audio_b_FWDCLOutputProgram * this, uint param_1)


/* m_audio_b_FWDCLOutputProgram::Connect(unsigned int) */

void __thiscall
m_audio_b_FWDCLOutputProgram::Connect(m_audio_b_FWDCLOutputProgram *this,uint param_1)

{
  m_audio_b_FWDCLProgram::CreateIsochChannelAndPorts
            ((m_audio_b_FWDCLProgram *)this,true,param_1,*(uint *)(this + 0x60));
  return;
}


