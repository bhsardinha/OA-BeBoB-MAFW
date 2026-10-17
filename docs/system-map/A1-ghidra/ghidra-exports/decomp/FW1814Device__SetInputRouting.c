// com_m_audio_FW1814Device::SetInputRouting
// Entry: 0000dbea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetInputRouting(unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FW1814Device::SetInputRouting(com_m_audio_FW1814Device *this,uint param_1,uint param_2)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  
  if ((param_1 == 0) ||
     (uVar2 = (**(code **)(*(long *)(this + 0x13e0) + 0x10))(this + 0x13e0), uVar2 >> 1 < param_1))
  {
    return 0xe00002c2;
  }
  cVar1 = (char)param_1;
  if (param_1 < 5) {
    uVar2 = 1 << (cVar1 - 1U & 0x1f);
    uVar2 = uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8 | uVar2 << 0x18;
    if ((param_2 & 1) == 0) {
      uVar2 = ~uVar2 & *(uint *)(this + 0x1308);
    }
    else {
      uVar2 = uVar2 | *(uint *)(this + 0x1308);
    }
    *(uint *)(this + 0x1308) = uVar2;
    uVar3 = 1 << (cVar1 + 3U & 0x1f);
    uVar3 = uVar3 >> 0x18 | (uVar3 & 0xff0000) >> 8 | (uVar3 & 0xff00) << 8 | uVar3 << 0x18;
    if ((param_2 & 2) == 0) {
      uVar3 = ~uVar3 & uVar2;
    }
    else {
      uVar3 = uVar3 | uVar2;
    }
  }
  else {
    if (5 < param_1) {
      if (param_1 < 10) {
        uVar2 = 1 << (cVar1 + 2U & 0x1f);
        uVar2 = uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8 | uVar2 << 0x18;
        if ((param_2 & 1) == 0) {
          uVar2 = ~uVar2 & *(uint *)(this + 0x1308);
        }
        else {
          uVar2 = uVar2 | *(uint *)(this + 0x1308);
        }
        *(uint *)(this + 0x1308) = uVar2;
        uVar3 = 1 << (cVar1 + 6U & 0x1f);
        uVar3 = uVar3 >> 0x18 | (uVar3 & 0xff0000) >> 8 | (uVar3 & 0xff00) << 8 | uVar3 << 0x18;
        if ((param_2 & 2) == 0) {
          uVar3 = ~uVar3 & uVar2;
        }
        else {
          uVar3 = uVar3 | uVar2;
        }
        *(uint *)(this + 0x1308) = uVar3;
      }
      goto LAB_0000dd0d;
    }
    uVar2 = *(uint *)(this + 0x1308) | 0x100;
    if ((param_2 & 1) == 0) {
      uVar2 = *(uint *)(this + 0x1308) & 0xfffffeff;
    }
    uVar3 = uVar2 | 0x200;
    if ((param_2 & 2) == 0) {
      uVar3 = uVar2 & 0xfffffdff;
    }
  }
  *(uint *)(this + 0x1308) = uVar3;
LAB_0000dd0d:
  (**(code **)(*(long *)this + 0xba8))(this,0x90,1,this + 0x1308);
  return 0;
}
