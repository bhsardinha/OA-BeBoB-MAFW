// com_m_audio_FWMetaNub::FirmwareWriteCommand
// Entry: 0001be72
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::FirmwareWriteCommand(unsigned int, unsigned int) */

void __thiscall
com_m_audio_FWMetaNub::FirmwareWriteCommand(com_m_audio_FWMetaNub *this,uint param_1,uint param_2)

{
  undefined4 local_20;
  uint local_1c;
  uint local_18;
  undefined2 local_10;
  undefined2 local_e;
  undefined4 local_c;
  
  local_10 = 0;
  local_e = 0xffff;
  local_c = 0xc8021000;
  local_20 = *(undefined4 *)(this + 0x98);
  local_1c = param_1;
  local_18 = param_2;
  FireWireWriteCommand(this,(FWAddressStruct *)&local_10,0xc,&local_20);
  return;
}
