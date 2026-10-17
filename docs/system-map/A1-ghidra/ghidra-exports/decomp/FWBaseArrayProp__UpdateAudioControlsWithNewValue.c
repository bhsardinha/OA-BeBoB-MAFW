// m_audio_b_FWBaseArrayProp::UpdateAudioControlsWithNewValue
// Entry: 000081c0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::UpdateAudioControlsWithNewValue(OSObject*) */

undefined8 __thiscall
m_audio_b_FWBaseArrayProp::UpdateAudioControlsWithNewValue
          (m_audio_b_FWBaseArrayProp *this,OSObject *param_1)

{
  code *pcVar1;
  int iVar2;
  uint uVar3;
  long *plVar4;
  undefined8 uVar5;
  uint uVar6;
  
  plVar4 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030078);
  if (plVar4 == (long *)0x0) {
    uVar5 = 0xe00002c2;
  }
  else {
    iVar2 = (**(code **)(*plVar4 + 0x130))(plVar4);
    if (iVar2 != 0) {
      uVar6 = 0;
      do {
        pcVar1 = *(code **)(*(long *)this + 0x130);
        uVar5 = (**(code **)(*plVar4 + 0x1e8))(plVar4,uVar6);
        (*pcVar1)(this,*(int *)(this + 0x58) + uVar6,uVar5);
        uVar3 = (**(code **)(*plVar4 + 0x130))(plVar4);
        uVar6 = uVar6 + 1;
      } while (uVar6 < uVar3);
    }
    uVar5 = 0;
  }
  return uVar5;
}
