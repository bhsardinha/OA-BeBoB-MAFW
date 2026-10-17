// m_audio_b_FWDiscreteEngine::DeactivateSubEngine
// Entry: 00018b1e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::DeactivateSubEngine(unsigned int) */

void __thiscall
m_audio_b_FWDiscreteEngine::DeactivateSubEngine(m_audio_b_FWDiscreteEngine *this,uint param_1)

{
  int iVar1;
  uint uVar2;
  long *plVar3;
  
  iVar1 = *(int *)(this + 0x128);
  plVar3 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 0xb8),
                              (OSMetaClass *)m_audio_b_FWDiscreteDevice::metaClass);
  if (((plVar3 != (long *)0x0) &&
      (uVar2 = *(uint *)(this + 0x128), *(uint *)(this + 0x128) = ~param_1 & uVar2,
      (~param_1 & uVar2) == 0)) && (iVar1 != 0)) {
    (**(code **)(*(long *)this + 0xc20))(this);
    if (param_1 != 1) {
      (**(code **)(*(long *)this + 0xb50))(this,0);
      (**(code **)(*(long *)this + 0xba8))(this,4);
                    /* WARNING: Could not recover jumptable at 0x00018bb6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*plVar3 + 0xa18))(plVar3);
      return;
    }
  }
  return;
}
