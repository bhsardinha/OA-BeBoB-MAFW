// m_audio_b_FWBaseArrayProp::OSArrayToSeriatimValues
// Entry: 000080cc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::OSArrayToSeriatimValues(OSObject*, unsigned int) */

undefined8 __thiscall
m_audio_b_FWBaseArrayProp::OSArrayToSeriatimValues
          (m_audio_b_FWBaseArrayProp *this,OSObject *param_1,uint param_2)

{
  OSMetaClass *pOVar1;
  code *pcVar2;
  uint uVar3;
  undefined4 uVar4;
  long *plVar5;
  OSMetaClassBase *pOVar6;
  long lVar7;
  
  plVar5 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030078);
  uVar3 = (**(code **)(*plVar5 + 0x130))(plVar5);
  if (uVar3 < param_2) {
    param_2 = uVar3;
  }
  pOVar1 = *(OSMetaClass **)PTR_metaClass_00030080;
  uVar3 = 0;
  while( true ) {
    if (param_2 <= uVar3) {
      return 0;
    }
    pOVar6 = (OSMetaClassBase *)(**(code **)(*plVar5 + 0x1e8))(plVar5,uVar3);
    if ((pOVar6 == (OSMetaClassBase *)0x0) ||
       (lVar7 = OSMetaClassBase::safeMetaCast(pOVar6,pOVar1), lVar7 == 0)) break;
    pcVar2 = *(code **)(*(long *)this + 0x160);
    uVar4 = (**(code **)(*(long *)pOVar6 + 0x148))(pOVar6);
    (*pcVar2)(this,uVar3,uVar4);
    uVar3 = uVar3 + 1;
  }
  return 0xe00002c2;
}
