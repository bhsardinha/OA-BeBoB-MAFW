// _Float32ToSwapInt32_X86
// Entry: 0002ced6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _Float32ToSwapInt32_X86(float *param_1,undefined1 (*param_2) [16],uint param_3)

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
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  uint uVar20;
  ulong uVar21;
  uint uVar22;
  uint uVar23;
  undefined1 (*pauVar24) [16];
  float *pfVar25;
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
  
  dVar11 = DAT_0002fff0;
  dVar10 = DAT_0002ffe8;
  if (param_3 < 4) {
    if (param_3 == 0) {
      return;
    }
    uVar21 = (ulong)param_3;
    do {
      dVar1 = (double)*param_1 * dVar10 + dVar11;
      if (dVar1 < _DAT_0002fff8) {
        uVar20 = (uint)dVar1;
      }
      else {
        uVar20 = 0x7fffffff;
      }
      *(uint *)*param_2 =
           uVar20 >> 0x18 | (uVar20 & 0xff0000) >> 8 | (uVar20 & 0xff00) << 8 | uVar20 << 0x18;
      param_2 = (undefined1 (*) [16])(*param_2 + 4);
      param_1 = param_1 + 1;
      uVar21 = uVar21 - 1;
    } while (uVar21 != 0);
    return;
  }
  uVar22 = (uint)param_2;
  pauVar24 = param_2;
  pfVar25 = param_1;
  uVar20 = param_3;
  fVar12 = _DAT_0002fec0;
  fVar13 = _UNK_0002fec4;
  fVar14 = _UNK_0002fec8;
  fVar15 = _UNK_0002fecc;
  fVar16 = _DAT_0002fed0;
  fVar17 = _UNK_0002fed4;
  fVar18 = _UNK_0002fed8;
  fVar19 = _UNK_0002fedc;
  auVar28 = _DAT_0002fee0;
  auVar26 = _DAT_0002fef0;
  if (((uVar22 | (uint)param_1) & 0xf) != 0) {
    auVar28._0_4_ = *param_1 * _DAT_0002fec0 + _DAT_0002fed0;
    auVar28._4_4_ = param_1[1] * _UNK_0002fec4 + _UNK_0002fed4;
    auVar28._8_4_ = param_1[2] * _UNK_0002fec8 + _UNK_0002fed8;
    auVar28._12_4_ = param_1[3] * _UNK_0002fecc + _UNK_0002fedc;
    auVar28 = maxps(auVar28,_DAT_0002fee0);
    auVar28 = minps(auVar28,_DAT_0002fef0);
    auVar26._0_4_ = (int)auVar28._0_4_;
    auVar26._4_4_ = (int)auVar28._4_4_;
    auVar26._8_4_ = (int)auVar28._8_4_;
    auVar26._12_4_ = (int)auVar28._12_4_;
    auVar28 = pshuflw(auVar26,auVar26,0xb1);
    auVar28 = pshufhw(auVar28,auVar28,0xb1);
    auVar30._0_2_ = auVar28._0_2_ >> 8;
    auVar30._2_2_ = auVar28._2_2_ >> 8;
    auVar30._4_2_ = auVar28._4_2_ >> 8;
    auVar30._6_2_ = auVar28._6_2_ >> 8;
    auVar30._8_2_ = auVar28._8_2_ >> 8;
    auVar30._10_2_ = auVar28._10_2_ >> 8;
    auVar30._12_2_ = auVar28._12_2_ >> 8;
    auVar30._14_2_ = auVar28._14_2_ >> 8;
    auVar28 = psllw(auVar28,8);
    uVar23 = 0x10 - (uVar22 & 0xf) >> 2;
    uVar20 = param_3 - uVar23;
    *param_2 = auVar28 | auVar30;
    auVar27 = _DAT_0002fef0;
    auVar30 = _DAT_0002fee0;
    fVar9 = _UNK_0002fedc;
    fVar8 = _UNK_0002fed8;
    fVar7 = _UNK_0002fed4;
    fVar6 = _DAT_0002fed0;
    fVar5 = _UNK_0002fecc;
    fVar4 = _UNK_0002fec8;
    fVar3 = _UNK_0002fec4;
    fVar2 = _DAT_0002fec0;
    fVar12 = _DAT_0002fec0;
    fVar13 = _UNK_0002fec4;
    fVar14 = _UNK_0002fec8;
    fVar15 = _UNK_0002fecc;
    fVar16 = _DAT_0002fed0;
    fVar17 = _UNK_0002fed4;
    fVar18 = _UNK_0002fed8;
    fVar19 = _UNK_0002fedc;
    auVar28 = _DAT_0002fee0;
    auVar26 = _DAT_0002fef0;
    if (((ulong)(param_1 + uVar23) & 0xf) != 0) {
      if (3 < uVar20) {
        uVar20 = 0x10 - (uVar22 & 0xf) >> 2;
        pauVar24 = (undefined1 (*) [16])(*param_2 + (ulong)uVar20 * 4);
        pfVar25 = param_1 + uVar20;
        uVar20 = param_3 - uVar20;
        do {
          auVar33._0_4_ = *pfVar25 * fVar2 + fVar6;
          auVar33._4_4_ = pfVar25[1] * fVar3 + fVar7;
          auVar33._8_4_ = pfVar25[2] * fVar4 + fVar8;
          auVar33._12_4_ = pfVar25[3] * fVar5 + fVar9;
          auVar28 = maxps(auVar33,auVar30);
          auVar28 = minps(auVar28,auVar27);
          auVar35._0_4_ = (int)auVar28._0_4_;
          auVar35._4_4_ = (int)auVar28._4_4_;
          auVar35._8_4_ = (int)auVar28._8_4_;
          auVar35._12_4_ = (int)auVar28._12_4_;
          auVar28 = pshuflw(auVar35,auVar35,0xb1);
          auVar28 = pshufhw(auVar28,auVar28,0xb1);
          auVar37._0_2_ = auVar28._0_2_ >> 8;
          auVar37._2_2_ = auVar28._2_2_ >> 8;
          auVar37._4_2_ = auVar28._4_2_ >> 8;
          auVar37._6_2_ = auVar28._6_2_ >> 8;
          auVar37._8_2_ = auVar28._8_2_ >> 8;
          auVar37._10_2_ = auVar28._10_2_ >> 8;
          auVar37._12_2_ = auVar28._12_2_ >> 8;
          auVar37._14_2_ = auVar28._14_2_ >> 8;
          auVar28 = psllw(auVar28,8);
          *pauVar24 = auVar28 | auVar37;
          pauVar24 = pauVar24 + 1;
          pfVar25 = pfVar25 + 4;
          uVar20 = uVar20 - 4;
          fVar12 = _DAT_0002fec0;
          fVar13 = _UNK_0002fec4;
          fVar14 = _UNK_0002fec8;
          fVar15 = _UNK_0002fecc;
          fVar16 = _DAT_0002fed0;
          fVar17 = _UNK_0002fed4;
          fVar18 = _UNK_0002fed8;
          fVar19 = _UNK_0002fedc;
          auVar28 = _DAT_0002fee0;
          auVar26 = _DAT_0002fef0;
        } while (3 < uVar20);
      }
      goto LAB_0002d076;
    }
    pauVar24 = (undefined1 (*) [16])(*param_2 + (ulong)uVar23 * 4);
    pfVar25 = param_1 + uVar23;
  }
  for (; auVar27 = _DAT_0002fef0, auVar30 = _DAT_0002fee0, fVar9 = _UNK_0002fedc,
      fVar8 = _UNK_0002fed8, fVar7 = _UNK_0002fed4, fVar6 = _DAT_0002fed0, fVar5 = _UNK_0002fecc,
      fVar4 = _UNK_0002fec8, fVar3 = _UNK_0002fec4, fVar2 = _DAT_0002fec0, 3 < uVar20;
      uVar20 = uVar20 - 4) {
    auVar32._0_4_ = *pfVar25 * _DAT_0002fec0 + _DAT_0002fed0;
    auVar32._4_4_ = pfVar25[1] * _UNK_0002fec4 + _UNK_0002fed4;
    auVar32._8_4_ = pfVar25[2] * _UNK_0002fec8 + _UNK_0002fed8;
    auVar32._12_4_ = pfVar25[3] * _UNK_0002fecc + _UNK_0002fedc;
    auVar33 = maxps(auVar32,_DAT_0002fee0);
    auVar33 = minps(auVar33,_DAT_0002fef0);
    auVar34._0_4_ = (int)auVar33._0_4_;
    auVar34._4_4_ = (int)auVar33._4_4_;
    auVar34._8_4_ = (int)auVar33._8_4_;
    auVar34._12_4_ = (int)auVar33._12_4_;
    auVar33 = pshuflw(auVar34,auVar34,0xb1);
    auVar33 = pshufhw(auVar33,auVar33,0xb1);
    auVar36._0_2_ = auVar33._0_2_ >> 8;
    auVar36._2_2_ = auVar33._2_2_ >> 8;
    auVar36._4_2_ = auVar33._4_2_ >> 8;
    auVar36._6_2_ = auVar33._6_2_ >> 8;
    auVar36._8_2_ = auVar33._8_2_ >> 8;
    auVar36._10_2_ = auVar33._10_2_ >> 8;
    auVar36._12_2_ = auVar33._12_2_ >> 8;
    auVar36._14_2_ = auVar33._14_2_ >> 8;
    auVar33 = psllw(auVar33,8);
    _DAT_0002fec0 = fVar12;
    _UNK_0002fec4 = fVar13;
    _UNK_0002fec8 = fVar14;
    _UNK_0002fecc = fVar15;
    _DAT_0002fed0 = fVar16;
    _UNK_0002fed4 = fVar17;
    _UNK_0002fed8 = fVar18;
    _UNK_0002fedc = fVar19;
    _DAT_0002fee0 = auVar28;
    _DAT_0002fef0 = auVar26;
    *pauVar24 = auVar33 | auVar36;
    pauVar24 = pauVar24 + 1;
    pfVar25 = pfVar25 + 4;
    fVar12 = _DAT_0002fec0;
    fVar13 = _UNK_0002fec4;
    fVar14 = _UNK_0002fec8;
    fVar15 = _UNK_0002fecc;
    fVar16 = _DAT_0002fed0;
    fVar17 = _UNK_0002fed4;
    fVar18 = _UNK_0002fed8;
    fVar19 = _UNK_0002fedc;
    auVar28 = _DAT_0002fee0;
    auVar26 = _DAT_0002fef0;
    _DAT_0002fef0 = auVar27;
    _DAT_0002fee0 = auVar30;
    _UNK_0002fedc = fVar9;
    _UNK_0002fed8 = fVar8;
    _UNK_0002fed4 = fVar7;
    _DAT_0002fed0 = fVar6;
    _UNK_0002fecc = fVar5;
    _UNK_0002fec8 = fVar4;
    _UNK_0002fec4 = fVar3;
    _DAT_0002fec0 = fVar2;
  }
LAB_0002d076:
  _DAT_0002fef0 = auVar26;
  _DAT_0002fee0 = auVar28;
  _UNK_0002fedc = fVar19;
  _UNK_0002fed8 = fVar18;
  _UNK_0002fed4 = fVar17;
  _DAT_0002fed0 = fVar16;
  _UNK_0002fecc = fVar15;
  _UNK_0002fec8 = fVar14;
  _UNK_0002fec4 = fVar13;
  _DAT_0002fec0 = fVar12;
  if (uVar20 != 0) {
    param_1 = param_1 + ((ulong)param_3 - 4);
    auVar27._0_4_ = *param_1 * _DAT_0002fec0 + _DAT_0002fed0;
    auVar27._4_4_ = param_1[1] * _UNK_0002fec4 + _UNK_0002fed4;
    auVar27._8_4_ = param_1[2] * _UNK_0002fec8 + _UNK_0002fed8;
    auVar27._12_4_ = param_1[3] * _UNK_0002fecc + _UNK_0002fedc;
    auVar28 = maxps(auVar27,_DAT_0002fee0);
    auVar28 = minps(auVar28,_DAT_0002fef0);
    auVar29._0_4_ = (int)auVar28._0_4_;
    auVar29._4_4_ = (int)auVar28._4_4_;
    auVar29._8_4_ = (int)auVar28._8_4_;
    auVar29._12_4_ = (int)auVar28._12_4_;
    auVar28 = pshuflw(auVar29,auVar29,0xb1);
    auVar28 = pshufhw(auVar28,auVar28,0xb1);
    auVar31._0_2_ = auVar28._0_2_ >> 8;
    auVar31._2_2_ = auVar28._2_2_ >> 8;
    auVar31._4_2_ = auVar28._4_2_ >> 8;
    auVar31._6_2_ = auVar28._6_2_ >> 8;
    auVar31._8_2_ = auVar28._8_2_ >> 8;
    auVar31._10_2_ = auVar28._10_2_ >> 8;
    auVar31._12_2_ = auVar28._12_2_ >> 8;
    auVar31._14_2_ = auVar28._14_2_ >> 8;
    auVar28 = psllw(auVar28,8);
    *(undefined1 (*) [16])(param_2[-1] + (ulong)param_3 * 4) = auVar28 | auVar31;
  }
  return;
}
