// m_audio_b_FWBaseEngine::getLocalUniqueID
// Entry: 00005adc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e11)

/* m_audio_b_FWBaseEngine::getLocalUniqueID() */

undefined8 __thiscall m_audio_b_FWBaseEngine::getLocalUniqueID(m_audio_b_FWBaseEngine *this)

{
  OSMetaClassBase *pOVar1;
  long *plVar2;
  undefined8 uVar3;
  ulong uVar4;
  char *pcVar5;
  char local_28 [32];
  
  plVar2 = *(long **)(this + 0x140);
  if (plVar2 == (long *)0x0) {
    uVar3 = 0;
  }
  else {
    pOVar1 = (OSMetaClassBase *)(**(code **)(*plVar2 + 0x2b8))(plVar2,"GUID");
    plVar2 = (long *)OSMetaClassBase::safeMetaCast(pOVar1,*(OSMetaClass **)PTR_metaClass_00030080);
    if (plVar2 == (long *)0x0) {
      pcVar5 = "<unknown>";
    }
    else {
      uVar4 = (**(code **)(*plVar2 + 0x150))(plVar2);
      if (uVar4 < 0x100000000) {
        _sprintf(local_28,"0x%lX",uVar4);
      }
      else {
        _sprintf(local_28,"0x%lX%08lX",uVar4 >> 0x20,uVar4 & 0xffffffff);
      }
      pcVar5 = local_28;
    }
    uVar3 = OSString::withCString(pcVar5);
  }
  return uVar3;
}
