// m_audio_b_FWBaseEngine::getGlobalUniqueID
// Entry: 00005b84
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e11)

/* m_audio_b_FWBaseEngine::getGlobalUniqueID() */

undefined8 __thiscall m_audio_b_FWBaseEngine::getGlobalUniqueID(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  char *pcVar2;
  long lVar3;
  char *pcVar4;
  long *plVar5;
  size_t sVar6;
  char *pcVar7;
  undefined8 uVar8;
  uint uVar9;
  ulong uVar10;
  
  if (this[0x34a] == (m_audio_b_FWBaseEngine)0x0) {
    lVar3 = (**(code **)(*(long *)this + 0x38))(this);
    if (lVar3 == 0) {
      pcVar2 = (char *)0x0;
    }
    else {
      pcVar2 = (char *)OSMetaClass::getClassName();
    }
  }
  else {
    pcVar2 = "com_m_audio_FWEngine1";
  }
  pcVar4 = (char *)(**(code **)(*(long *)this + 0x3b8))(this,0);
  plVar5 = (long *)(**(code **)(*(long *)this + 0xb38))(this);
  if (pcVar2 == (char *)0x0) {
    uVar9 = 3;
  }
  else {
    sVar6 = _strlen(pcVar2);
    uVar9 = (int)sVar6 + 3;
  }
  if (pcVar4 != (char *)0x0) {
    sVar6 = _strlen(pcVar4);
    uVar9 = uVar9 + (int)sVar6;
  }
  if (plVar5 != (long *)0x0) {
    iVar1 = (**(code **)(*plVar5 + 0x130))(plVar5);
    uVar9 = uVar9 + iVar1;
  }
  uVar10 = (ulong)uVar9;
  pcVar7 = (char *)_IOMallocAligned(uVar10,1);
  if (pcVar7 == (char *)0x0) {
    uVar8 = 0;
  }
  else {
    _bzero(pcVar7,uVar10);
    if (pcVar2 != (char *)0x0) {
      _sprintf(pcVar7,"%s:",pcVar2);
    }
    if (pcVar4 != (char *)0x0) {
      _strcat(pcVar7,pcVar4);
      _strcat(pcVar7,":");
    }
    if (plVar5 != (long *)0x0) {
      pcVar2 = (char *)(**(code **)(*plVar5 + 0x148))(plVar5);
      _strcat(pcVar7,pcVar2);
      (**(code **)(*plVar5 + 0x28))(plVar5);
    }
    uVar8 = OSString::withCString(pcVar7);
    _IOFreeAligned(pcVar7,uVar10);
  }
  return uVar8;
}
