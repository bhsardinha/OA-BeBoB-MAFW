// com_m_audio_FWMetaNub::DownloadSectionStart
// Entry: 0001c468  Size: 324 bytes
// Prototype: undefined DownloadSectionStart(com_m_audio_FWMetaNub * this, uint param_1, uint param_2, uint param_3, uint param_4, uint param_5)


/* com_m_audio_FWMetaNub::DownloadSectionStart(unsigned int, unsigned int, unsigned int, unsigned
   int, unsigned int) */

ulong __thiscall
com_m_audio_FWMetaNub::DownloadSectionStart
          (com_m_audio_FWMetaNub *this,uint param_1,uint param_2,uint param_3,uint param_4,
          uint param_5)

{
  long lVar1;
  uint uVar2;
  ulong uVar3;
  MAFireWireResponseDevice local_b0 [72];
  undefined4 local_68;
  undefined4 local_64;
  uint local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  uint local_44;
  uint local_40;
  uint local_3c;
  undefined2 local_38;
  undefined2 local_36;
  undefined4 local_34;
  
  local_64 = 0xa040000;
  lVar1 = *(long *)(this + 0x140);
  local_5c = *(undefined4 *)(lVar1 + 8);
  local_58 = *(undefined4 *)(lVar1 + 0xc);
  local_54 = *(undefined4 *)(lVar1 + 0x10);
  local_50 = *(undefined4 *)(lVar1 + 0x14);
  local_4c = *(undefined4 *)(lVar1 + 0x18);
  local_48 = *(undefined4 *)(lVar1 + 0x1c);
  local_38 = 0;
  local_36 = 0xffff;
  local_34 = 0xc8021000;
  local_68 = *(undefined4 *)(this + 0x98);
  local_60 = param_1;
  local_44 = param_2;
  local_40 = param_3;
  local_3c = param_4;
  uVar3 = FireWireWriteCommand(this,(FWAddressStruct *)&local_38,0x30,&local_68);
  if ((int)uVar3 == 0) {
    uVar2 = FirmwareReadResponse(this,local_b0,0x28);
    uVar3 = (ulong)uVar2;
    if (uVar2 == 0) {
      *(ulong *)(this + 0x148) = (ulong)param_5;
      *(ulong *)(this + 0x178) = (ulong)param_2;
      *(ulong *)(this + 0x160) = (ulong)param_3;
      *(undefined8 *)(this + 0x150) = 0;
      *(undefined8 *)(this + 0x168) = 0;
      *(undefined8 *)(this + 0x170) = 0;
      *(ulong *)(this + 0x180) = (ulong)param_4;
      *(undefined8 *)(this + 0x158) = 0x200;
      _IOLog("M-FW: FirmUp- upload start successful..%c\n",0x2e);
    }
  }
  return uVar3;
}


