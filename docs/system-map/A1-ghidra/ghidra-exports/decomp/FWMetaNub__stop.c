// com_m_audio_FWMetaNub::stop
// Entry: 0001ba84  Size: 170 bytes
// Prototype: undefined stop(com_m_audio_FWMetaNub * this, IOService * param_1)


/* com_m_audio_FWMetaNub::stop(IOService*) */

void __thiscall com_m_audio_FWMetaNub::stop(com_m_audio_FWMetaNub *this,IOService *param_1)

{
  int iVar1;
  OSMetaClassBase *pOVar2;
  long lVar3;
  
  pOVar2 = (OSMetaClassBase *)(**(code **)(*(long *)this + 0x2b8))(this,"GUIDString");
  lVar3 = OSMetaClassBase::safeMetaCast(pOVar2,*(OSMetaClass **)PTR_metaClass_00030090);
  this[0xf4] = (com_m_audio_FWMetaNub)0x1;
  (**(code **)(PTR_vtable_000300e0 + 0x5d8))(this,param_1);
  if ((lVar3 != 0) && (sNubDictionary != (long *)0x0)) {
    (**(code **)(*sNubDictionary + 0x1e0))(sNubDictionary,lVar3);
    iVar1 = (**(code **)(*sNubDictionary + 0x130))();
    if (iVar1 == 0) {
      (**(code **)(*sNubDictionary + 0x28))();
      sNubDictionary = (long *)0x0;
    }
  }
  return;
}


