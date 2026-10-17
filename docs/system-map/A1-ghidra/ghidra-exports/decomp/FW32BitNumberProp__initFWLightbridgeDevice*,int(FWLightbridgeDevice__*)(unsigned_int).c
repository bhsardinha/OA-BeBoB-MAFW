// m_audio_b_FW32BitNumberProp::init<com_m_audio_FWLightbridgeDevice*,int(com_m_audio_FWLightbridgeDevice::*)(unsigned_int)>
// Entry: 0001f832
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* int m_audio_b_FW32BitNumberProp::init<com_m_audio_FWLightbridgeDevice*, int
   (com_m_audio_FWLightbridgeDevice::*)(unsigned int)>(m_audio_b_FWPropertyDict*, char const*,
   com_m_audio_FWLightbridgeDevice*, int (com_m_audio_FWLightbridgeDevice::*)(unsigned int)) */

int __thiscall
m_audio_b_FW32BitNumberProp::
init<com_m_audio_FWLightbridgeDevice*,int(com_m_audio_FWLightbridgeDevice::*)(unsigned_int)>
          (m_audio_b_FW32BitNumberProp *this,m_audio_b_FWPropertyDict *param_1,char *param_2,
          com_m_audio_FWLightbridgeDevice *param_3,_func_int_uint *param_4)

{
  int iVar1;
  undefined8 *puVar2;
  
  if (param_3 != (com_m_audio_FWLightbridgeDevice *)0x0) {
    if (*(void **)(this + 0x38) != (void *)0x0) {
      operator_delete(*(void **)(this + 0x38));
    }
    if (param_4 == (_func_int_uint *)0x0) {
      *(undefined8 *)(this + 0x38) = 0;
    }
    else {
      puVar2 = operator_new(0x20);
      if (puVar2 != (undefined8 *)0x0) {
        *puVar2 = &PTR_Call_00048010;
        puVar2[1] = param_3;
        puVar2[2] = param_4;
        puVar2[3] = 0;
        *(undefined8 **)(this + 0x38) = puVar2;
        iVar1 = (*(code *)PTR_init_00037a68)(this,param_1,param_2);
        return iVar1;
      }
      *(undefined8 *)(this + 0x38) = 0;
    }
  }
  return -0x1ffffd44;
}
