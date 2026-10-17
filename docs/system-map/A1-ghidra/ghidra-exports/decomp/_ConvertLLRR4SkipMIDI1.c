// _ConvertLLRR4SkipMIDI1
// Entry: 00028eeb
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

void _ConvertLLRR4SkipMIDI1(int param_1,long param_2,long param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int *piVar5;
  uint *puVar6;
  
  if (param_1 != 0) {
    piVar5 = (int *)(param_3 + 0xc);
    puVar6 = (uint *)(param_2 + 0xc);
    do {
      uVar1 = puVar6[-3];
      uVar2 = puVar6[-2];
      uVar3 = puVar6[-1];
      uVar4 = *puVar6;
      piVar5[-3] = (int)((uVar2 & 0xff0000) + ((uVar2 & 0xff00) << 8 | uVar2 >> 0x18) * 0x100) >> 8;
      piVar5[-2] = (int)((uVar4 & 0xff0000) + ((uVar4 & 0xff00) << 8 | uVar4 >> 0x18) * 0x100) >> 8;
      piVar5[-1] = (int)((uVar1 & 0xff0000) + ((uVar1 & 0xff00) << 8 | uVar1 >> 0x18) * 0x100) >> 8;
      *piVar5 = (int)((uVar3 & 0xff0000) + ((uVar3 & 0xff00) << 8 | uVar3 >> 0x18) * 0x100) >> 8;
      piVar5 = piVar5 + 4;
      puVar6 = puVar6 + 5;
      param_1 = param_1 + -4;
    } while (param_1 != 0);
  }
  return;
}
