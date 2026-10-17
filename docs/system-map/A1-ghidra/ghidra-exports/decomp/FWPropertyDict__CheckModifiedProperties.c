// m_audio_b_FWPropertyDict::CheckModifiedProperties
// Entry: 00009c9e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::CheckModifiedProperties(bool) */

void __thiscall
m_audio_b_FWPropertyDict::CheckModifiedProperties(m_audio_b_FWPropertyDict *this,bool param_1)

{
  if (*(long *)(this + 0x58) == 0) {
    return;
  }
  ApplyModifiedProperties(this,param_1);
  SendPropertyChangedMessagesToUserClients(this);
  return;
}
