// m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients
// Entry: 00009d88
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients(OSSymbol const*, OSObject*) */

void __thiscall
m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients
          (m_audio_b_FWPropertyDict *this,OSSymbol *param_1,OSObject *param_2)

{
  OSDictionary *pOVar1;
  OSObject *local_28;
  OSSymbol *local_20;
  
  local_28 = param_2;
  local_20 = param_1;
  pOVar1 = (OSDictionary *)OSDictionary::withObjects(&local_28,&local_20,1,1);
  if (pOVar1 != (OSDictionary *)0x0) {
    CacheModifiedPropertyDictionaryInUserClients(this,pOVar1);
    (**(code **)(*(long *)pOVar1 + 0x28))(pOVar1);
    if (*(long *)(this + 0x30) != 0) {
      IOEventSource::signalWorkAvailable();
    }
  }
  return;
}
