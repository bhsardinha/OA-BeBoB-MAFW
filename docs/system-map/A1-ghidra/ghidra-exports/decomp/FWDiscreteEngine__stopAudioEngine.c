// m_audio_b_FWDiscreteEngine::stopAudioEngine
// Entry: 00018d8c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::stopAudioEngine() */

ulong __thiscall m_audio_b_FWDiscreteEngine::stopAudioEngine(m_audio_b_FWDiscreteEngine *this)

{
  int iVar1;
  ulong uVar2;
  long *plVar3;
  
  iVar1 = (**(code **)(*(long *)this + 0xa70))(this);
  if (iVar1 == 1) {
    uVar2 = (**(code **)(*(long *)this + 0xa68))(this);
    if ((int)uVar2 != 0) {
      return uVar2;
    }
  }
  else {
    if (iVar1 != 3) {
      return 0;
    }
    uVar2 = 0;
  }
  if (*(uint *)(this + 0x128) < 2) {
    plVar3 = (long *)OSMetaClassBase::safeMetaCast
                               (*(OSMetaClassBase **)(this + 0xb8),
                                (OSMetaClass *)m_audio_b_FWDiscreteDevice::metaClass);
    (**(code **)(*(long *)this + 0xb50))(this,0);
    (**(code **)(*(long *)this + 0xba8))(this,4);
    if (plVar3 != (long *)0x0) {
      (**(code **)(*plVar3 + 0xa18))();
    }
  }
  return uVar2 & 0xffffffff;
}
