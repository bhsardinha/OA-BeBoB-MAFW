// com_m_audio_FWMetaNub::FirmwareWriteCommand
// Entry: 0001be3c  Size: 53 bytes
// Prototype: undefined FirmwareWriteCommand(com_m_audio_FWMetaNub * this, MAFireWireRequestHeader * param_1, uint param_2)


/* com_m_audio_FWMetaNub::FirmwareWriteCommand(MAFireWireRequestHeader*, unsigned int) */

void __thiscall
com_m_audio_FWMetaNub::FirmwareWriteCommand
          (com_m_audio_FWMetaNub *this,MAFireWireRequestHeader *param_1,uint param_2)

{
  undefined2 local_10;
  undefined2 local_e;
  undefined4 local_c;
  
  local_10 = 0;
  local_e = 0xffff;
  local_c = 0xc8021000;
  *(undefined4 *)param_1 = *(undefined4 *)(this + 0x98);
  FireWireWriteCommand(this,(FWAddressStruct *)&local_10,param_2,param_1);
  return;
}


