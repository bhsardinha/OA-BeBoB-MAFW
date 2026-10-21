// com_m_audio_FWMetaNub::setProperties
// Entry: 0001cb64
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::setProperties(OSObject*) */

ulong __thiscall com_m_audio_FWMetaNub::setProperties(com_m_audio_FWMetaNub *this,OSObject *param_1)

{
  byte bVar1;
  int iVar2;
  long lVar3;
  OSString *pOVar4;
  OSObject *pOVar5;
  OSString *pOVar6;
  ulong uVar7;
  
  iVar2 = (**(code **)(**(long **)(this + 0xe8) + 0xa18))();
  lVar3 = OSMetaClassBase::safeMetaCast
                    ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030008);
  if (lVar3 == 0) {
    lVar3 = OSMetaClassBase::safeMetaCast
                      ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030090);
    if (lVar3 == 0) {
      uVar7 = 0xe00002c2;
    }
    else {
      pOVar4 = (OSString *)OSSymbol::withString((OSString *)param_1);
      bVar1 = SetOneProperty(this,pOVar4,(OSObject *)0x0);
      uVar7 = (ulong)bVar1;
      (**(code **)(*(long *)pOVar4 + 0x28))(pOVar4);
    }
  }
  else {
    if (param_1 == (OSObject *)0x0) {
      pOVar6 = (OSString *)0x0;
      pOVar4 = pOVar6;
    }
    else {
      pOVar4 = (OSString *)OSCollectionIterator::withCollection((OSCollection *)param_1);
      if (pOVar4 == (OSString *)0x0) {
        pOVar6 = (OSString *)0x0;
      }
      else {
        pOVar6 = (OSString *)(**(code **)(*(long *)pOVar4 + 0x128))(pOVar4);
      }
    }
    uVar7 = 0xe00002c2;
    while (pOVar6 != (OSString *)0x0) {
      pOVar5 = (OSObject *)(**(code **)(*(long *)param_1 + 0x1f8))(param_1,pOVar6);
      bVar1 = SetOneProperty(this,pOVar6,pOVar5);
      uVar7 = (ulong)bVar1;
      if (bVar1 != 0) break;
      pOVar6 = (OSString *)(**(code **)(*(long *)pOVar4 + 0x128))(pOVar4);
    }
    if (pOVar4 != (OSString *)0x0) {
      (**(code **)(*(long *)pOVar4 + 0x28))(pOVar4);
    }
  }
  if (iVar2 == 0) {
    (**(code **)(**(long **)(this + 0xe8) + 0xa20))();
  }
  return uVar7;
}
