// _NativeInt16ToFloat32_X86
// Entry: 0002cd46
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _NativeInt16ToFloat32_X86(ushort *param_1,float *param_2,uint param_3)

{
  int iVar1;
  undefined6 uVar2;
  unkbyte10 Var3;
  undefined1 auVar4 [14];
  undefined1 auVar5 [14];
  undefined1 auVar6 [14];
  ushort uVar7;
  double dVar8;
  uint uVar9;
  ulong uVar10;
  float *pfVar11;
  uint uVar12;
  ushort *puVar13;
  ulong uVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar25;
  float fVar26;
  undefined1 auVar19 [16];
  undefined1 auVar22 [16];
  float fVar27;
  undefined1 auVar34 [14];
  undefined1 auVar28 [16];
  undefined1 auVar31 [16];
  undefined1 auVar20 [16];
  undefined1 auVar23 [16];
  undefined1 auVar21 [16];
  undefined1 auVar24 [16];
  undefined1 auVar29 [16];
  undefined1 auVar32 [16];
  undefined1 auVar30 [16];
  undefined1 auVar33 [16];
  
  dVar8 = DAT_0002ffe0;
  if (param_3 < 8) {
    if (param_3 == 0) {
      return;
    }
    uVar10 = (ulong)param_3;
    do {
      *param_2 = (float)((double)(int)(short)*param_1 * dVar8);
      param_2 = param_2 + 1;
      param_1 = param_1 + 1;
      uVar10 = uVar10 - 1;
    } while (uVar10 != 0);
    return;
  }
  uVar12 = (uint)param_2;
  pfVar11 = param_2;
  puVar13 = param_1;
  uVar9 = param_3;
  if (((uVar12 | (uint)param_1) & 0xf) != 0) {
    auVar21._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
    auVar21._12_2_ = 0;
    auVar21._14_2_ = param_1[3];
    auVar20._12_4_ = auVar21._12_4_;
    auVar20._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
    auVar20._10_2_ = param_1[2];
    auVar19._10_6_ = auVar20._10_6_;
    auVar19._0_10_ = (unkuint10)0 << 0x40;
    auVar34 = SUB1614((undefined1  [16])0x0,2);
    iVar1 = CONCAT22(auVar34._8_2_,param_1[4]);
    uVar2 = CONCAT24(param_1[5],iVar1);
    Var3 = CONCAT28(param_1[6],CONCAT26(auVar34._10_2_,uVar2));
    auVar34._10_2_ = auVar34._12_2_;
    auVar34._0_10_ = Var3;
    auVar34._12_2_ = param_1[7];
    fVar15 = (float)((int)CONCAT82(auVar19._8_8_,param_1[1]) << 0x10) * _UNK_0002feb4;
    fVar16 = (float)auVar19._8_4_ * _UNK_0002feb8;
    fVar17 = (float)auVar20._12_4_ * _UNK_0002febc;
    fVar18 = (float)(iVar1 << 0x10) * _DAT_0002feb0;
    fVar25 = (float)(int)((uint6)uVar2 >> 0x10) * _UNK_0002feb4;
    fVar26 = (float)(int)((unkuint10)Var3 >> 0x30) * _UNK_0002feb8;
    fVar27 = (float)auVar34._10_4_ * _UNK_0002febc;
    uVar9 = 0x10 - (uVar12 & 0xf) >> 2;
    uVar10 = (ulong)uVar9;
    uVar9 = param_3 - uVar9;
    *param_2 = (float)(int)((uint)*param_1 << 0x10) * _DAT_0002feb0;
    param_2[1] = fVar15;
    param_2[2] = fVar16;
    param_2[3] = fVar17;
    param_2[4] = fVar18;
    param_2[5] = fVar25;
    param_2[6] = fVar26;
    param_2[7] = fVar27;
    fVar18 = _UNK_0002febc;
    fVar17 = _UNK_0002feb8;
    fVar16 = _UNK_0002feb4;
    fVar15 = _DAT_0002feb0;
    if (((ulong)(param_1 + uVar10) & 0xf) != 0) {
      if (7 < uVar9) {
        uVar12 = 0x10 - (uVar12 & 0xf) >> 2;
        uVar9 = param_3 - uVar12;
        uVar14 = 0;
        do {
          puVar13 = param_1 + (uVar10 | uVar14);
          uVar7 = puVar13[1];
          auVar33._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
          auVar33._12_2_ = 0;
          auVar33._14_2_ = puVar13[3];
          auVar32._12_4_ = auVar33._12_4_;
          auVar32._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
          auVar32._10_2_ = puVar13[2];
          auVar31._10_6_ = auVar32._10_6_;
          auVar31._0_10_ = (unkuint10)0 << 0x40;
          auVar34 = SUB1614((undefined1  [16])0x0,2);
          iVar1 = CONCAT22(auVar34._8_2_,puVar13[4]);
          uVar2 = CONCAT24(puVar13[5],iVar1);
          Var3 = CONCAT28(puVar13[6],CONCAT26(auVar34._10_2_,uVar2));
          auVar5._10_2_ = auVar34._12_2_;
          auVar5._0_10_ = Var3;
          auVar5._12_2_ = puVar13[7];
          pfVar11 = param_2 + (uVar10 | uVar14);
          *pfVar11 = (float)(int)((uint)*puVar13 << 0x10) * fVar15;
          pfVar11[1] = (float)((int)CONCAT82(auVar31._8_8_,uVar7) << 0x10) * fVar16;
          pfVar11[2] = (float)auVar31._8_4_ * fVar17;
          pfVar11[3] = (float)auVar32._12_4_ * fVar18;
          pfVar11 = param_2 + (ulong)uVar12 + uVar14 + 4;
          *pfVar11 = (float)(iVar1 << 0x10) * fVar15;
          pfVar11[1] = (float)(int)((uint6)uVar2 >> 0x10) * fVar16;
          pfVar11[2] = (float)(int)((unkuint10)Var3 >> 0x30) * fVar17;
          pfVar11[3] = (float)auVar5._10_4_ * fVar18;
          uVar14 = uVar14 + 8;
          uVar9 = uVar9 - 8;
        } while (7 < uVar9);
      }
      goto LAB_0002ce6b;
    }
    pfVar11 = param_2 + uVar10;
    puVar13 = param_1 + uVar10;
  }
  fVar18 = _UNK_0002febc;
  fVar17 = _UNK_0002feb8;
  fVar16 = _UNK_0002feb4;
  fVar15 = _DAT_0002feb0;
  if (7 < uVar9) {
    pfVar11 = pfVar11 + 4;
    do {
      uVar7 = puVar13[1];
      auVar30._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
      auVar30._12_2_ = 0;
      auVar30._14_2_ = puVar13[3];
      auVar29._12_4_ = auVar30._12_4_;
      auVar29._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
      auVar29._10_2_ = puVar13[2];
      auVar28._10_6_ = auVar29._10_6_;
      auVar28._0_10_ = (unkuint10)0 << 0x40;
      auVar34 = SUB1614((undefined1  [16])0x0,2);
      iVar1 = CONCAT22(auVar34._8_2_,puVar13[4]);
      uVar2 = CONCAT24(puVar13[5],iVar1);
      Var3 = CONCAT28(puVar13[6],CONCAT26(auVar34._10_2_,uVar2));
      auVar4._10_2_ = auVar34._12_2_;
      auVar4._0_10_ = Var3;
      auVar4._12_2_ = puVar13[7];
      pfVar11[-4] = (float)(int)((uint)*puVar13 << 0x10) * fVar15;
      pfVar11[-3] = (float)((int)CONCAT82(auVar28._8_8_,uVar7) << 0x10) * fVar16;
      pfVar11[-2] = (float)auVar28._8_4_ * fVar17;
      pfVar11[-1] = (float)auVar29._12_4_ * fVar18;
      *pfVar11 = (float)(iVar1 << 0x10) * fVar15;
      pfVar11[1] = (float)(int)((uint6)uVar2 >> 0x10) * fVar16;
      pfVar11[2] = (float)(int)((unkuint10)Var3 >> 0x30) * fVar17;
      pfVar11[3] = (float)auVar4._10_4_ * fVar18;
      pfVar11 = pfVar11 + 8;
      puVar13 = puVar13 + 8;
      uVar9 = uVar9 - 8;
    } while (7 < uVar9);
  }
LAB_0002ce6b:
  if (uVar9 != 0) {
    param_1 = param_1 + ((ulong)param_3 - 8);
    auVar24._0_12_ = SUB1412(SUB1614((undefined1  [16])0x0,0),0);
    auVar24._12_2_ = 0;
    auVar24._14_2_ = param_1[3];
    auVar23._12_4_ = auVar24._12_4_;
    auVar23._0_10_ = SUB1410(SUB1614((undefined1  [16])0x0,0),0);
    auVar23._10_2_ = param_1[2];
    auVar22._10_6_ = auVar23._10_6_;
    auVar22._0_10_ = (unkuint10)0 << 0x40;
    auVar34 = SUB1614((undefined1  [16])0x0,2);
    iVar1 = CONCAT22(auVar34._8_2_,param_1[4]);
    uVar2 = CONCAT24(param_1[5],iVar1);
    Var3 = CONCAT28(param_1[6],CONCAT26(auVar34._10_2_,uVar2));
    auVar6._10_2_ = auVar34._12_2_;
    auVar6._0_10_ = Var3;
    auVar6._12_2_ = param_1[7];
    fVar15 = (float)((int)CONCAT82(auVar22._8_8_,param_1[1]) << 0x10) * _UNK_0002feb4;
    fVar16 = (float)auVar22._8_4_ * _UNK_0002feb8;
    fVar17 = (float)auVar23._12_4_ * _UNK_0002febc;
    fVar18 = (float)(iVar1 << 0x10) * _DAT_0002feb0;
    fVar25 = (float)(int)((uint6)uVar2 >> 0x10) * _UNK_0002feb4;
    fVar26 = (float)(int)((unkuint10)Var3 >> 0x30) * _UNK_0002feb8;
    fVar27 = (float)auVar6._10_4_ * _UNK_0002febc;
    pfVar11 = param_2 + ((ulong)param_3 - 8);
    *pfVar11 = (float)(int)((uint)*param_1 << 0x10) * _DAT_0002feb0;
    pfVar11[1] = fVar15;
    pfVar11[2] = fVar16;
    pfVar11[3] = fVar17;
    param_2 = param_2 + ((ulong)param_3 - 4);
    *param_2 = fVar18;
    param_2[1] = fVar25;
    param_2[2] = fVar26;
    param_2[3] = fVar27;
  }
  return;
}
