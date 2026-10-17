// _ConvertLRLRInputToSInt32
// Entry: 00029357
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _ConvertLRLRInputToSInt32(uint param_1,long param_2,long param_3)

{
  int *piVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  ulong uVar6;
  ulong uVar7;
  
  if (param_1 < 4) {
    uVar6 = 0;
  }
  else {
    uVar6 = 0;
    do {
      uVar5 = *(uint *)(param_2 + uVar6 * 4);
      uVar4 = *(uint *)(param_2 + 4 + uVar6 * 4);
      uVar2 = *(uint *)(param_2 + 8 + uVar6 * 4);
      uVar3 = *(uint *)(param_2 + 0xc + uVar6 * 4);
      *(int *)(param_3 + uVar6 * 4) =
           (int)(((uVar5 >> 8 & 0xff00) + (uVar5 >> 0x18) + (uVar5 & 0xff00) * 0x100) * 0x100) >> 8;
      *(int *)(param_3 + 4 + uVar6 * 4) =
           (int)(((uVar4 >> 8 & 0xff00) + (uVar4 >> 0x18) + (uVar4 & 0xff00) * 0x100) * 0x100) >> 8;
      *(int *)(param_3 + 8 + uVar6 * 4) =
           (int)(((uVar2 >> 8 & 0xff00) + (uVar2 >> 0x18) + (uVar2 & 0xff00) * 0x100) * 0x100) >> 8;
      *(int *)(param_3 + 0xc + uVar6 * 4) =
           (int)(((uVar3 >> 8 & 0xff00) + (uVar3 >> 0x18) + (uVar3 & 0xff00) * 0x100) * 0x100) >> 8;
      uVar6 = uVar6 + 4;
    } while ((uint)uVar6 < (param_1 & 0xfffffffc));
  }
  while (uVar5 = (uint)uVar6, uVar5 < param_1) {
    uVar6 = uVar6 & 0xffffffff;
    uVar4 = *(uint *)(param_2 + uVar6 * 4);
    uVar7 = (ulong)(uVar5 | 1);
    *(uint *)(param_3 + uVar6 * 4) =
         ((uVar4 >> 8 & 0xff00) + (uVar4 >> 0x18) + (uVar4 & 0xff00) * 0x100) * 0x100;
    uVar4 = *(uint *)(param_2 + uVar7 * 4);
    *(uint *)(param_3 + uVar7 * 4) =
         ((uVar4 >> 8 & 0xff00) + (uVar4 >> 0x18) + (uVar4 & 0xff00) * 0x100) * 0x100;
    piVar1 = (int *)(param_3 + uVar6 * 4);
    *piVar1 = *piVar1 >> 8;
    piVar1 = (int *)(param_3 + uVar7 * 4);
    *piVar1 = *piVar1 >> 8;
    uVar6 = (ulong)(uVar5 + 2);
  }
  return;
}
