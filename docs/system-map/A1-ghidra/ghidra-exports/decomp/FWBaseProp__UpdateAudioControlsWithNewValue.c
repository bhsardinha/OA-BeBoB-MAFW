// m_audio_b_FWBaseProp::UpdateAudioControlsWithNewValue
// Entry: 00008d04
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::UpdateAudioControlsWithNewValue(OSObject*) */

undefined8 __thiscall
m_audio_b_FWBaseProp::UpdateAudioControlsWithNewValue(m_audio_b_FWBaseProp *this,OSObject *param_1)

{
  OSMetaClass *pOVar1;
  code *pcVar2;
  int iVar3;
  long *plVar4;
  long *plVar5;
  long *local_38;
  
  local_38 = (long *)0x0;
  plVar4 = (long *)TCollectionIterator<IOAudioControl*>::start
                             ((TCollectionIterator<IOAudioControl*> *)&local_38,
                              *(OSCollection **)(this + 0x10));
  plVar5 = local_38;
  if (plVar4 != (long *)0x0) {
    pOVar1 = *(OSMetaClass **)PTR_metaClass_00030080;
    do {
      iVar3 = (**(code **)(*plVar4 + 0x9c8))(plVar4);
      if (((iVar3 == 0x70617373) &&
          (iVar3 = (**(code **)(*plVar4 + 0x9c0))(plVar4), iVar3 == 0x6d757465)) &&
         (plVar5 = (long *)OSMetaClassBase::safeMetaCast((OSMetaClassBase *)param_1,pOVar1),
         plVar5 != (long *)0x0)) {
        pcVar2 = *(code **)(*plVar5 + 0x160);
        iVar3 = (**(code **)(*plVar5 + 0x148))(plVar5);
        (*pcVar2)(plVar5,iVar3 == 0);
      }
      (**(code **)(*plVar4 + 0x980))(plVar4,param_1);
      plVar5 = local_38;
      plVar4 = (long *)(**(code **)(*local_38 + 0x128))(local_38);
    } while (plVar4 != (long *)0x0);
  }
  if (plVar5 != (long *)0x0) {
    (**(code **)(*plVar5 + 0x28))(plVar5);
  }
  return 0;
}
