// com_m_audio_FW1814Device::SetHeadphoneSelector
// Entry: 0000d1f8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetHeadphoneSelector(unsigned int, unsigned int) */

void __thiscall
com_m_audio_FW1814Device::SetHeadphoneSelector
          (com_m_audio_FW1814Device *this,uint param_1,uint param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  
  uVar1 = *(uint *)(this + 0x1310) & 0xfffff8ff;
  if (param_1 == 0) {
    uVar1 = *(uint *)(this + 0x1310) & 0xf8ffffff;
  }
  *(uint *)(this + 0x1310) = uVar1;
  if (param_2 == 2) {
    iVar3 = (uint)(param_1 == 0) << 0x1a;
    iVar2 = (uint)(param_1 != 0) << 10;
  }
  else if (param_2 == 1) {
    iVar3 = (uint)(param_1 == 0) << 0x19;
    iVar2 = (uint)(param_1 != 0) << 9;
  }
  else {
    if (param_2 != 0) goto LAB_0000d27e;
    iVar3 = (uint)(param_1 == 0) << 0x18;
    iVar2 = (uint)(param_1 != 0) << 8;
  }
  *(uint *)(this + 0x1310) = iVar2 + iVar3 | uVar1;
LAB_0000d27e:
                    /* WARNING: Could not recover jumptable at 0x0000d296. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xba8))(this,0x98,1);
  return;
}
