// com_m_audio_FWUserClient::initWithTask
// Entry: 0000773e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::initWithTask(task*, void*, unsigned int) */

bool __thiscall
com_m_audio_FWUserClient::initWithTask
          (com_m_audio_FWUserClient *this,task *param_1,void *param_2,uint param_3)

{
  char cVar1;
  
  _bzero(this + 0x148,0x140);
  *(undefined8 *)(this + 0x290) = 0;
  *(undefined8 *)(this + 0x288) = 0;
  cVar1 = (*(code *)PTR_initWithTask_0003dc08)(this,param_1,param_2,param_3);
  return cVar1 != '\0';
}
