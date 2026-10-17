// m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
// Entry: 00009f44
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients(char const*, OSObject*) */

void __thiscall
m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
          (m_audio_b_FWPropertyDict *this,char *param_1,OSObject *param_2)

{
  ModifyProperty(this,param_1,param_2);
  if (*(long *)(this + 0x30) == 0) {
    return;
  }
  IOEventSource::signalWorkAvailable();
  return;
}
