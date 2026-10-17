// _Float32ToNativeInt16_X86
// Entry: 0002c66e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _Float32ToNativeInt16_X86(float *param_1,undefined1 (*param_2) [16],uint param_3)

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
  undefined2 uVar12;
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
        uVar12 = (undefined2)((uint)(int)dVar1 >> 0x10);
      }
      else {
        uVar12 = 0x7fff;
      }
      *(undefined2 *)*param_2 = uVar12;
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
    auVar20._0_4_ = *param_1 * _DAT_0002fe40 + _DAT_0002fe50;
    auVar20._4_4_ = param_1[1] * _UNK_0002fe44 + _UNK_0002fe54;
    auVar20._8_4_ = param_1[2] * _UNK_0002fe48 + _UNK_0002fe58;
    auVar20._12_4_ = param_1[3] * _UNK_0002fe4c + _UNK_0002fe5c;
    auVar26._0_4_ = param_1[4] * _DAT_0002fe40 + _DAT_0002fe50;
    auVar26._4_4_ = param_1[5] * _UNK_0002fe44 + _UNK_0002fe54;
    auVar26._8_4_ = param_1[6] * _UNK_0002fe48 + _UNK_0002fe58;
    auVar26._12_4_ = param_1[7] * _UNK_0002fe4c + _UNK_0002fe5c;
    auVar20 = maxps(auVar20,_DAT_0002fe60);
    auVar23 = maxps(auVar26,_DAT_0002fe60);
    auVar20 = minps(auVar20,_DAT_0002fe70);
    auVar26 = minps(auVar23,_DAT_0002fe70);
    auVar23._0_4_ = (int)auVar20._0_4_;
    auVar23._4_4_ = (int)auVar20._4_4_;
    auVar23._8_4_ = (int)auVar20._8_4_;
    auVar23._12_4_ = (int)auVar20._12_4_;
    auVar31._0_4_ = (int)auVar26._0_4_;
    auVar31._4_4_ = (int)auVar26._4_4_;
    auVar31._8_4_ = (int)auVar26._8_4_;
    auVar31._12_4_ = (int)auVar26._12_4_;
    auVar20 = packssdw(auVar23,auVar31);
    uVar15 = 0x10 - (uVar16 & 0xf) >> 1;
    uVar13 = param_3 - uVar15;
    *param_2 = auVar20;
    auVar23 = _DAT_0002fe70;
    auVar20 = _DAT_0002fe60;
    fVar9 = _UNK_0002fe5c;
    fVar8 = _UNK_0002fe58;
    fVar7 = _UNK_0002fe54;
    fVar6 = _DAT_0002fe50;
    fVar5 = _UNK_0002fe4c;
    fVar4 = _UNK_0002fe48;
    fVar3 = _UNK_0002fe44;
    fVar2 = _DAT_0002fe40;
    if (((ulong)(param_1 + uVar15) & 0xf) != 0) {
      if (7 < uVar13) {
        uVar16 = 0x10 - (uVar16 & 0xf) >> 1;
        pauVar17 = (undefined1 (*) [16])(*param_2 + (ulong)uVar16 * 2);
        uVar13 = param_3 - uVar16;
        pfVar18 = param_1 + (ulong)uVar16 + 4;
        do {
          auVar28._0_4_ = pfVar18[-4] * fVar2 + fVar6;
          auVar28._4_4_ = pfVar18[-3] * fVar3 + fVar7;
          auVar28._8_4_ = pfVar18[-2] * fVar4 + fVar8;
          auVar28._12_4_ = pfVar18[-1] * fVar5 + fVar9;
          auVar33._0_4_ = *pfVar18 * fVar2 + fVar6;
          auVar33._4_4_ = pfVar18[1] * fVar3 + fVar7;
          auVar33._8_4_ = pfVar18[2] * fVar4 + fVar8;
          auVar33._12_4_ = pfVar18[3] * fVar5 + fVar9;
          auVar26 = maxps(auVar28,auVar20);
          auVar31 = maxps(auVar33,auVar20);
          auVar26 = minps(auVar26,auVar23);
          auVar31 = minps(auVar31,auVar23);
          auVar29._0_4_ = (int)auVar26._0_4_;
          auVar29._4_4_ = (int)auVar26._4_4_;
          auVar29._8_4_ = (int)auVar26._8_4_;
          auVar29._12_4_ = (int)auVar26._12_4_;
          auVar34._0_4_ = (int)auVar31._0_4_;
          auVar34._4_4_ = (int)auVar31._4_4_;
          auVar34._8_4_ = (int)auVar31._8_4_;
          auVar34._12_4_ = (int)auVar31._12_4_;
          auVar26 = packssdw(auVar29,auVar34);
          pfVar18 = pfVar18 + 8;
          *pauVar17 = auVar26;
          pauVar17 = pauVar17 + 1;
          uVar13 = uVar13 - 8;
        } while (7 < uVar13);
      }
      goto LAB_0002c80f;
    }
    pauVar17 = (undefined1 (*) [16])(*param_2 + (ulong)uVar15 * 2);
    pfVar18 = param_1 + uVar15;
  }
  auVar23 = _DAT_0002fe70;
  auVar20 = _DAT_0002fe60;
  fVar9 = _UNK_0002fe5c;
  fVar8 = _UNK_0002fe58;
  fVar7 = _UNK_0002fe54;
  fVar6 = _DAT_0002fe50;
  fVar5 = _UNK_0002fe4c;
  fVar4 = _UNK_0002fe48;
  fVar3 = _UNK_0002fe44;
  fVar2 = _DAT_0002fe40;
  if (7 < uVar13) {
    pfVar18 = pfVar18 + 4;
    do {
      auVar25._0_4_ = pfVar18[-4] * fVar2 + fVar6;
      auVar25._4_4_ = pfVar18[-3] * fVar3 + fVar7;
      auVar25._8_4_ = pfVar18[-2] * fVar4 + fVar8;
      auVar25._12_4_ = pfVar18[-1] * fVar5 + fVar9;
      auVar30._0_4_ = *pfVar18 * fVar2 + fVar6;
      auVar30._4_4_ = pfVar18[1] * fVar3 + fVar7;
      auVar30._8_4_ = pfVar18[2] * fVar4 + fVar8;
      auVar30._12_4_ = pfVar18[3] * fVar5 + fVar9;
      auVar26 = maxps(auVar25,auVar20);
      auVar31 = maxps(auVar30,auVar20);
      auVar26 = minps(auVar26,auVar23);
      auVar31 = minps(auVar31,auVar23);
      auVar27._0_4_ = (int)auVar26._0_4_;
      auVar27._4_4_ = (int)auVar26._4_4_;
      auVar27._8_4_ = (int)auVar26._8_4_;
      auVar27._12_4_ = (int)auVar26._12_4_;
      auVar32._0_4_ = (int)auVar31._0_4_;
      auVar32._4_4_ = (int)auVar31._4_4_;
      auVar32._8_4_ = (int)auVar31._8_4_;
      auVar32._12_4_ = (int)auVar31._12_4_;
      auVar26 = packssdw(auVar27,auVar32);
      pfVar18 = pfVar18 + 8;
      *pauVar17 = auVar26;
      pauVar17 = pauVar17 + 1;
      uVar13 = uVar13 - 8;
    } while (7 < uVar13);
  }
LAB_0002c80f:
  if (uVar13 != 0) {
    uVar14 = (ulong)param_3;
    pfVar18 = param_1 + (uVar14 - 8);
    param_1 = param_1 + (uVar14 - 4);
    auVar19._0_4_ = *pfVar18 * _DAT_0002fe40 + _DAT_0002fe50;
    auVar19._4_4_ = pfVar18[1] * _UNK_0002fe44 + _UNK_0002fe54;
    auVar19._8_4_ = pfVar18[2] * _UNK_0002fe48 + _UNK_0002fe58;
    auVar19._12_4_ = pfVar18[3] * _UNK_0002fe4c + _UNK_0002fe5c;
    auVar22._0_4_ = *param_1 * _DAT_0002fe40 + _DAT_0002fe50;
    auVar22._4_4_ = param_1[1] * _UNK_0002fe44 + _UNK_0002fe54;
    auVar22._8_4_ = param_1[2] * _UNK_0002fe48 + _UNK_0002fe58;
    auVar22._12_4_ = param_1[3] * _UNK_0002fe4c + _UNK_0002fe5c;
    auVar20 = maxps(auVar19,_DAT_0002fe60);
    auVar23 = maxps(auVar22,_DAT_0002fe60);
    auVar20 = minps(auVar20,_DAT_0002fe70);
    auVar23 = minps(auVar23,_DAT_0002fe70);
    auVar21._0_4_ = (int)auVar20._0_4_;
    auVar21._4_4_ = (int)auVar20._4_4_;
    auVar21._8_4_ = (int)auVar20._8_4_;
    auVar21._12_4_ = (int)auVar20._12_4_;
    auVar24._0_4_ = (int)auVar23._0_4_;
    auVar24._4_4_ = (int)auVar23._4_4_;
    auVar24._8_4_ = (int)auVar23._8_4_;
    auVar24._12_4_ = (int)auVar23._12_4_;
    auVar20 = packssdw(auVar21,auVar24);
    *(undefined1 (*) [16])(param_2[-1] + uVar14 * 2) = auVar20;
  }
  return;
}
