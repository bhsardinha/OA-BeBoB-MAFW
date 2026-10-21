// com_m_audio_FW410Device::SetInputPan
// Entry: 00014e7a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::SetInputPan(unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FW410Device::SetInputPan(com_m_audio_FW410Device *this,uint param_1,uint param_2)

{
  undefined *puVar1;
  uint uVar2;
  undefined8 uVar3;
  ulong uVar4;
  
  uVar2 = param_1 - 1;
  if (uVar2 < 4) {
    uVar4 = (ulong)uVar2;
    *(ushort *)(this + (long)(&UNK_00001110 + uVar4 * 2)) =
         ((ushort)(-param_2 >> 1) & 0xff) + (short)param_2 * -0x8000;
    if ((param_1 & 1) == 0) {
      puVar1 = &UNK_0000110e + uVar4 * 2;
    }
    else {
      puVar1 = &UNK_00001110 + uVar4 * 2;
      uVar2 = param_1;
    }
                    /* WARNING: Could not recover jumptable at 0x00014ed0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar3 = (**(code **)(*(long *)this + 0xba8))(this,(uVar2 - 1) * 2 + 0x38,1,this + (long)puVar1);
    return uVar3;
  }
  return 0xe00002c2;
}
