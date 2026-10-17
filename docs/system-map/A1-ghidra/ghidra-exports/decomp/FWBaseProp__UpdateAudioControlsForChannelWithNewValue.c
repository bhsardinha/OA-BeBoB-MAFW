// m_audio_b_FWBaseProp::UpdateAudioControlsForChannelWithNewValue
// Entry: 00008dea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::UpdateAudioControlsForChannelWithNewValue(unsigned int, OSObject*) */

undefined8 __thiscall
m_audio_b_FWBaseProp::UpdateAudioControlsForChannelWithNewValue
          (m_audio_b_FWBaseProp *this,uint param_1,OSObject *param_2)

{
  long *plVar1;
  uint uVar2;
  long *plVar3;
  long *local_30;
  
  local_30 = (long *)0x0;
  plVar3 = (long *)TCollectionIterator<IOAudioControl*>::start
                             ((TCollectionIterator<IOAudioControl*> *)&local_30,
                              *(OSCollection **)(this + 0x10));
  plVar1 = local_30;
  while (plVar3 != (long *)0x0) {
    uVar2 = (**(code **)(*plVar3 + 0x9b0))(plVar3);
    if (uVar2 == param_1) {
      (**(code **)(*plVar3 + 0x980))(plVar3,param_2);
    }
    plVar1 = local_30;
    plVar3 = (long *)(**(code **)(*local_30 + 0x128))(local_30);
  }
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x28))(plVar1);
  }
  return 0;
}
