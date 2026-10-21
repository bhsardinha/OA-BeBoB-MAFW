// com_m_audio_FW410Device::ReadHSCIData
// Entry: 00015198
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::ReadHSCIData(unsigned int*) */

void __thiscall com_m_audio_FW410Device::ReadHSCIData(com_m_audio_FW410Device *this,uint *param_1)

{
  uint uVar1;
  char cVar2;
  int local_1c;
  
  **(uint **)(this + 0x108) = (*param_1 & 0xff00) * 0x100 + *param_1 * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 4) =
       (param_1[1] & 0xff00) * 0x100 + param_1[1] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 8) =
       (param_1[2] & 0xff00) * 0x100 + param_1[2] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0xc) =
       (param_1[3] & 0xff00) * 0x100 + param_1[3] * 0x1000000 >> 0x10;
  *(undefined4 *)(*(long *)(this + 0x108) + 0x10) = *(undefined4 *)(*(long *)(this + 0x110) + 0x10);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x14) = *(undefined4 *)(*(long *)(this + 0x110) + 0x14);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x18) = *(undefined4 *)(*(long *)(this + 0x110) + 0x18);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x1c) = *(undefined4 *)(*(long *)(this + 0x110) + 0x1c);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x20) = *(undefined4 *)(*(long *)(this + 0x110) + 0x20);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x24) = *(undefined4 *)(*(long *)(this + 0x110) + 0x24);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x28) = *(undefined4 *)(*(long *)(this + 0x110) + 0x28);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x2c) = *(undefined4 *)(*(long *)(this + 0x110) + 0x2c);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x30) = *(undefined4 *)(*(long *)(this + 0x110) + 0x30);
  *(undefined4 *)(*(long *)(this + 0x108) + 0x34) = *(undefined4 *)(*(long *)(this + 0x110) + 0x34);
  *(uint *)(*(long *)(this + 0x108) + 0x38) =
       (param_1[4] & 0xff00) * 0x100 + param_1[4] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x3c) =
       (param_1[5] & 0xff00) * 0x100 + param_1[5] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x40) =
       (param_1[6] & 0xff00) * 0x100 + param_1[6] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x44) =
       (param_1[7] & 0xff00) * 0x100 + param_1[7] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x48) =
       (param_1[8] & 0xff00) * 0x100 + param_1[8] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x4c) =
       (param_1[9] & 0xff00) * 0x100 + param_1[9] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x50) =
       (param_1[10] & 0xff00) * 0x100 + param_1[10] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x54) =
       (param_1[0xb] & 0xff00) * 0x100 + param_1[0xb] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x58) =
       (param_1[0xc] & 0xff00) * 0x100 + param_1[0xc] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x5c) =
       (param_1[0xd] & 0xff00) * 0x100 + param_1[0xd] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x60) =
       (param_1[0xe] & 0xff00) * 0x100 + param_1[0xe] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 100) =
       (param_1[0xf] & 0xff00) * 0x100 + param_1[0xf] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x68) =
       (param_1[0x10] & 0xff00) * 0x100 + param_1[0x10] * 0x1000000 >> 0x10;
  *(uint *)(*(long *)(this + 0x108) + 0x6c) =
       (param_1[0x11] & 0xff00) * 0x100 + param_1[0x11] * 0x1000000 >> 0x10;
  uVar1 = param_1[0x12];
  m_audio_b_FWAudioDevice::DetectExternalSyncChanges
            ((m_audio_b_FWAudioDevice *)this,uVar1 >> 0x18 & 1,uVar1 >> 0x10 & 7);
  cVar2 = TRotaryControl::RotaryValueChanged
                    ((TRotaryControl *)(this + 0x2d0),uVar1 >> 8 & 0xff,&local_1c);
  if (cVar2 != '\0') {
    FWSettingsRotaryControl::UserRotatedHardwareKnob
              ((FWSettingsRotaryControl *)(this + 0x2d8),local_1c);
  }
  return;
}
