// m_audio_b_FWKIPCUserClient::m_audio_b_FWKIPCUserClient
// Entry: 0001632e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::m_audio_b_FWKIPCUserClient() */

void __thiscall
m_audio_b_FWKIPCUserClient::m_audio_b_FWKIPCUserClient(m_audio_b_FWKIPCUserClient *this)

{
  IOUserClient::IOUserClient((IOUserClient *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWKIPCUserClient_0003d320;
  OSMetaClass::instanceConstructed();
  return;
}
