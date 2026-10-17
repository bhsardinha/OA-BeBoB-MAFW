// _SwapInt16ToFloat32_X86
// Entry: 0002cb67
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _SwapInt16ToFloat32_X86(undefined1 (*param_1) [16],float *param_2,uint param_3)

{
  int iVar1;
  undefined6 uVar2;
  unkbyte10 Var3;
  undefined1 auVar4 [14];
  undefined1 auVar5 [14];
  undefined1 auVar6 [14];
  double dVar7;
  uint uVar8;
  ulong uVar9;
  float *pfVar10;
  uint uVar11;
  undefined1 (*pauVar12) [16];
  ulong uVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar27;
  float fVar28;
  undefined1 auVar18 [16];
  undefined1 auVar19 [16];
  undefined1 auVar22 [16];
  undefined1 auVar23 [16];
  undefined1 auVar24 [16];
  float fVar29;
  undefined1 auVar38 [14];
  undefined1 auVar30 [16];
  undefined1 auVar31 [16];
  undefined1 auVar34 [16];
  undefined1 auVar35 [16];
  undefined1 auVar20 [16];
  undefined1 auVar25 [16];
  undefined1 auVar21 [16];
  undefined1 auVar26 [16];
  undefined1 auVar32 [16];
  undefined1 auVar36 [16];
  undefined1 auVar33 [16];
  undefined1 auVar37 [16];
  
  dVar7 = DAT_0002ffe0;
  if (param_3 < 8) {
    if (param_3 == 0) {
      return;
    }
    uVar9 = (ulong)param_3;
    do {
      *param_2 = (float)((double)(int)(short)(*(ushort *)*param_1 << 8 | *(ushort *)*param_1 >> 8) *
                        dVar7);
      param_2 = param_2 + 1;
      param_1 = (undefined1 (*) [16])(*param_1 + 2);
      uVar9 = uVar9 - 1;
    } while (uVar9 != 0);
    return;
  }
  uVar11 = (uint)param_2;
  pfVar10 = param_2;
  pauVar12 = param_1;
  uVar8 = param_3;
  if (((uVar11 | (uint)param_1) & 0xf) != 0) {
    auVar22 = *param_1;
    auVar18._0_2_ = auVar22._0_2_ >> 8;
    auVar18._2_2_ = auVar22._2_2_ >> 8;
    auVar18._4_2_ = auVar22._4_2_ >> 8;
    auVar18._6_2_ = auVar22._6_2_ >> 8;
    auVar18._8_2_ = auVar22._8_2_ >> 8;
    auVar18._10_2_ = auVar22._10_2_ >> 8;
    auVar18._12_2_ = auVar22._12_2_ >> 8;
    auVar18._14_2_ = auVar22._14_2_ >> 8;
    auVar22 = psllw(auVar22,8);
    auVar22 = auVar22 | auVar18;
    auVar21._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
    auVar21._12_2_ = 0;
    auVar21._14_2_ = auVar22._6_2_;
    auVar20._12_4_ = auVar21._12_4_;
    auVar20._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
    auVar20._10_2_ = auVar22._4_2_;
    auVar19._10_6_ = auVar20._10_6_;
    auVar19._0_10_ = (unkuint10)0 << 0x40;
    auVar38 = SUB1614((undefined1  [16])0x0,2);
    iVar1 = CONCAT22(auVar38._8_2_,auVar22._8_2_);
    uVar2 = CONCAT24(auVar22._10_2_,iVar1);
    Var3 = CONCAT28(auVar22._12_2_,CONCAT26(auVar38._10_2_,uVar2));
    auVar38._10_2_ = auVar38._12_2_;
    auVar38._0_10_ = Var3;
    auVar38._12_2_ = auVar22._14_2_;
    fVar14 = (float)((int)CONCAT82(auVar19._8_8_,auVar22._2_2_) << 0x10) * _UNK_0002fea4;
    fVar15 = (float)auVar19._8_4_ * _UNK_0002fea8;
    fVar16 = (float)auVar20._12_4_ * _UNK_0002feac;
    fVar17 = (float)(iVar1 << 0x10) * _DAT_0002fea0;
    fVar27 = (float)(int)((uint6)uVar2 >> 0x10) * _UNK_0002fea4;
    fVar28 = (float)(int)((unkuint10)Var3 >> 0x30) * _UNK_0002fea8;
    fVar29 = (float)auVar38._10_4_ * _UNK_0002feac;
    uVar8 = 0x10 - (uVar11 & 0xf) >> 2;
    uVar9 = (ulong)uVar8;
    uVar8 = param_3 - uVar8;
    *param_2 = (float)(int)((uint)auVar22._0_2_ << 0x10) * _DAT_0002fea0;
    param_2[1] = fVar14;
    param_2[2] = fVar15;
    param_2[3] = fVar16;
    param_2[4] = fVar17;
    param_2[5] = fVar27;
    param_2[6] = fVar28;
    param_2[7] = fVar29;
    fVar17 = _UNK_0002feac;
    fVar16 = _UNK_0002fea8;
    fVar15 = _UNK_0002fea4;
    fVar14 = _DAT_0002fea0;
    if (((ulong)(*param_1 + uVar9 * 2) & 0xf) != 0) {
      if (7 < uVar8) {
        uVar11 = 0x10 - (uVar11 & 0xf) >> 2;
        uVar8 = param_3 - uVar11;
        uVar13 = 0;
        do {
          auVar22 = *(undefined1 (*) [16])(*param_1 + (uVar9 | uVar13) * 2);
          auVar34._0_2_ = auVar22._0_2_ >> 8;
          auVar34._2_2_ = auVar22._2_2_ >> 8;
          auVar34._4_2_ = auVar22._4_2_ >> 8;
          auVar34._6_2_ = auVar22._6_2_ >> 8;
          auVar34._8_2_ = auVar22._8_2_ >> 8;
          auVar34._10_2_ = auVar22._10_2_ >> 8;
          auVar34._12_2_ = auVar22._12_2_ >> 8;
          auVar34._14_2_ = auVar22._14_2_ >> 8;
          auVar22 = psllw(auVar22,8);
          auVar22 = auVar22 | auVar34;
          auVar37._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
          auVar37._12_2_ = 0;
          auVar37._14_2_ = auVar22._6_2_;
          auVar36._12_4_ = auVar37._12_4_;
          auVar36._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
          auVar36._10_2_ = auVar22._4_2_;
          auVar35._10_6_ = auVar36._10_6_;
          auVar35._0_10_ = (unkuint10)0 << 0x40;
          auVar38 = SUB1614((undefined1  [16])0x0,2);
          iVar1 = CONCAT22(auVar38._8_2_,auVar22._8_2_);
          uVar2 = CONCAT24(auVar22._10_2_,iVar1);
          Var3 = CONCAT28(auVar22._12_2_,CONCAT26(auVar38._10_2_,uVar2));
          auVar5._10_2_ = auVar38._12_2_;
          auVar5._0_10_ = Var3;
          auVar5._12_2_ = auVar22._14_2_;
          pfVar10 = param_2 + (uVar9 | uVar13);
          *pfVar10 = (float)(int)((uint)auVar22._0_2_ << 0x10) * fVar14;
          pfVar10[1] = (float)((int)CONCAT82(auVar35._8_8_,auVar22._2_2_) << 0x10) * fVar15;
          pfVar10[2] = (float)auVar35._8_4_ * fVar16;
          pfVar10[3] = (float)auVar36._12_4_ * fVar17;
          pfVar10 = param_2 + (ulong)uVar11 + uVar13 + 4;
          *pfVar10 = (float)(iVar1 << 0x10) * fVar14;
          pfVar10[1] = (float)(int)((uint6)uVar2 >> 0x10) * fVar15;
          pfVar10[2] = (float)(int)((unkuint10)Var3 >> 0x30) * fVar16;
          pfVar10[3] = (float)auVar5._10_4_ * fVar17;
          uVar13 = uVar13 + 8;
          uVar8 = uVar8 - 8;
        } while (7 < uVar8);
      }
      goto LAB_0002ccc2;
    }
    pfVar10 = param_2 + uVar9;
    pauVar12 = (undefined1 (*) [16])(*param_1 + uVar9 * 2);
  }
  fVar17 = _UNK_0002feac;
  fVar16 = _UNK_0002fea8;
  fVar15 = _UNK_0002fea4;
  fVar14 = _DAT_0002fea0;
  if (7 < uVar8) {
    pfVar10 = pfVar10 + 4;
    do {
      auVar22 = *pauVar12;
      auVar30._0_2_ = auVar22._0_2_ >> 8;
      auVar30._2_2_ = auVar22._2_2_ >> 8;
      auVar30._4_2_ = auVar22._4_2_ >> 8;
      auVar30._6_2_ = auVar22._6_2_ >> 8;
      auVar30._8_2_ = auVar22._8_2_ >> 8;
      auVar30._10_2_ = auVar22._10_2_ >> 8;
      auVar30._12_2_ = auVar22._12_2_ >> 8;
      auVar30._14_2_ = auVar22._14_2_ >> 8;
      auVar22 = psllw(auVar22,8);
      auVar22 = auVar22 | auVar30;
      auVar33._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
      auVar33._12_2_ = 0;
      auVar33._14_2_ = auVar22._6_2_;
      auVar32._12_4_ = auVar33._12_4_;
      auVar32._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
      auVar32._10_2_ = auVar22._4_2_;
      auVar31._10_6_ = auVar32._10_6_;
      auVar31._0_10_ = (unkuint10)0 << 0x40;
      auVar38 = SUB1614((undefined1  [16])0x0,2);
      iVar1 = CONCAT22(auVar38._8_2_,auVar22._8_2_);
      uVar2 = CONCAT24(auVar22._10_2_,iVar1);
      Var3 = CONCAT28(auVar22._12_2_,CONCAT26(auVar38._10_2_,uVar2));
      auVar4._10_2_ = auVar38._12_2_;
      auVar4._0_10_ = Var3;
      auVar4._12_2_ = auVar22._14_2_;
      pfVar10[-4] = (float)(int)((uint)auVar22._0_2_ << 0x10) * fVar14;
      pfVar10[-3] = (float)((int)CONCAT82(auVar31._8_8_,auVar22._2_2_) << 0x10) * fVar15;
      pfVar10[-2] = (float)auVar31._8_4_ * fVar16;
      pfVar10[-1] = (float)auVar32._12_4_ * fVar17;
      *pfVar10 = (float)(iVar1 << 0x10) * fVar14;
      pfVar10[1] = (float)(int)((uint6)uVar2 >> 0x10) * fVar15;
      pfVar10[2] = (float)(int)((unkuint10)Var3 >> 0x30) * fVar16;
      pfVar10[3] = (float)auVar4._10_4_ * fVar17;
      pfVar10 = pfVar10 + 8;
      pauVar12 = pauVar12 + 1;
      uVar8 = uVar8 - 8;
    } while (7 < uVar8);
  }
LAB_0002ccc2:
  if (uVar8 != 0) {
    uVar9 = (ulong)param_3;
    auVar22 = *(undefined1 (*) [16])(param_1[-1] + uVar9 * 2);
    auVar23._0_2_ = auVar22._0_2_ >> 8;
    auVar23._2_2_ = auVar22._2_2_ >> 8;
    auVar23._4_2_ = auVar22._4_2_ >> 8;
    auVar23._6_2_ = auVar22._6_2_ >> 8;
    auVar23._8_2_ = auVar22._8_2_ >> 8;
    auVar23._10_2_ = auVar22._10_2_ >> 8;
    auVar23._12_2_ = auVar22._12_2_ >> 8;
    auVar23._14_2_ = auVar22._14_2_ >> 8;
    auVar22 = psllw(auVar22,8);
    auVar22 = auVar22 | auVar23;
    auVar26._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
    auVar26._12_2_ = 0;
    auVar26._14_2_ = auVar22._6_2_;
    auVar25._12_4_ = auVar26._12_4_;
    auVar25._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
    auVar25._10_2_ = auVar22._4_2_;
    auVar24._10_6_ = auVar25._10_6_;
    auVar24._0_10_ = (unkuint10)0 << 0x40;
    auVar38 = SUB1614((undefined1  [16])0x0,2);
    iVar1 = CONCAT22(auVar38._8_2_,auVar22._8_2_);
    uVar2 = CONCAT24(auVar22._10_2_,iVar1);
    Var3 = CONCAT28(auVar22._12_2_,CONCAT26(auVar38._10_2_,uVar2));
    auVar6._10_2_ = auVar38._12_2_;
    auVar6._0_10_ = Var3;
    auVar6._12_2_ = auVar22._14_2_;
    fVar14 = (float)((int)CONCAT82(auVar24._8_8_,auVar22._2_2_) << 0x10) * _UNK_0002fea4;
    fVar15 = (float)auVar24._8_4_ * _UNK_0002fea8;
    fVar16 = (float)auVar25._12_4_ * _UNK_0002feac;
    fVar17 = (float)(iVar1 << 0x10) * _DAT_0002fea0;
    fVar27 = (float)(int)((uint6)uVar2 >> 0x10) * _UNK_0002fea4;
    fVar28 = (float)(int)((unkuint10)Var3 >> 0x30) * _UNK_0002fea8;
    fVar29 = (float)auVar6._10_4_ * _UNK_0002feac;
    pfVar10 = param_2 + (uVar9 - 8);
    *pfVar10 = (float)(int)((uint)auVar22._0_2_ << 0x10) * _DAT_0002fea0;
    pfVar10[1] = fVar14;
    pfVar10[2] = fVar15;
    pfVar10[3] = fVar16;
    param_2 = param_2 + (uVar9 - 4);
    *param_2 = fVar17;
    param_2[1] = fVar27;
    param_2[2] = fVar28;
    param_2[3] = fVar29;
  }
  return;
}
