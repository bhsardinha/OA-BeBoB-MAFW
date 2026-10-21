// com_m_audio_FWMetaNub::probe
// Entry: 0001ce62
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::probe(IOService*, int*) */

com_m_audio_FWMetaNub * __thiscall
com_m_audio_FWMetaNub::probe(com_m_audio_FWMetaNub *this,IOService *param_1,int *param_2)

{
  char cVar1;
  long *plVar2;
  long *plVar3;
  long *plVar4;
  char *pcVar5;
  long lVar6;
  
  plVar2 = (long *)(**(code **)(*(long *)param_1 + 0x2b8))(param_1,"FireWire Vendor Name");
  plVar3 = (long *)OSString::withCStringNoCopy("M-Audio");
  plVar4 = (long *)OSString::withCStringNoCopy("M-AUDIO");
  if (plVar3 == (long *)0x0) {
    return (com_m_audio_FWMetaNub *)0x0;
  }
  if (plVar4 == (long *)0x0) {
    this = (com_m_audio_FWMetaNub *)0x0;
    goto LAB_0001cf47;
  }
  if ((plVar2 == (long *)0x0) ||
     ((cVar1 = (**(code **)(*plVar2 + 0x40))(plVar2,plVar3), cVar1 == '\0' &&
      (cVar1 = (**(code **)(*plVar2 + 0x40))(plVar2,plVar4), cVar1 == '\0')))) {
LAB_0001cecc:
    this = (com_m_audio_FWMetaNub *)0x0;
  }
  else {
    plVar2 = (long *)(**(code **)(*(long *)param_1 + 0x2b8))(param_1,"FireWire Product Name");
    pcVar5 = (char *)(**(code **)(*plVar2 + 0x148))(plVar2);
    lVar6 = GetActualProductName(this,pcVar5);
    if (lVar6 == 0) goto LAB_0001cecc;
    _IOLog("M-FW: M-Audio FireWire device found%c\n",0x2e);
    *param_2 = 0x7fffff;
  }
  (**(code **)(*plVar4 + 0x28))(plVar4);
LAB_0001cf47:
  (**(code **)(*plVar3 + 0x28))(plVar3);
  return this;
}
