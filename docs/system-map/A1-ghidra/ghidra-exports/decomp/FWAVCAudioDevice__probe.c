// m_audio_b_FWAVCAudioDevice::probe
// Entry: 00022f78
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_b_FWAVCAudioDevice::probe(IOService*, int*) */

m_audio_b_FWAVCAudioDevice * __thiscall
m_audio_b_FWAVCAudioDevice::probe(m_audio_b_FWAVCAudioDevice *this,IOService *param_1,int *param_2)

{
  code *pcVar1;
  char cVar2;
  long *plVar3;
  long lVar4;
  undefined8 uVar5;
  long lVar6;
  
  plVar3 = (long *)(**(code **)(*(long *)param_1 + 0x2b8))(param_1,"FireWire Product Name");
  lVar4 = OSMetaClassBase::safeMetaCast
                    ((OSMetaClassBase *)param_1,(OSMetaClass *)com_m_audio_FWMetaNub::metaClass);
  *param_2 = 0;
  if ((lVar4 != 0) && (plVar3 != (long *)0x0)) {
    pcVar1 = *(code **)(*plVar3 + 0x40);
    uVar5 = (**(code **)(*(long *)this + 0x2b8))(this,"FireWire Product Name");
    cVar2 = (*pcVar1)(plVar3,uVar5);
    if (cVar2 != '\0') {
      plVar3 = (long *)(**(code **)(*(long *)param_1 + 0x2b8))(param_1,"Model_ID");
      lVar6 = (**(code **)(*(long *)this + 0x2b8))(this,"Model_ID");
      if ((((plVar3 == (long *)0x0) || (lVar6 == 0)) ||
          ((lVar6 != 0 && (cVar2 = (**(code **)(*plVar3 + 0x40))(plVar3,lVar6), cVar2 != '\0')))) &&
         ((*(char *)(lVar4 + 0xf0) == '\0' && (*(char *)(lVar4 + 0xf3) != '\0')))) {
        *param_2 = 0x7fffff;
        return this;
      }
    }
  }
  return (m_audio_b_FWAVCAudioDevice *)0x0;
}
