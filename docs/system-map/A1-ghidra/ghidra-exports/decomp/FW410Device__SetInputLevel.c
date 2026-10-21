// com_m_audio_FW410Device::SetInputLevel
// Entry: 00014cf8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::SetInputLevel(unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FW410Device::SetInputLevel(com_m_audio_FW410Device *this,uint param_1,uint param_2)

{
  undefined8 uVar1;
  uint uVar2;
  short sVar3;
  
  uVar2 = param_1 - 1;
  if (uVar2 < 4) {
    sVar3 = ((ushort)(param_2 >> 1) & 0xff) + (short)(param_2 << 0xf);
    if (sVar3 != *(short *)(this + (long)(&UNK_00001100 + (ulong)uVar2 * 2))) {
      *(short *)(this + (long)(&UNK_00001100 + (ulong)uVar2 * 2)) = sVar3;
                    /* WARNING: Could not recover jumptable at 0x00014d57. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      uVar1 = (**(code **)(*(long *)this + 0xba8))
                        (this,(2 < param_1) * '\x04' + '(',1,
                         this + (long)(&UNK_00001100 + (ulong)(2 < param_1) * 4));
      return uVar1;
    }
    uVar1 = 0;
  }
  else {
    uVar1 = 0xe00002c2;
  }
  return uVar1;
}
