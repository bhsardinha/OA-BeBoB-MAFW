// com_m_audio_FWMetaNub::FirmwareStart
// Entry: 0001beb4  Size: 221 bytes
// Prototype: undefined FirmwareStart(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::FirmwareStart() */

undefined4 __thiscall com_m_audio_FWMetaNub::FirmwareStart(com_m_audio_FWMetaNub *this)

{
  undefined4 uVar1;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined2 local_20;
  undefined2 local_1e;
  undefined4 local_1c;
  
  *(undefined4 *)(this + 0x88) = 3;
  if (this[0xf0] == (com_m_audio_FWMetaNub)0x0) {
    if (this[0xf3] == (com_m_audio_FWMetaNub)0x0) {
      _IOLog("M-FW: Firmware active: registering metanub%c\n",0x2e);
      this[0xf3] = (com_m_audio_FWMetaNub)0x1;
      (**(code **)(*(long *)this + 0x5b0))(this,0);
    }
    else {
      _IOLog("M-FW: Firnware active and we already started matching.%c\n",0x2e);
    }
    uVar1 = 0;
  }
  else {
    _IOLog("M-FW: Bootloader active: starting firmware%c\n",0x2e);
    local_2c = 0x1110000;
    local_28 = 0;
    local_20 = 0;
    local_1e = 0xffff;
    local_1c = 0xc8021000;
    local_30 = *(undefined4 *)(this + 0x98);
    uVar1 = FireWireWriteCommand(this,(FWAddressStruct *)&local_20,0xc,&local_30);
    ReregisterDeviceAfterFirmwareReset(this);
  }
  return uVar1;
}


