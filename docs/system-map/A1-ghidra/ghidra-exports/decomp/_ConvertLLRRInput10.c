// _ConvertLLRRInput10
// Entry: 00028fa7
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _ConvertLLRRInput10(uint param_1,long param_2,long param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  long lVar11;
  
  if (param_1 != 0) {
    lVar11 = 0;
    do {
      uVar1 = *(uint *)(param_2 + lVar11 * 4);
      uVar2 = *(uint *)(param_2 + 0x14 + lVar11 * 4);
      uVar3 = *(uint *)(param_2 + 4 + lVar11 * 4);
      uVar4 = *(uint *)(param_2 + 0x18 + lVar11 * 4);
      uVar5 = *(uint *)(param_2 + 8 + lVar11 * 4);
      uVar6 = *(uint *)(param_2 + 0x1c + lVar11 * 4);
      uVar7 = *(uint *)(param_2 + 0xc + lVar11 * 4);
      uVar8 = *(uint *)(param_2 + 0x20 + lVar11 * 4);
      uVar9 = *(uint *)(param_2 + 0x10 + lVar11 * 4);
      uVar10 = *(uint *)(param_2 + 0x24 + lVar11 * 4);
      *(int *)(param_3 + lVar11 * 4) =
           (int)((uVar1 & 0xff0000) + ((uVar1 & 0xff00) << 8 | uVar1 >> 0x18) * 0x100) >> 8;
      *(int *)(param_3 + 4 + lVar11 * 4) =
           (int)((uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 8 + lVar11 * 4) =
           (int)((uVar3 >> 0x18 | (uVar3 & 0xff0000) >> 8 | (uVar3 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0xc + lVar11 * 4) =
           (int)((uVar4 >> 0x18 | (uVar4 & 0xff0000) >> 8 | (uVar4 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0x10 + lVar11 * 4) =
           (int)((uVar5 >> 0x18 | (uVar5 & 0xff0000) >> 8 | (uVar5 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0x14 + lVar11 * 4) =
           (int)((uVar6 >> 0x18 | (uVar6 & 0xff0000) >> 8 | (uVar6 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0x18 + lVar11 * 4) =
           (int)((uVar7 >> 0x18 | (uVar7 & 0xff0000) >> 8 | (uVar7 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0x1c + lVar11 * 4) =
           (int)((uVar8 >> 0x18 | (uVar8 & 0xff0000) >> 8 | (uVar8 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0x20 + lVar11 * 4) =
           (int)((uVar9 >> 0x18 | (uVar9 & 0xff0000) >> 8 | (uVar9 & 0xff00) << 8) << 8) >> 8;
      *(int *)(param_3 + 0x24 + lVar11 * 4) =
           (int)((uVar10 >> 0x18 | (uVar10 & 0xff0000) >> 8 | (uVar10 & 0xff00) << 8) << 8) >> 8;
      lVar11 = lVar11 + 10;
    } while ((uint)lVar11 < param_1);
  }
  return;
}
