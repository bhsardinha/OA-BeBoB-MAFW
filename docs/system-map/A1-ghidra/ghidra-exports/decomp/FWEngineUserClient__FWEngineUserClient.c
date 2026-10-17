// m_audio_b_FWEngineUserClient::m_audio_b_FWEngineUserClient
// Entry: 00005ede
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWEngineUserClient::m_audio_b_FWEngineUserClient(OSMetaClass const*) */

void __thiscall
m_audio_b_FWEngineUserClient::m_audio_b_FWEngineUserClient
          (m_audio_b_FWEngineUserClient *this,OSMetaClass *param_1)

{
  IOAudioEngineUserClient::IOAudioEngineUserClient((IOAudioEngineUserClient *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FWEngineUserClient_00031ec0;
  return;
}
