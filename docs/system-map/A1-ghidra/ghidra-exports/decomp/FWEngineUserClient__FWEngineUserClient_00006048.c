// m_audio_b_FWEngineUserClient::m_audio_b_FWEngineUserClient
// Entry: 00006048
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWEngineUserClient::m_audio_b_FWEngineUserClient() */

void __thiscall
m_audio_b_FWEngineUserClient::m_audio_b_FWEngineUserClient(m_audio_b_FWEngineUserClient *this)

{
  IOAudioEngineUserClient::IOAudioEngineUserClient
            ((IOAudioEngineUserClient *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWEngineUserClient_00031ec0;
  OSMetaClass::instanceConstructed();
  return;
}
