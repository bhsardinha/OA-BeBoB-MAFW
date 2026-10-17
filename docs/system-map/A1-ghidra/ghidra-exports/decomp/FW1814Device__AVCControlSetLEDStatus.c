// com_m_audio_FW1814Device::AVCControlSetLEDStatus
// Entry: 0000daaa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::AVCControlSetLEDStatus(unsigned int) */

void __thiscall
com_m_audio_FW1814Device::AVCControlSetLEDStatus(com_m_audio_FW1814Device *this,uint param_1)

{
  undefined1 *puVar1;
  undefined1 local_44 [12];
  undefined1 *local_38;
  uint local_30;
  undefined1 local_2c [16];
  undefined4 local_1c;
  
  local_30 = 8;
  local_38 = local_2c;
  _bzero(local_38,8);
  _bzero(local_44,8);
  *local_38 = 0;
  puVar1 = local_38;
  if (1 < local_30) {
    puVar1 = local_38 + 1;
  }
  *puVar1 = 0xff;
  puVar1 = local_38;
  if (2 < local_30) {
    puVar1 = local_38 + 2;
  }
  *puVar1 = 0;
  puVar1 = local_38;
  if (3 < local_30) {
    puVar1 = local_38 + 3;
  }
  *puVar1 = 3;
  puVar1 = local_38;
  if (4 < local_30) {
    puVar1 = local_38 + 4;
  }
  *puVar1 = 0;
  puVar1 = local_38;
  if (5 < local_30) {
    puVar1 = local_38 + 5;
  }
  *puVar1 = 1;
  puVar1 = local_38;
  if (6 < local_30) {
    puVar1 = local_38 + 6;
  }
  *puVar1 = param_1 != 1;
  puVar1 = local_38;
  if (7 < local_30) {
    puVar1 = local_38 + 7;
  }
  *puVar1 = 0;
  local_1c = 8;
  (**(code **)(**(long **)(this + 0x268) + 0x8b8))
            (*(long **)(this + 0x268),local_38,8,local_44,&local_1c);
  return;
}
