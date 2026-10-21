// com_m_audio_FW410Device::SetHeadphoneSource
// Entry: 000144bc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::SetHeadphoneSource(unsigned int) */

void __thiscall
com_m_audio_FW410Device::SetHeadphoneSource(com_m_audio_FW410Device *this,uint param_1)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = (*(uint *)(this + 0x1140) & 0xc4ffffff) + 0x1000000;
  if ((param_1 & 1) == 0) {
    uVar1 = *(uint *)(this + 0x1140) & 0xc4ffffff;
  }
  uVar2 = uVar1 | 0x2000000;
  if ((param_1 & 2) == 0) {
    uVar2 = uVar1;
  }
  uVar1 = uVar2 | 0x8000000;
  if ((param_1 & 4) == 0) {
    uVar1 = uVar2;
  }
  uVar2 = uVar1 | 0x10000000;
  if ((param_1 & 8) == 0) {
    uVar2 = uVar1;
  }
  uVar1 = uVar2 | 0x20000000;
  if ((param_1 & 0x10) == 0) {
    uVar1 = uVar2;
  }
  uVar2 = uVar1 | 0x4000000;
  if ((param_1 & 0x20) == 0) {
    uVar2 = uVar1;
  }
  *(uint *)(this + 0x1140) = uVar2;
                    /* WARNING: Could not recover jumptable at 0x00014545. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xba8))(this,0x68,1,this + 0x1140,*(code **)(*(long *)this + 0xba8));
  return;
}
