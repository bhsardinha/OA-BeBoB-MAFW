// m_audio_b_FWBaseProp::ChangedValueThroughAudioControl
// Entry: 00008c14
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::ChangedValueThroughAudioControl(IOAudioControl*, int, int) */

undefined8 __thiscall
m_audio_b_FWBaseProp::ChangedValueThroughAudioControl
          (m_audio_b_FWBaseProp *this,IOAudioControl *param_1,int param_2,int param_3)

{
  int iVar1;
  OSObject *pOVar2;
  undefined4 in_register_00000014;
  
  if (param_1 != (IOAudioControl *)0x0) {
    iVar1 = (**(code **)(*(long *)param_1 + 0x9c8))
                      (param_1,param_1,CONCAT44(in_register_00000014,param_2));
    if ((iVar1 == 0x70617373) &&
       (iVar1 = (**(code **)(*(long *)param_1 + 0x9c0))(param_1), iVar1 == 0x6d757465)) {
      param_3 = (int)(param_3 == 0);
    }
    pOVar2 = (OSObject *)OSNumber::withNumber((long)param_3,0x20);
    if (pOVar2 == (OSObject *)0x0) {
      return 0xe00002bd;
    }
    if (*(m_audio_b_FWPropertyDict **)(this + 0x20) != (m_audio_b_FWPropertyDict *)0x0) {
      m_audio_b_FWPropertyDict::InjectModifiedProperty
                (*(m_audio_b_FWPropertyDict **)(this + 0x20),*(OSSymbol **)(this + 0x18),pOVar2,
                 false);
      (**(code **)(*(long *)pOVar2 + 0x28))(pOVar2);
    }
  }
  return 0;
}
