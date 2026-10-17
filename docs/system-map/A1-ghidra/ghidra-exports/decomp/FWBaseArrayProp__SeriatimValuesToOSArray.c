// m_audio_b_FWBaseArrayProp::SeriatimValuesToOSArray
// Entry: 0000824e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::SeriatimValuesToOSArray(unsigned int, OSObject*&) */

undefined8 __thiscall
m_audio_b_FWBaseArrayProp::SeriatimValuesToOSArray
          (m_audio_b_FWBaseArrayProp *this,uint param_1,OSObject **param_2)

{
  uint uVar1;
  OSObject *pOVar2;
  undefined8 uVar3;
  long *plVar4;
  uint uVar5;
  
  pOVar2 = (OSObject *)OSArray::withCapacity(param_1);
  if (pOVar2 == (OSObject *)0x0) {
LAB_00008277:
    uVar3 = 0xe00002bd;
  }
  else {
    for (uVar5 = 0; uVar5 < param_1; uVar5 = uVar5 + 1) {
      uVar1 = (**(code **)(*(long *)this + 0x158))(this,uVar5);
      plVar4 = (long *)OSNumber::withNumber((ulong)uVar1,0x20);
      if (plVar4 == (long *)0x0) {
        (**(code **)(*(long *)pOVar2 + 0x28))(pOVar2);
        pOVar2 = (OSObject *)0x0;
        goto LAB_00008277;
      }
      (**(code **)(*(long *)pOVar2 + 0x1b8))(pOVar2,plVar4);
      (**(code **)(*plVar4 + 0x28))(plVar4);
    }
    uVar3 = 0;
  }
  *param_2 = pOVar2;
  return uVar3;
}
