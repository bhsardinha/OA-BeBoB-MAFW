// com_m_audio_FWMetaNub::ReplaceExistingMetaNub
// Entry: 0001cd5a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::ReplaceExistingMetaNub() */

void __thiscall com_m_audio_FWMetaNub::ReplaceExistingMetaNub(com_m_audio_FWMetaNub *this)

{
  OSMetaClassBase *pOVar1;
  long lVar2;
  long *plVar3;
  
  pOVar1 = (OSMetaClassBase *)(**(code **)(*(long *)this + 0x2b8))(this,"GUIDString");
  lVar2 = OSMetaClassBase::safeMetaCast(pOVar1,*(OSMetaClass **)PTR_metaClass_00030090);
  if (lVar2 == 0) {
    return;
  }
  if ((sNubDictionary == (long *)0x0) &&
     (sNubDictionary = (long *)OSDictionary::withCapacity(4), sNubDictionary == (long *)0x0)) {
    return;
  }
  pOVar1 = (OSMetaClassBase *)(**(code **)(*sNubDictionary + 0x200))(sNubDictionary,lVar2);
  plVar3 = (long *)OSMetaClassBase::safeMetaCast(pOVar1,(OSMetaClass *)&gMetaClass);
  if (plVar3 != (long *)0x0) {
    (**(code **)(*plVar3 + 0x600))(plVar3,0);
  }
                    /* WARNING: Could not recover jumptable at 0x0001cdff. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*sNubDictionary + 0x1c8))(sNubDictionary,lVar2,this);
  return;
}
