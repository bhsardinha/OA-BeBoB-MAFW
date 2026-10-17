// m_audio_b_FWKIPCUserClient::UserClientRemoved
// Entry: 00015bea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::UserClientRemoved(m_audio_b_FWKIPCUserClient*) */

void m_audio_b_FWKIPCUserClient::UserClientRemoved(m_audio_b_FWKIPCUserClient *param_1)

{
  int iVar1;
  
  if (sUserClients != (long *)0x0) {
    (**(code **)(*sUserClients + 0x1d8))(sUserClients,param_1);
    iVar1 = (**(code **)(*sUserClients + 0x130))();
    if (iVar1 == 0) {
      (**(code **)(*sUserClients + 0x28))();
      sUserClients = (long *)0x0;
    }
  }
  return;
}
