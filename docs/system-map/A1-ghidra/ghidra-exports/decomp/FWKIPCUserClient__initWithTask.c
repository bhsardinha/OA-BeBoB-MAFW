// m_audio_b_FWKIPCUserClient::initWithTask
// Entry: 0001618e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::initWithTask(task*, void*, unsigned int) */

bool __thiscall
m_audio_b_FWKIPCUserClient::initWithTask
          (m_audio_b_FWKIPCUserClient *this,task *param_1,void *param_2,uint param_3)

{
  char cVar1;
  
  _bzero(this + 0xe0,0x2c);
  *(undefined4 *)(this + 0x118) = 0;
  *(undefined8 *)(this + 0x110) = 0;
  *(undefined8 *)(this + 0x120) = 0;
  *(task **)(this + 0x138) = param_1;
  cVar1 = (**(code **)(PTR_vtable_000300a0 + 0x8f8))(this,param_1,param_2,param_3);
  return cVar1 != '\0';
}
