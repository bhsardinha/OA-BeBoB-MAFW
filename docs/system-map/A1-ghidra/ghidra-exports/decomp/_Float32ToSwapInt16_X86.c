// _Float32ToSwapInt16_X86
// Entry: 0002d69e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _Float32ToSwapInt16_X86(float *param_1,undefined1 (*param_2) [16],uint param_3)

{
  double dVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  double dVar10;
  double dVar11;
  ushort uVar12;
  uint uVar13;
  ulong uVar14;
  uint uVar15;
  uint uVar16;
  undefined1 (*pauVar17) [16];
  float *pfVar18;
  undefined1 auVar19 [16];
  undefined1 auVar20 [16];
  undefined1 auVar21 [16];
  undefined1 auVar22 [16];
  undefined1 auVar23 [16];
  undefined1 auVar24 [16];
  undefined1 auVar25 [16];
  undefined1 auVar26 [16];
  undefined1 auVar27 [16];
  undefined1 auVar28 [16];
  undefined1 auVar29 [16];
  undefined1 auVar30 [16];
  undefined1 auVar31 [16];
  undefined1 auVar32 [16];
  undefined1 auVar33 [16];
  undefined1 auVar34 [16];
  undefined1 auVar35 [16];
  undefined1 auVar36 [16];
  undefined1 auVar37 [16];
  undefined1 auVar38 [16];
  
  dVar11 = DAT_0002ffc8;
  dVar10 = DAT_0002ffc0;
  if (param_3 < 8) {
    if (param_3 == 0) {
      return;
    }
    uVar14 = (ulong)param_3;
    do {
      dVar1 = (double)*param_1 * dVar10 + dVar11;
      if (dVar1 < _DAT_0002ffd0) {
        uVar12 = (ushort)((uint)(int)dVar1 >> 0x10);
      }
      else {
        uVar12 = 0x7fff;
      }
      *(ushort *)*param_2 = uVar12 << 8 | uVar12 >> 8;
      param_2 = (undefined1 (*) [16])(*param_2 + 2);
      param_1 = param_1 + 1;
      uVar14 = uVar14 - 1;
    } while (uVar14 != 0);
    return;
  }
  uVar16 = (uint)param_2;
  pauVar17 = param_2;
  pfVar18 = param_1;
  uVar13 = param_3;
  if (((uVar16 | (uint)param_1) & 0xf) != 0) {
    auVar20._0_4_ = *param_1 * _DAT_0002ff80 + _DAT_0002ff90;
    auVar20._4_4_ = param_1[1] * _UNK_0002ff84 + _UNK_0002ff94;
    auVar20._8_4_ = param_1[2] * _UNK_0002ff88 + _UNK_0002ff98;
    auVar20._12_4_ = param_1[3] * _UNK_0002ff8c + _UNK_0002ff9c;
    auVar28._0_4_ = param_1[4] * _DAT_0002ff80 + _DAT_0002ff90;
    auVar28._4_4_ = param_1[5] * _UNK_0002ff84 + _UNK_0002ff94;
    auVar28._8_4_ = param_1[6] * _UNK_0002ff88 + _UNK_0002ff98;
    auVar28._12_4_ = param_1[7] * _UNK_0002ff8c + _UNK_0002ff9c;
    auVar20 = maxps(auVar20,_DAT_0002ffa0);
    auVar24 = maxps(auVar28,_DAT_0002ffa0);
    auVar20 = minps(auVar20,_DAT_0002ffb0);
    auVar28 = minps(auVar24,_DAT_0002ffb0);
    auVar24._0_4_ = (int)auVar20._0_4_;
    auVar24._4_4_ = (int)auVar20._4_4_;
    auVar24._8_4_ = (int)auVar20._8_4_;
    auVar24._12_4_ = (int)auVar20._12_4_;
    auVar33._0_4_ = (int)auVar28._0_4_;
    auVar33._4_4_ = (int)auVar28._4_4_;
    auVar33._8_4_ = (int)auVar28._8_4_;
    auVar33._12_4_ = (int)auVar28._12_4_;
    auVar20 = packssdw(auVar24,auVar33);
    auVar22._0_2_ = auVar20._0_2_ >> 8;
    auVar22._2_2_ = auVar20._2_2_ >> 8;
    auVar22._4_2_ = auVar20._4_2_ >> 8;
    auVar22._6_2_ = auVar20._6_2_ >> 8;
    auVar22._8_2_ = auVar20._8_2_ >> 8;
    auVar22._10_2_ = auVar20._10_2_ >> 8;
    auVar22._12_2_ = auVar20._12_2_ >> 8;
    auVar22._14_2_ = auVar20._14_2_ >> 8;
    auVar20 = psllw(auVar20,8);
    uVar15 = 0x10 - (uVar16 & 0xf) >> 1;
    uVar13 = param_3 - uVar15;
    *param_2 = auVar20 | auVar22;
    auVar24 = _DAT_0002ffb0;
    auVar20 = _DAT_0002ffa0;
    fVar9 = _UNK_0002ff9c;
    fVar8 = _UNK_0002ff98;
    fVar7 = _UNK_0002ff94;
    fVar6 = _DAT_0002ff90;
    fVar5 = _UNK_0002ff8c;
    fVar4 = _UNK_0002ff88;
    fVar3 = _UNK_0002ff84;
    fVar2 = _DAT_0002ff80;
    if (((ulong)(param_1 + uVar15) & 0xf) != 0) {
      if (7 < uVar13) {
        uVar16 = 0x10 - (uVar16 & 0xf) >> 1;
        pauVar17 = (undefined1 (*) [16])(*param_2 + (ulong)uVar16 * 2);
        uVar13 = param_3 - uVar16;
        pfVar18 = param_1 + (ulong)uVar16 + 4;
        do {
          auVar30._0_4_ = pfVar18[-4] * fVar2 + fVar6;
          auVar30._4_4_ = pfVar18[-3] * fVar3 + fVar7;
          auVar30._8_4_ = pfVar18[-2] * fVar4 + fVar8;
          auVar30._12_4_ = pfVar18[-1] * fVar5 + fVar9;
          auVar36._0_4_ = *pfVar18 * fVar2 + fVar6;
          auVar36._4_4_ = pfVar18[1] * fVar3 + fVar7;
          auVar36._8_4_ = pfVar18[2] * fVar4 + fVar8;
          auVar36._12_4_ = pfVar18[3] * fVar5 + fVar9;
          auVar28 = maxps(auVar30,auVar20);
          auVar33 = maxps(auVar36,auVar20);
          auVar28 = minps(auVar28,auVar24);
          auVar33 = minps(auVar33,auVar24);
          auVar31._0_4_ = (int)auVar28._0_4_;
          auVar31._4_4_ = (int)auVar28._4_4_;
          auVar31._8_4_ = (int)auVar28._8_4_;
          auVar31._12_4_ = (int)auVar28._12_4_;
          auVar37._0_4_ = (int)auVar33._0_4_;
          auVar37._4_4_ = (int)auVar33._4_4_;
          auVar37._8_4_ = (int)auVar33._8_4_;
          auVar37._12_4_ = (int)auVar33._12_4_;
          auVar28 = packssdw(auVar31,auVar37);
          auVar38._0_2_ = auVar28._0_2_ >> 8;
          auVar38._2_2_ = auVar28._2_2_ >> 8;
          auVar38._4_2_ = auVar28._4_2_ >> 8;
          auVar38._6_2_ = auVar28._6_2_ >> 8;
          auVar38._8_2_ = auVar28._8_2_ >> 8;
          auVar38._10_2_ = auVar28._10_2_ >> 8;
          auVar38._12_2_ = auVar28._12_2_ >> 8;
          auVar38._14_2_ = auVar28._14_2_ >> 8;
          auVar28 = psllw(auVar28,8);
          pfVar18 = pfVar18 + 8;
          *pauVar17 = auVar28 | auVar38;
          pauVar17 = pauVar17 + 1;
          uVar13 = uVar13 - 8;
        } while (7 < uVar13);
      }
      goto LAB_0002d880;
    }
    pauVar17 = (undefined1 (*) [16])(*param_2 + (ulong)uVar15 * 2);
    pfVar18 = param_1 + uVar15;
  }
  auVar24 = _DAT_0002ffb0;
  auVar20 = _DAT_0002ffa0;
  fVar9 = _UNK_0002ff9c;
  fVar8 = _UNK_0002ff98;
  fVar7 = _UNK_0002ff94;
  fVar6 = _DAT_0002ff90;
  fVar5 = _UNK_0002ff8c;
  fVar4 = _UNK_0002ff88;
  fVar3 = _UNK_0002ff84;
  fVar2 = _DAT_0002ff80;
  if (7 < uVar13) {
    pfVar18 = pfVar18 + 4;
    do {
      auVar27._0_4_ = pfVar18[-4] * fVar2 + fVar6;
      auVar27._4_4_ = pfVar18[-3] * fVar3 + fVar7;
      auVar27._8_4_ = pfVar18[-2] * fVar4 + fVar8;
      auVar27._12_4_ = pfVar18[-1] * fVar5 + fVar9;
      auVar32._0_4_ = *pfVar18 * fVar2 + fVar6;
      auVar32._4_4_ = pfVar18[1] * fVar3 + fVar7;
      auVar32._8_4_ = pfVar18[2] * fVar4 + fVar8;
      auVar32._12_4_ = pfVar18[3] * fVar5 + fVar9;
      auVar28 = maxps(auVar27,auVar20);
      auVar33 = maxps(auVar32,auVar20);
      auVar28 = minps(auVar28,auVar24);
      auVar33 = minps(auVar33,auVar24);
      auVar29._0_4_ = (int)auVar28._0_4_;
      auVar29._4_4_ = (int)auVar28._4_4_;
      auVar29._8_4_ = (int)auVar28._8_4_;
      auVar29._12_4_ = (int)auVar28._12_4_;
      auVar34._0_4_ = (int)auVar33._0_4_;
      auVar34._4_4_ = (int)auVar33._4_4_;
      auVar34._8_4_ = (int)auVar33._8_4_;
      auVar34._12_4_ = (int)auVar33._12_4_;
      auVar28 = packssdw(auVar29,auVar34);
      auVar35._0_2_ = auVar28._0_2_ >> 8;
      auVar35._2_2_ = auVar28._2_2_ >> 8;
      auVar35._4_2_ = auVar28._4_2_ >> 8;
      auVar35._6_2_ = auVar28._6_2_ >> 8;
      auVar35._8_2_ = auVar28._8_2_ >> 8;
      auVar35._10_2_ = auVar28._10_2_ >> 8;
      auVar35._12_2_ = auVar28._12_2_ >> 8;
      auVar35._14_2_ = auVar28._14_2_ >> 8;
      auVar28 = psllw(auVar28,8);
      pfVar18 = pfVar18 + 8;
      *pauVar17 = auVar28 | auVar35;
      pauVar17 = pauVar17 + 1;
      uVar13 = uVar13 - 8;
    } while (7 < uVar13);
  }
LAB_0002d880:
  if (uVar13 != 0) {
    uVar14 = (ulong)param_3;
    pfVar18 = param_1 + (uVar14 - 8);
    param_1 = param_1 + (uVar14 - 4);
    auVar19._0_4_ = *pfVar18 * _DAT_0002ff80 + _DAT_0002ff90;
    auVar19._4_4_ = pfVar18[1] * _UNK_0002ff84 + _UNK_0002ff94;
    auVar19._8_4_ = pfVar18[2] * _UNK_0002ff88 + _UNK_0002ff98;
    auVar19._12_4_ = pfVar18[3] * _UNK_0002ff8c + _UNK_0002ff9c;
    auVar23._0_4_ = *param_1 * _DAT_0002ff80 + _DAT_0002ff90;
    auVar23._4_4_ = param_1[1] * _UNK_0002ff84 + _UNK_0002ff94;
    auVar23._8_4_ = param_1[2] * _UNK_0002ff88 + _UNK_0002ff98;
    auVar23._12_4_ = param_1[3] * _UNK_0002ff8c + _UNK_0002ff9c;
    auVar20 = maxps(auVar19,_DAT_0002ffa0);
    auVar24 = maxps(auVar23,_DAT_0002ffa0);
    auVar20 = minps(auVar20,_DAT_0002ffb0);
    auVar24 = minps(auVar24,_DAT_0002ffb0);
    auVar21._0_4_ = (int)auVar20._0_4_;
    auVar21._4_4_ = (int)auVar20._4_4_;
    auVar21._8_4_ = (int)auVar20._8_4_;
    auVar21._12_4_ = (int)auVar20._12_4_;
    auVar25._0_4_ = (int)auVar24._0_4_;
    auVar25._4_4_ = (int)auVar24._4_4_;
    auVar25._8_4_ = (int)auVar24._8_4_;
    auVar25._12_4_ = (int)auVar24._12_4_;
    auVar20 = packssdw(auVar21,auVar25);
    auVar26._0_2_ = auVar20._0_2_ >> 8;
    auVar26._2_2_ = auVar20._2_2_ >> 8;
    auVar26._4_2_ = auVar20._4_2_ >> 8;
    auVar26._6_2_ = auVar20._6_2_ >> 8;
    auVar26._8_2_ = auVar20._8_2_ >> 8;
    auVar26._10_2_ = auVar20._10_2_ >> 8;
    auVar26._12_2_ = auVar20._12_2_ >> 8;
    auVar26._14_2_ = auVar20._14_2_ >> 8;
    auVar20 = psllw(auVar20,8);
    *(undefined1 (*) [16])(param_2[-1] + uVar14 * 2) = auVar20 | auVar26;
  }
  return;
}
