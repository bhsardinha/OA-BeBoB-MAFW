// _SwapInt32ToFloat32_X86
// Entry: 0002c9e6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _SwapInt32ToFloat32_X86(undefined1 (*param_1) [16],undefined1 (*param_2) [16],uint param_3)

{
  undefined1 (*pauVar1) [16];
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  double dVar6;
  float fVar7;
  uint uVar8;
  ulong uVar9;
  uint uVar10;
  uint uVar11;
  float *pfVar12;
  undefined1 (*pauVar13) [16];
  float fVar14;
  float fVar15;
  float fVar16;
  undefined1 in_XMM1 [16];
  undefined1 auVar17 [16];
  undefined1 auVar18 [16];
  undefined1 auVar19 [16];
  undefined1 auVar20 [16];
  
  dVar6 = DAT_0002ffd8;
  if (param_3 < 4) {
    if (param_3 == 0) {
      return;
    }
    uVar9 = (ulong)param_3;
    do {
      uVar8 = *(uint *)*param_1;
      *(float *)*param_2 =
           (float)((double)(int)(uVar8 >> 0x18 | (uVar8 & 0xff0000) >> 8 | (uVar8 & 0xff00) << 8 |
                                uVar8 << 0x18) * dVar6);
      param_2 = (undefined1 (*) [16])(*param_2 + 4);
      param_1 = (undefined1 (*) [16])(*param_1 + 4);
      uVar9 = uVar9 - 1;
    } while (uVar9 != 0);
    return;
  }
  uVar11 = (uint)param_2;
  pauVar13 = param_2;
  pauVar1 = param_1;
  uVar8 = param_3;
  fVar14 = _DAT_0002fe90;
  fVar15 = _UNK_0002fe94;
  fVar16 = _UNK_0002fe98;
  fVar7 = _UNK_0002fe9c;
  if (((uVar11 | (uint)param_1) & 0xf) != 0) {
    auVar17 = pshuflw(*param_1,*param_1,0xb1);
    auVar17 = pshufhw(auVar17,auVar17,0xb1);
    in_XMM1._0_2_ = auVar17._0_2_ >> 8;
    in_XMM1._2_2_ = auVar17._2_2_ >> 8;
    in_XMM1._4_2_ = auVar17._4_2_ >> 8;
    in_XMM1._6_2_ = auVar17._6_2_ >> 8;
    in_XMM1._8_2_ = auVar17._8_2_ >> 8;
    in_XMM1._10_2_ = auVar17._10_2_ >> 8;
    in_XMM1._12_2_ = auVar17._12_2_ >> 8;
    in_XMM1._14_2_ = auVar17._14_2_ >> 8;
    auVar17 = psllw(auVar17,8);
    auVar17 = auVar17 | in_XMM1;
    fVar14 = (float)auVar17._4_4_ * _UNK_0002fe94;
    fVar15 = (float)auVar17._8_4_ * _UNK_0002fe98;
    fVar16 = (float)auVar17._12_4_ * _UNK_0002fe9c;
    uVar10 = 0x10 - (uVar11 & 0xf) >> 2;
    uVar8 = param_3 - uVar10;
    *(float *)*param_2 = (float)auVar17._0_4_ * _DAT_0002fe90;
    *(float *)(*param_2 + 4) = fVar14;
    *(float *)(*param_2 + 8) = fVar15;
    *(float *)(*param_2 + 0xc) = fVar16;
    fVar5 = _UNK_0002fe9c;
    fVar4 = _UNK_0002fe98;
    fVar3 = _UNK_0002fe94;
    fVar2 = _DAT_0002fe90;
    fVar14 = _DAT_0002fe90;
    fVar15 = _UNK_0002fe94;
    fVar16 = _UNK_0002fe98;
    fVar7 = _UNK_0002fe9c;
    if (((ulong)(*param_1 + (ulong)uVar10 * 4) & 0xf) != 0) {
      if (3 < uVar8) {
        uVar8 = 0x10 - (uVar11 & 0xf) >> 2;
        pfVar12 = (float *)(*param_2 + (ulong)uVar8 * 4);
        pauVar13 = (undefined1 (*) [16])(*param_1 + (ulong)uVar8 * 4);
        uVar8 = param_3 - uVar8;
        do {
          auVar17 = pshuflw(*pauVar13,*pauVar13,0xb1);
          auVar17 = pshufhw(auVar17,auVar17,0xb1);
          auVar20._0_2_ = auVar17._0_2_ >> 8;
          auVar20._2_2_ = auVar17._2_2_ >> 8;
          auVar20._4_2_ = auVar17._4_2_ >> 8;
          auVar20._6_2_ = auVar17._6_2_ >> 8;
          auVar20._8_2_ = auVar17._8_2_ >> 8;
          auVar20._10_2_ = auVar17._10_2_ >> 8;
          auVar20._12_2_ = auVar17._12_2_ >> 8;
          auVar20._14_2_ = auVar17._14_2_ >> 8;
          auVar17 = psllw(auVar17,8);
          auVar17 = auVar17 | auVar20;
          *pfVar12 = (float)auVar17._0_4_ * fVar2;
          pfVar12[1] = (float)auVar17._4_4_ * fVar3;
          pfVar12[2] = (float)auVar17._8_4_ * fVar4;
          pfVar12[3] = (float)auVar17._12_4_ * fVar5;
          pfVar12 = pfVar12 + 4;
          pauVar13 = pauVar13 + 1;
          uVar8 = uVar8 - 4;
          fVar14 = _DAT_0002fe90;
          fVar15 = _UNK_0002fe94;
          fVar16 = _UNK_0002fe98;
          fVar7 = _UNK_0002fe9c;
        } while (3 < uVar8);
      }
      goto LAB_0002cafd;
    }
    pauVar13 = (undefined1 (*) [16])(*param_2 + (ulong)uVar10 * 4);
    pauVar1 = (undefined1 (*) [16])(*param_1 + (ulong)uVar10 * 4);
  }
  for (; fVar5 = _UNK_0002fe9c, fVar4 = _UNK_0002fe98, fVar3 = _UNK_0002fe94, fVar2 = _DAT_0002fe90,
      3 < uVar8; uVar8 = uVar8 - 4) {
    auVar17 = pshuflw(in_XMM1,*pauVar1,0xb1);
    auVar17 = pshufhw(auVar17,auVar17,0xb1);
    auVar19._0_2_ = auVar17._0_2_ >> 8;
    auVar19._2_2_ = auVar17._2_2_ >> 8;
    auVar19._4_2_ = auVar17._4_2_ >> 8;
    auVar19._6_2_ = auVar17._6_2_ >> 8;
    auVar19._8_2_ = auVar17._8_2_ >> 8;
    auVar19._10_2_ = auVar17._10_2_ >> 8;
    auVar19._12_2_ = auVar17._12_2_ >> 8;
    auVar19._14_2_ = auVar17._14_2_ >> 8;
    auVar17 = psllw(auVar17,8);
    auVar17 = auVar17 | auVar19;
    in_XMM1._0_4_ = (float)auVar17._0_4_ * _DAT_0002fe90;
    in_XMM1._4_4_ = (float)auVar17._4_4_ * _UNK_0002fe94;
    in_XMM1._8_4_ = (float)auVar17._8_4_ * _UNK_0002fe98;
    in_XMM1._12_4_ = (float)auVar17._12_4_ * _UNK_0002fe9c;
    _DAT_0002fe90 = fVar14;
    _UNK_0002fe94 = fVar15;
    _UNK_0002fe98 = fVar16;
    _UNK_0002fe9c = fVar7;
    *pauVar13 = in_XMM1;
    pauVar13 = pauVar13 + 1;
    pauVar1 = pauVar1 + 1;
    fVar14 = _DAT_0002fe90;
    fVar15 = _UNK_0002fe94;
    fVar16 = _UNK_0002fe98;
    fVar7 = _UNK_0002fe9c;
    _UNK_0002fe9c = fVar5;
    _UNK_0002fe98 = fVar4;
    _UNK_0002fe94 = fVar3;
    _DAT_0002fe90 = fVar2;
  }
LAB_0002cafd:
  _UNK_0002fe9c = fVar7;
  _UNK_0002fe98 = fVar16;
  _UNK_0002fe94 = fVar15;
  _DAT_0002fe90 = fVar14;
  if (uVar8 != 0) {
    auVar17 = pshuflw(*(undefined1 (*) [16])(param_1[-1] + (ulong)param_3 * 4),
                      *(undefined1 (*) [16])(param_1[-1] + (ulong)param_3 * 4),0xb1);
    auVar17 = pshufhw(auVar17,auVar17,0xb1);
    auVar18._0_2_ = auVar17._0_2_ >> 8;
    auVar18._2_2_ = auVar17._2_2_ >> 8;
    auVar18._4_2_ = auVar17._4_2_ >> 8;
    auVar18._6_2_ = auVar17._6_2_ >> 8;
    auVar18._8_2_ = auVar17._8_2_ >> 8;
    auVar18._10_2_ = auVar17._10_2_ >> 8;
    auVar18._12_2_ = auVar17._12_2_ >> 8;
    auVar18._14_2_ = auVar17._14_2_ >> 8;
    auVar17 = psllw(auVar17,8);
    auVar17 = auVar17 | auVar18;
    fVar14 = (float)auVar17._4_4_ * _UNK_0002fe94;
    fVar15 = (float)auVar17._8_4_ * _UNK_0002fe98;
    fVar16 = (float)auVar17._12_4_ * _UNK_0002fe9c;
    pfVar12 = (float *)(param_2[-1] + (ulong)param_3 * 4);
    *pfVar12 = (float)auVar17._0_4_ * _DAT_0002fe90;
    pfVar12[1] = fVar14;
    pfVar12[2] = fVar15;
    pfVar12[3] = fVar16;
  }
  return;
}
