// m_audio_b_FWAVCAudioDevice::setProperties
// Entry: 00022e28
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAVCAudioDevice::setProperties(OSObject*) */

undefined8 __thiscall
m_audio_b_FWAVCAudioDevice::setProperties(m_audio_b_FWAVCAudioDevice *this,OSObject *param_1)

{
  int iVar1;
  long *plVar2;
  OSMetaClassBase *pOVar3;
  undefined8 uVar4;
  
  if ((*(long *)(this + 0xf8) != 0) && (*(long *)(this + 0xe8) != 0)) {
    plVar2 = (long *)OSMetaClassBase::safeMetaCast
                               ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030008);
    if (plVar2 == (long *)0x0) {
      return 0;
    }
    pOVar3 = (OSMetaClassBase *)(**(code **)(*plVar2 + 0x208))(plVar2,"MASampleRate");
    plVar2 = (long *)OSMetaClassBase::safeMetaCast(pOVar3,*(OSMetaClass **)PTR_metaClass_00030080);
    if (plVar2 != (long *)0x0) {
      iVar1 = (**(code **)(*plVar2 + 0x148))(plVar2);
      if (*(int *)(*(long *)(this + 0xe8) + 0x8c) != iVar1) {
        (**(code **)(*(long *)this + 0xa78))(this,iVar1);
      }
    }
  }
  uVar4 = (*(code *)PTR_setProperties_00031470)(this,param_1);
  return uVar4;
}
