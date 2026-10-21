// com_m_audio_FWMetaNub::GetActualProductName
// Entry: 0001ce06
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::GetActualProductName(char const*) */

undefined8 __thiscall
com_m_audio_FWMetaNub::GetActualProductName(com_m_audio_FWMetaNub *this,char *param_1)

{
  int iVar1;
  undefined8 uVar2;
  long lVar3;
  
  uVar2 = 0;
  lVar3 = 8;
  do {
    iVar1 = _strcmp(param_1,*(char **)(&UNK_00046448 + lVar3));
    if (iVar1 == 0) {
LAB_0001ce44:
      uVar2 = *(undefined8 *)(&UNK_00046448 + lVar3);
    }
    else {
      iVar1 = _strcmp(param_1,*(char **)((long)&kDeviceSpecs + lVar3));
      if (iVar1 == 0) goto LAB_0001ce44;
    }
    lVar3 = lVar3 + 0x18;
    if (lVar3 == 200) {
      return uVar2;
    }
  } while( true );
}
