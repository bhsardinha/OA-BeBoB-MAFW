// m_audio_b_FWPropertyDict::CacheModifiedPropertyDictionaryInUserClients
// Entry: 00009a2e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWPropertyDict::CacheModifiedPropertyDictionaryInUserClients(OSDictionary*) */

void __thiscall
m_audio_b_FWPropertyDict::CacheModifiedPropertyDictionaryInUserClients
          (m_audio_b_FWPropertyDict *this,OSDictionary *param_1)

{
  long *plVar1;
  OSCollection *pOVar2;
  long *plVar3;
  long *local_28;
  
  local_28 = (long *)0x0;
  if (param_1 != (OSDictionary *)0x0) {
    pOVar2 = (OSCollection *)m_audio_b_FWKIPCUserClient::UserClients();
    plVar3 = (long *)TCollectionIterator<m_audio_b_FWKIPCUserClient*>::start
                               ((TCollectionIterator<m_audio_b_FWKIPCUserClient*> *)&local_28,pOVar2
                               );
    plVar1 = local_28;
    while (plVar3 != (long *)0x0) {
      if (plVar3[0x1b] == *(long *)(this + 0x48)) {
        (**(code **)(*plVar3 + 0x960))(plVar3,param_1);
      }
      plVar1 = local_28;
      plVar3 = (long *)(**(code **)(*local_28 + 0x128))(local_28);
    }
    if (plVar1 != (long *)0x0) {
      (**(code **)(*plVar1 + 0x28))(plVar1);
    }
  }
  return;
}
