// _NativeInt32ToFloat32_X86
// Entry: 0002c8e0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _NativeInt32ToFloat32_X86(int *param_1,float *param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  double dVar8;
  float fVar9;
  uint uVar10;
  ulong uVar11;
  uint uVar12;
  uint uVar13;
  float *pfVar14;
  int *piVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  float fVar22;
  
  dVar8 = DAT_0002ffd8;
  if (param_3 < 4) {
    if (param_3 == 0) {
      return;
    }
    uVar11 = (ulong)param_3;
    do {
      *param_2 = (float)((double)*param_1 * dVar8);
      param_2 = param_2 + 1;
      param_1 = param_1 + 1;
      uVar11 = uVar11 - 1;
    } while (uVar11 != 0);
    return;
  }
  uVar13 = (uint)param_2;
  pfVar14 = param_2;
  piVar15 = param_1;
  uVar10 = param_3;
  fVar16 = _DAT_0002fe80;
  fVar17 = _UNK_0002fe84;
  fVar18 = _UNK_0002fe88;
  fVar9 = _UNK_0002fe8c;
  if (((uVar13 | (uint)param_1) & 0xf) != 0) {
    fVar16 = (float)param_1[1] * _UNK_0002fe84;
    fVar17 = (float)param_1[2] * _UNK_0002fe88;
    fVar18 = (float)param_1[3] * _UNK_0002fe8c;
    uVar12 = 0x10 - (uVar13 & 0xf) >> 2;
    uVar10 = param_3 - uVar12;
    *param_2 = (float)*param_1 * _DAT_0002fe80;
    param_2[1] = fVar16;
    param_2[2] = fVar17;
    param_2[3] = fVar18;
    fVar7 = _UNK_0002fe8c;
    fVar6 = _UNK_0002fe88;
    fVar5 = _UNK_0002fe84;
    fVar4 = _DAT_0002fe80;
    fVar16 = _DAT_0002fe80;
    fVar17 = _UNK_0002fe84;
    fVar18 = _UNK_0002fe88;
    fVar9 = _UNK_0002fe8c;
    if (((ulong)(param_1 + uVar12) & 0xf) != 0) {
      if (3 < uVar10) {
        uVar10 = 0x10 - (uVar13 & 0xf) >> 2;
        pfVar14 = param_2 + uVar10;
        piVar15 = param_1 + uVar10;
        uVar10 = param_3 - uVar10;
        do {
          iVar1 = piVar15[1];
          iVar2 = piVar15[2];
          iVar3 = piVar15[3];
          *pfVar14 = (float)*piVar15 * fVar4;
          pfVar14[1] = (float)iVar1 * fVar5;
          pfVar14[2] = (float)iVar2 * fVar6;
          pfVar14[3] = (float)iVar3 * fVar7;
          pfVar14 = pfVar14 + 4;
          piVar15 = piVar15 + 4;
          uVar10 = uVar10 - 4;
          fVar16 = _DAT_0002fe80;
          fVar17 = _UNK_0002fe84;
          fVar18 = _UNK_0002fe88;
          fVar9 = _UNK_0002fe8c;
        } while (3 < uVar10);
      }
      goto LAB_0002c99d;
    }
    pfVar14 = param_2 + uVar12;
    piVar15 = param_1 + uVar12;
  }
  for (; fVar7 = _UNK_0002fe8c, fVar6 = _UNK_0002fe88, fVar5 = _UNK_0002fe84, fVar4 = _DAT_0002fe80,
      3 < uVar10; uVar10 = uVar10 - 4) {
    fVar19 = (float)*piVar15 * _DAT_0002fe80;
    fVar20 = (float)piVar15[1] * _UNK_0002fe84;
    fVar21 = (float)piVar15[2] * _UNK_0002fe88;
    fVar22 = (float)piVar15[3] * _UNK_0002fe8c;
    _DAT_0002fe80 = fVar16;
    _UNK_0002fe84 = fVar17;
    _UNK_0002fe88 = fVar18;
    _UNK_0002fe8c = fVar9;
    *pfVar14 = fVar19;
    pfVar14[1] = fVar20;
    pfVar14[2] = fVar21;
    pfVar14[3] = fVar22;
    pfVar14 = pfVar14 + 4;
    piVar15 = piVar15 + 4;
    fVar16 = _DAT_0002fe80;
    fVar17 = _UNK_0002fe84;
    fVar18 = _UNK_0002fe88;
    fVar9 = _UNK_0002fe8c;
    _UNK_0002fe8c = fVar7;
    _UNK_0002fe88 = fVar6;
    _UNK_0002fe84 = fVar5;
    _DAT_0002fe80 = fVar4;
  }
LAB_0002c99d:
  _UNK_0002fe8c = fVar9;
  _UNK_0002fe88 = fVar18;
  _UNK_0002fe84 = fVar17;
  _DAT_0002fe80 = fVar16;
  if (uVar10 != 0) {
    param_1 = param_1 + ((ulong)param_3 - 4);
    fVar16 = (float)param_1[1] * _UNK_0002fe84;
    fVar17 = (float)param_1[2] * _UNK_0002fe88;
    fVar18 = (float)param_1[3] * _UNK_0002fe8c;
    param_2 = param_2 + ((ulong)param_3 - 4);
    *param_2 = (float)*param_1 * _DAT_0002fe80;
    param_2[1] = fVar16;
    param_2[2] = fVar17;
    param_2[3] = fVar18;
  }
  return;
}
