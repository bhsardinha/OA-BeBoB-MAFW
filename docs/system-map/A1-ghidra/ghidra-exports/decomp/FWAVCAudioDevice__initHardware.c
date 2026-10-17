// m_audio_b_FWAVCAudioDevice::initHardware
// Entry: 00022ed2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e5)

/* m_audio_b_FWAVCAudioDevice::initHardware(IOService*) */

char __thiscall
m_audio_b_FWAVCAudioDevice::initHardware(m_audio_b_FWAVCAudioDevice *this,IOService *param_1)

{
  char cVar1;
  char cVar2;
  char cVar3;
  long lVar4;
  
  lVar4 = OSMetaClassBase::safeMetaCast
                    ((OSMetaClassBase *)param_1,(OSMetaClass *)com_m_audio_FWMetaNub::metaClass);
  if (lVar4 != 0) {
    param_1 = (IOService *)(**(code **)(*(long *)param_1 + 0x680))(param_1);
  }
  lVar4 = OSMetaClassBase::safeMetaCast
                    ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030050);
  *(long *)(this + 0xe0) = lVar4;
  if (lVar4 == 0) {
    cVar3 = '\0';
  }
  else {
    cVar1 = (*(code *)PTR_initHardware_00031ac0)(this,param_1);
    cVar3 = cVar1;
    if (cVar1 != '\0') {
      (**(code **)(*(long *)this + 0xb58))(this);
      (**(code **)(*(long *)this + 0xb00))(this);
      cVar2 = (**(code **)(*(long *)this + 0xb78))(this);
      cVar3 = '\0';
      if (cVar2 != '\0') {
        cVar3 = cVar1;
      }
    }
  }
  return cVar3;
}
