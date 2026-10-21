// com_m_audio_FWMetaNub::FirmwareReadResponse
// Entry: 0001c1f2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::FirmwareReadResponse(MAFireWireResponseDevice*, unsigned int) */

int __thiscall
com_m_audio_FWMetaNub::FirmwareReadResponse
          (com_m_audio_FWMetaNub *this,MAFireWireResponseDevice *param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  undefined2 local_38;
  undefined2 local_36;
  undefined4 local_34;
  
  local_38 = 0;
  local_36 = 0xffff;
  local_34 = 0xc8029000;
  iVar1 = 0;
  iVar2 = 3;
  do {
    if (iVar1 + 0x1ffffd2bU < 2) {
      _IOSleep(2000);
    }
    iVar1 = FireWireReadCommand(this,(FWAddressStruct *)&local_38,param_2,param_1);
    if (iVar1 != 0) {
      _IOLog("M-FW: Retry (%u) status %X submitting read command \n",iVar2,iVar1);
    }
  } while ((iVar1 + 0x1ffffd2bU < 2) && (bVar3 = iVar2 != 0, iVar2 = iVar2 + -1, bVar3));
  return iVar1;
}
