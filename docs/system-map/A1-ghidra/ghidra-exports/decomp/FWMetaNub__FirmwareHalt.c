// com_m_audio_FWMetaNub::FirmwareHalt
// Entry: 0001bff2  Size: 123 bytes
// Prototype: undefined FirmwareHalt(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::FirmwareHalt() */

undefined4 __thiscall com_m_audio_FWMetaNub::FirmwareHalt(com_m_audio_FWMetaNub *this)

{
  undefined4 uVar1;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined2 local_20;
  undefined2 local_1e;
  undefined4 local_1c;
  
  *(undefined4 *)(this + 0x88) = 4;
  if (this[0xf0] == (com_m_audio_FWMetaNub)0x0) {
    local_2c = 0x1020000;
    local_28 = 1;
    local_20 = 0;
    local_1e = 0xffff;
    local_1c = 0xc8021000;
    local_30 = *(undefined4 *)(this + 0x98);
    uVar1 = FireWireWriteCommand(this,(FWAddressStruct *)&local_20,0xc,&local_30);
    ReregisterDeviceAfterFirmwareReset(this);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}


