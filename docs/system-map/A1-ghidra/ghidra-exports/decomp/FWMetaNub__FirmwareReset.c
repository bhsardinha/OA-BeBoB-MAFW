// com_m_audio_FWMetaNub::FirmwareReset
// Entry: 0001bf92  Size: 96 bytes
// Prototype: undefined FirmwareReset(com_m_audio_FWMetaNub * this, uint param_1)


/* com_m_audio_FWMetaNub::FirmwareReset(unsigned int) */

undefined4 __thiscall com_m_audio_FWMetaNub::FirmwareReset(com_m_audio_FWMetaNub *this,uint param_1)

{
  undefined4 uVar1;
  undefined4 local_30;
  undefined4 local_2c;
  uint local_28;
  undefined2 local_20;
  undefined2 local_1e;
  undefined4 local_1c;
  
  local_2c = 0x1020000;
  local_20 = 0;
  local_1e = 0xffff;
  local_1c = 0xc8021000;
  local_30 = *(undefined4 *)(this + 0x98);
  local_28 = param_1;
  uVar1 = FireWireWriteCommand(this,(FWAddressStruct *)&local_20,0xc,&local_30);
  ReregisterDeviceAfterFirmwareReset(this);
  return uVar1;
}


