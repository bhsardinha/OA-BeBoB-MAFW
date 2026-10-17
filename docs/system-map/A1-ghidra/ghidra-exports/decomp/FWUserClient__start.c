// com_m_audio_FWUserClient::start
// Entry: 000076de
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::start(IOService*) */

undefined1 __thiscall
com_m_audio_FWUserClient::start(com_m_audio_FWUserClient *this,IOService *param_1)

{
  char cVar1;
  undefined1 uVar2;
  long lVar3;
  
  cVar1 = (*(code *)PTR_start_0003d8e0)(this,param_1);
  if (cVar1 == '\0') {
    uVar2 = 0;
  }
  else {
    lVar3 = OSMetaClassBase::safeMetaCast
                      ((OSMetaClassBase *)param_1,(OSMetaClass *)m_audio_b_FWAudioDevice::metaClass)
    ;
    *(long *)(this + 0x290) = lVar3;
    if (lVar3 == 0) {
      *(undefined8 *)(this + 0x290) = 0;
      uVar2 = 0;
    }
    else {
      uVar2 = 1;
    }
  }
  return uVar2;
}
