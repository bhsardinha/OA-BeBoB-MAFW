// com_m_audio_FW410Device::SetSWReturnRouting
// Entry: 00014076
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::SetSWReturnRouting(unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FW410Device::SetSWReturnRouting(com_m_audio_FW410Device *this,uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  
  switch(param_1) {
  case 1:
    uVar1 = (*(uint *)(this + 0x113c) & 0x8cffffff) + 0x1000000;
    if ((param_2 & 1) == 0) {
      uVar1 = *(uint *)(this + 0x113c) & 0x8cffffff;
    }
    uVar2 = uVar1 | 0x2000000;
    if ((param_2 & 2) == 0) {
      uVar2 = uVar1;
    }
    uVar1 = uVar2 | 0x10000000;
    if ((param_2 & 4) == 0) {
      uVar1 = uVar2;
    }
    uVar2 = uVar1 | 0x20000000;
    if ((param_2 & 8) == 0) {
      uVar2 = uVar1;
    }
    *(uint *)(this + 0x113c) = uVar2;
    if ((param_2 & 0x10) == 0) {
      return 0;
    }
    uVar2 = uVar2 | 0x40000000;
    break;
  case 2:
    uVar1 = (*(uint *)(this + 0x113c) & 0x73fcffff) + 0x4000000;
    if ((param_2 & 1) == 0) {
      uVar1 = *(uint *)(this + 0x113c) & 0x73fcffff;
    }
    uVar2 = uVar1 | 0x8000000;
    if ((param_2 & 2) == 0) {
      uVar2 = uVar1;
    }
    uVar1 = uVar2 | 0x80000000;
    if ((param_2 & 4) == 0) {
      uVar1 = uVar2;
    }
    uVar2 = uVar1 | 0x10000;
    if ((param_2 & 8) == 0) {
      uVar2 = uVar1;
    }
    *(uint *)(this + 0x113c) = uVar2;
    if ((param_2 & 0x10) == 0) {
      return 0;
    }
    uVar2 = uVar2 | 0x20000;
    break;
  case 3:
    uVar1 = (*(uint *)(this + 0x113c) & 0xff83ffff) + 0x40000;
    if ((param_2 & 1) == 0) {
      uVar1 = *(uint *)(this + 0x113c) & 0xff83ffff;
    }
    uVar2 = uVar1 | 0x80000;
    if ((param_2 & 2) == 0) {
      uVar2 = uVar1;
    }
    uVar1 = uVar2 | 0x100000;
    if ((param_2 & 4) == 0) {
      uVar1 = uVar2;
    }
    uVar2 = uVar1 | 0x200000;
    if ((param_2 & 8) == 0) {
      uVar2 = uVar1;
    }
    *(uint *)(this + 0x113c) = uVar2;
    if ((param_2 & 0x10) == 0) {
      return 0;
    }
    uVar2 = uVar2 | 0x400000;
    break;
  case 4:
    uVar1 = (*(uint *)(this + 0x113c) & 0xff7ff0ff) + 0x800000;
    if ((param_2 & 1) == 0) {
      uVar1 = *(uint *)(this + 0x113c) & 0xff7ff0ff;
    }
    uVar2 = uVar1 | 0x100;
    if ((param_2 & 2) == 0) {
      uVar2 = uVar1;
    }
    uVar1 = uVar2 | 0x200;
    if ((param_2 & 4) == 0) {
      uVar1 = uVar2;
    }
    uVar2 = uVar1 | 0x400;
    if ((param_2 & 8) == 0) {
      uVar2 = uVar1;
    }
    *(uint *)(this + 0x113c) = uVar2;
    if ((param_2 & 0x10) == 0) {
      return 0;
    }
    uVar2 = uVar2 | 0x800;
    break;
  case 5:
    uVar1 = (*(uint *)(this + 0x113c) & 0xffff0ffe) + 0x1000;
    if ((param_2 & 1) == 0) {
      uVar1 = *(uint *)(this + 0x113c) & 0xffff0ffe;
    }
    uVar2 = uVar1 | 0x2000;
    if ((param_2 & 2) == 0) {
      uVar2 = uVar1;
    }
    uVar1 = uVar2 | 0x4000;
    if ((param_2 & 4) == 0) {
      uVar1 = uVar2;
    }
    uVar2 = uVar1 | 0x8000;
    if ((param_2 & 8) == 0) {
      uVar2 = uVar1;
    }
    *(uint *)(this + 0x113c) = uVar2;
    if ((param_2 & 0x10) == 0) {
      return 0;
    }
    uVar2 = uVar2 | 1;
    break;
  default:
    return 0xe00002c2;
  }
  *(uint *)(this + 0x113c) = uVar2;
  return 0;
}
