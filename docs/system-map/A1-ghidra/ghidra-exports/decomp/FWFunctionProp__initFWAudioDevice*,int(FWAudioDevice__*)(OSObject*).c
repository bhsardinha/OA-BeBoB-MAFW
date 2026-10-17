// m_audio_b_FWFunctionProp::init<m_audio_b_FWAudioDevice*,int(m_audio_b_FWAudioDevice::*)(OSObject*)>
// Entry: 0000247e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* int m_audio_b_FWFunctionProp::init<m_audio_b_FWAudioDevice*, int
   (m_audio_b_FWAudioDevice::*)(OSObject*)>(m_audio_b_FWPropertyDict*, char const*,
   m_audio_b_FWAudioDevice*, int (m_audio_b_FWAudioDevice::*)(OSObject*)) */

int __thiscall
m_audio_b_FWFunctionProp::init<m_audio_b_FWAudioDevice*,int(m_audio_b_FWAudioDevice::*)(OSObject*)>
          (m_audio_b_FWFunctionProp *this,m_audio_b_FWPropertyDict *param_1,char *param_2,
          m_audio_b_FWAudioDevice *param_3,_func_int_OSObject_ptr *param_4)

{
  int iVar1;
  undefined8 *puVar2;
  
  if (*(void **)(this + 0x38) != (void *)0x0) {
    operator_delete(*(void **)(this + 0x38));
  }
  if ((param_3 == (m_audio_b_FWAudioDevice *)0x0) || (param_4 == (_func_int_OSObject_ptr *)0x0)) {
    *(undefined8 *)(this + 0x38) = 0;
  }
  else {
    puVar2 = operator_new(0x20);
    if (puVar2 != (undefined8 *)0x0) {
      *puVar2 = &PTR_Call_00031050;
      puVar2[1] = param_3;
      puVar2[2] = param_4;
      puVar2[3] = 0;
      *(undefined8 **)(this + 0x38) = puVar2;
                    /* WARNING: Could not recover jumptable at 0x00002527. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      iVar1 = (**(code **)(*(long *)this + 0x148))(this,param_1,param_2);
      return iVar1;
    }
    *(undefined8 *)(this + 0x38) = 0;
  }
  return -0x1ffffd44;
}
