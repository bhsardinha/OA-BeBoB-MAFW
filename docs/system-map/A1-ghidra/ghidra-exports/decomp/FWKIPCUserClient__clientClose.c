// m_audio_b_FWKIPCUserClient::clientClose
// Entry: 00015c38
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::clientClose() */

undefined8 __thiscall m_audio_b_FWKIPCUserClient::clientClose(m_audio_b_FWKIPCUserClient *this)

{
  *(undefined8 *)(this + 0xe8) = 0;
  return 0;
}
