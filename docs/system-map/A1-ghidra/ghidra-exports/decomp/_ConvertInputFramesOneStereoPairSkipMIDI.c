// _ConvertInputFramesOneStereoPairSkipMIDI
// Entry: 0002911b
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

void _ConvertInputFramesOneStereoPairSkipMIDI(uint param_1,long param_2,long param_3)

{
  uint uVar1;
  uint uVar2;
  long lVar3;
  uint *puVar4;
  
  if (param_1 != 0) {
    puVar4 = (uint *)(param_2 + 4);
    lVar3 = 0;
    do {
      uVar1 = puVar4[-1];
      uVar2 = *puVar4;
      *(int *)(param_3 + lVar3 * 4) =
           (int)((uVar1 & 0xff0000) + ((uVar1 & 0xff00) << 8 | uVar1 >> 0x18) * 0x100) >> 8;
      *(int *)(param_3 + 4 + lVar3 * 4) =
           (int)((uVar2 & 0xff0000) + ((uVar2 & 0xff00) << 8 | uVar2 >> 0x18) * 0x100) >> 8;
      lVar3 = lVar3 + 2;
      puVar4 = puVar4 + 3;
    } while ((uint)lVar3 < param_1);
  }
  return;
}
