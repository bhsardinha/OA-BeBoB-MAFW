// m_audio_b_FWDiscreteEngine::ActivateSubEngine
// Entry: 00018c44
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::ActivateSubEngine(unsigned int) */

int __thiscall
m_audio_b_FWDiscreteEngine::ActivateSubEngine(m_audio_b_FWDiscreteEngine *this,uint param_1)

{
  uint uVar1;
  long *plVar2;
  int iVar3;
  
  iVar3 = *(int *)(this + 0x128);
  plVar2 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 0xb8),
                              (OSMetaClass *)m_audio_b_FWDiscreteDevice::metaClass);
  if (plVar2 == (long *)0x0) {
    iVar3 = -0x1ffffd3e;
  }
  else {
    uVar1 = *(uint *)(this + 0x128);
    *(uint *)(this + 0x128) = uVar1 | param_1;
    if ((((uVar1 | param_1) != 0) && (iVar3 == 0)) || (*(int *)(this + 0xa8) == 3)) {
      if (param_1 != 1) {
        (**(code **)(*plVar2 + 0xa10))(plVar2);
      }
      (**(code **)(PTR_vtable_000300b0 + 0xa38))(this);
      if (*(int *)(this + 0xa8) != 2) {
        iVar3 = (**(code **)(*(long *)this + 0xc18))(this);
        if (iVar3 != 0) {
          if (param_1 == 1) {
            return iVar3;
          }
          (**(code **)(*plVar2 + 0xa18))(plVar2);
          return iVar3;
        }
        if (param_1 == 1) {
          return 0;
        }
        (**(code **)(*(long *)this + 0xb50))(this,1);
        (**(code **)(*(long *)this + 0xba8))(this,3);
        return 0;
      }
    }
    iVar3 = 0;
  }
  return iVar3;
}
