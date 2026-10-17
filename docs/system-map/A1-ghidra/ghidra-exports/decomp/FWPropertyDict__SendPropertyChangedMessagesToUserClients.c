// m_audio_b_FWPropertyDict::SendPropertyChangedMessagesToUserClients
// Entry: 00009acc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWPropertyDict::SendPropertyChangedMessagesToUserClients() */

void __thiscall
m_audio_b_FWPropertyDict::SendPropertyChangedMessagesToUserClients(m_audio_b_FWPropertyDict *this)

{
  long *plVar1;
  OSCollection *pOVar2;
  m_audio_b_FWKIPCUserClient *this_00;
  long *local_20;
  
  local_20 = (long *)0x0;
  pOVar2 = (OSCollection *)m_audio_b_FWKIPCUserClient::UserClients();
  this_00 = (m_audio_b_FWKIPCUserClient *)
            TCollectionIterator<m_audio_b_FWKIPCUserClient*>::start
                      ((TCollectionIterator<m_audio_b_FWKIPCUserClient*> *)&local_20,pOVar2);
  plVar1 = local_20;
  while (this_00 != (m_audio_b_FWKIPCUserClient *)0x0) {
    if (*(long *)(this_00 + 0xd8) == *(long *)(this + 0x48)) {
      m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage(this_00);
    }
    plVar1 = local_20;
    this_00 = (m_audio_b_FWKIPCUserClient *)(**(code **)(*local_20 + 0x128))(local_20);
  }
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x28))(plVar1);
  }
  return;
}
