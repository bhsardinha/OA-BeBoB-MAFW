// m_audio_b_FWKIPCUserClient::start
// Entry: 0001625a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::start(IOService*) */

bool __thiscall
m_audio_b_FWKIPCUserClient::start(m_audio_b_FWKIPCUserClient *this,IOService *param_1)

{
  char cVar1;
  
  cVar1 = (**(code **)(PTR_vtable_000300a0 + 0x5d0))(this,param_1);
  if (cVar1 != '\0') {
    *(IOService **)(this + 0xd8) = param_1;
    UserClientAdded(this);
  }
  return cVar1 != '\0';
}
