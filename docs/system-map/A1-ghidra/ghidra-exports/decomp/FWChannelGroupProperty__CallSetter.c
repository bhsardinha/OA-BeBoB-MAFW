// m_audio_b_FWChannelGroupProperty::CallSetter
// Entry: 0000940e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWChannelGroupProperty::CallSetter(OSSymbol const*, OSObject*) */

int __thiscall
m_audio_b_FWChannelGroupProperty::CallSetter
          (m_audio_b_FWChannelGroupProperty *this,OSSymbol *param_1,OSObject *param_2)

{
  long *plVar1;
  int iVar2;
  void *pvVar3;
  void *pvVar4;
  
  pvVar3 = (void *)m_audio_FWb::BaseChannelGroup::AdjustedLevels
                             (*(BaseChannelGroup **)(this + 0x60));
  if (pvVar3 == (void *)0x0) {
    iVar2 = 0;
  }
  else {
    if (*(BaseChannelGroup **)(this + 0x70) == (BaseChannelGroup *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      pvVar4 = (void *)m_audio_FWb::BaseChannelGroup::AdjustedLevels
                                 (*(BaseChannelGroup **)(this + 0x70));
    }
    iVar2 = m_audio_b_FWBaseArrayProp::OSArrayToSeriatimValues
                      ((m_audio_b_FWBaseArrayProp *)this,param_2,*(uint *)(this + 0x2c));
    if (iVar2 == 0) {
      (**(code **)(**(long **)(this + 0x60) + 0x28))(*(long **)(this + 0x60),pvVar3);
      operator_delete__(pvVar3);
      plVar1 = *(long **)(this + 0x70);
      if (plVar1 != (long *)0x0) {
        (**(code **)(*plVar1 + 0x28))(plVar1,pvVar4);
        if (pvVar4 != (void *)0x0) {
          operator_delete__(pvVar4);
        }
      }
    }
  }
  return iVar2;
}
