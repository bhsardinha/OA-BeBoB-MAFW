// m_audio_b_FWPropertyDict::SendAllPropertiesToUserClients
// Entry: 0000a01a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWPropertyDict::SendAllPropertiesToUserClients() */

void __thiscall
m_audio_b_FWPropertyDict::SendAllPropertiesToUserClients(m_audio_b_FWPropertyDict *this)

{
  OSDictionary *pOVar1;
  
  pOVar1 = (OSDictionary *)CopyAllProperties(this);
  if (pOVar1 != (OSDictionary *)0x0) {
    CacheModifiedPropertyDictionaryInUserClients(this,pOVar1);
    (**(code **)(*(long *)pOVar1 + 0x28))(pOVar1);
    if (*(long *)(this + 0x30) != 0) {
      IOEventSource::signalWorkAvailable();
      return;
    }
  }
  return;
}
