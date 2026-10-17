// _ConvertInputFramesOneStereoPair
// Entry: 000290b9
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _ConvertInputFramesOneStereoPair(uint param_1,long param_2,long param_3)

{
  uint uVar1;
  uint uVar2;
  long lVar3;
  
  if (param_1 != 0) {
    lVar3 = 0;
    do {
      uVar1 = *(uint *)(param_2 + lVar3 * 4);
      uVar2 = *(uint *)(param_2 + 4 + lVar3 * 4);
      *(int *)(param_3 + lVar3 * 4) =
           (int)((uVar1 & 0xff0000) + ((uVar1 & 0xff00) << 8 | uVar1 >> 0x18) * 0x100) >> 8;
      *(int *)(param_3 + 4 + lVar3 * 4) =
           (int)((uVar2 & 0xff0000) + ((uVar2 & 0xff00) << 8 | uVar2 >> 0x18) * 0x100) >> 8;
      lVar3 = lVar3 + 2;
    } while ((uint)lVar3 < param_1);
  }
  return;
}
