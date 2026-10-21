// com_m_audio_FW410Device::SetInputRouting
// Entry: 00014d5a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::SetInputRouting(unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FW410Device::SetInputRouting(com_m_audio_FW410Device *this,uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  
  if ((param_1 != 0) &&
     (uVar1 = (**(code **)(*(long *)(this + 0x1310) + 0x10))(this + 0x1310), param_1 <= uVar1 >> 1))
  {
    if (param_1 == 1) {
      uVar1 = (*(uint *)(this + 0x1134) & 0xeeffbbfb) + 0x1000000;
      if ((param_2 & 1) == 0) {
        uVar1 = *(uint *)(this + 0x1134) & 0xeeffbbfb;
      }
      uVar2 = uVar1 | 0x10000000;
      if ((param_2 & 2) == 0) {
        uVar2 = uVar1;
      }
      uVar1 = uVar2 | 0x400;
      if ((param_2 & 4) == 0) {
        uVar1 = uVar2;
      }
      uVar2 = uVar1 | 0x4000;
      if ((param_2 & 8) == 0) {
        uVar2 = uVar1;
      }
      *(uint *)(this + 0x1134) = uVar2;
      if ((param_2 & 0x10) != 0) {
        *(uint *)(this + 0x1134) = uVar2 | 4;
      }
    }
    else {
      uVar1 = (*(uint *)(this + 0x1138) & 0xffffe0ff) + 0x100;
      if ((param_2 & 1) == 0) {
        uVar1 = *(uint *)(this + 0x1138) & 0xffffe0ff;
      }
      uVar2 = uVar1 | 0x200;
      if ((param_2 & 2) == 0) {
        uVar2 = uVar1;
      }
      uVar1 = uVar2 | 0x400;
      if ((param_2 & 4) == 0) {
        uVar1 = uVar2;
      }
      uVar2 = uVar1 | 0x800;
      if ((param_2 & 8) == 0) {
        uVar2 = uVar1;
      }
      *(uint *)(this + 0x1138) = uVar2;
      if ((param_2 & 0x10) != 0) {
        *(uint *)(this + 0x1138) = uVar2 | 0x1000;
      }
    }
    (**(code **)(*(long *)this + 0xba8))(this,0x5c,2,this + 0x1134);
    return 0;
  }
  return 0xe00002c2;
}
