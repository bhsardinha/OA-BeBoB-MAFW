// m_audio_b_FWPropertyDict::CacheModifiedPropertiesInUserClients
// Entry: 00009ab6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWPropertyDict::CacheModifiedPropertiesInUserClients() */

void __thiscall
m_audio_b_FWPropertyDict::CacheModifiedPropertiesInUserClients(m_audio_b_FWPropertyDict *this)

{
  if (*(OSDictionary **)(this + 0x58) == (OSDictionary *)0x0) {
    return;
  }
  CacheModifiedPropertyDictionaryInUserClients(this,*(OSDictionary **)(this + 0x58));
  return;
}
