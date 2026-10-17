// com_m_audio_FW1814Device::SetInputPan
// Entry: 0000dd3e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetInputPan(unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FW1814Device::SetInputPan(com_m_audio_FW1814Device *this,uint param_1,uint param_2)

{
  undefined *puVar1;
  uint uVar2;
  undefined8 uVar3;
  ulong uVar4;
  
  uVar2 = param_1 - 1;
  if (uVar2 < 0x12) {
    uVar4 = (ulong)uVar2;
    *(ushort *)(this + (long)(&UNK_000012b8 + uVar4 * 2)) =
         ((ushort)(-param_2 >> 1) & 0xff) + (short)param_2 * -0x8000;
    if ((param_1 & 1) == 0) {
      puVar1 = &UNK_000012b6 + uVar4 * 2;
    }
    else {
      puVar1 = &UNK_000012b8 + uVar4 * 2;
      uVar2 = param_1;
    }
                    /* WARNING: Could not recover jumptable at 0x0000dd94. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar3 = (**(code **)(*(long *)this + 0xba8))(this,uVar2 * 2 + 0x3e,1,this + (long)puVar1);
    return uVar3;
  }
  return 0xe00002c2;
}
