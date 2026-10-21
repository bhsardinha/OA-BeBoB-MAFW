// com_m_audio_FWMetaNub::FirmwareUpdate
// Entry: 0001c908
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::FirmwareUpdate() */

int __thiscall com_m_audio_FWMetaNub::FirmwareUpdate(com_m_audio_FWMetaNub *this)

{
  int iVar1;
  ushort local_26;
  ushort local_24 [3];
  ushort local_1e;
  ushort local_1c [2];
  
  this[0xf2] = (com_m_audio_FWMetaNub)0x0;
  this[0xf1] = (com_m_audio_FWMetaNub)0x1;
  *(undefined4 *)(this + 0x88) = 5;
  if (*(uint **)(this + 0xe0) != (uint *)0x0) {
    IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0xe0),local_1c,&local_1e);
  }
  iVar1 = *(int *)(*(long *)(this + 0x140) + 0x34);
  if ((iVar1 == 0x400c0080) || (iVar1 == 0x400b0080)) {
    iVar1 = FirmwareUpdateBeBoB32(this);
LAB_0001c97f:
    _IOLog("M-FW: FirmUp- status:%X\n",iVar1);
    this[0xf1] = (com_m_audio_FWMetaNub)0x0;
    if (iVar1 == 0) goto LAB_0001c9fa;
  }
  else {
    if (iVar1 == 0x20080000) {
      iVar1 = FirmwareUpdateBeBoB251(this);
      goto LAB_0001c97f;
    }
    _IOLog("M-FW: FirmUp- Unknown firmware base address %X\n");
    iVar1 = -0x1ffffd44;
    _IOLog("M-FW: FirmUp- status:%X\n",0xe00002bc);
    this[0xf1] = (com_m_audio_FWMetaNub)0x0;
  }
  _IOLog("M-FW: FirmUp- Error during firmware update, %X\n",iVar1);
  (**(code **)(**(long **)(this + 0xe8) + 0x880))();
  _IOSleep(0xdac);
LAB_0001c9fa:
  if (*(uint **)(this + 0xe0) != (uint *)0x0) {
    IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0xe0),local_24,&local_26);
  }
  return iVar1;
}
