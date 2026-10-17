// _Float32ToNativeInt32_X86
// Entry: 0002d13d
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _Float32ToNativeInt32_X86(float *param_1,int *param_2,uint param_3)

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
  undefined1 auVar10 [16];
  double dVar11;
  double dVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  uint uVar21;
  int iVar22;
  ulong uVar23;
  uint uVar24;
  uint uVar25;
  int *piVar26;
  float *pfVar27;
  undefined1 auVar28 [16];
  undefined1 auVar29 [16];
  undefined1 auVar30 [16];
  undefined1 auVar31 [16];
  
  dVar12 = DAT_0002fff0;
  dVar11 = DAT_0002ffe8;
  if (param_3 < 4) {
    if (param_3 == 0) {
      return;
    }
    uVar23 = (ulong)param_3;
    do {
      dVar1 = (double)*param_1 * dVar11 + dVar12;
      if (dVar1 < _DAT_0002fff8) {
        iVar22 = (int)dVar1;
      }
      else {
        iVar22 = 0x7fffffff;
      }
      *param_2 = iVar22;
      param_2 = param_2 + 1;
      param_1 = param_1 + 1;
      uVar23 = uVar23 - 1;
    } while (uVar23 != 0);
    return;
  }
  uVar24 = (uint)param_2;
  piVar26 = param_2;
  pfVar27 = param_1;
  uVar21 = param_3;
  fVar13 = _DAT_0002ff00;
  fVar14 = _UNK_0002ff04;
  fVar15 = _UNK_0002ff08;
  fVar16 = _UNK_0002ff0c;
  fVar17 = _DAT_0002ff10;
  fVar18 = _UNK_0002ff14;
  fVar19 = _UNK_0002ff18;
  fVar20 = _UNK_0002ff1c;
  auVar29 = _DAT_0002ff20;
  auVar31 = _DAT_0002ff30;
  if (((uVar24 | (uint)param_1) & 0xf) != 0) {
    auVar29._0_4_ = *param_1 * _DAT_0002ff00 + _DAT_0002ff10;
    auVar29._4_4_ = param_1[1] * _UNK_0002ff04 + _UNK_0002ff14;
    auVar29._8_4_ = param_1[2] * _UNK_0002ff08 + _UNK_0002ff18;
    auVar29._12_4_ = param_1[3] * _UNK_0002ff0c + _UNK_0002ff1c;
    auVar29 = maxps(auVar29,_DAT_0002ff20);
    auVar29 = minps(auVar29,_DAT_0002ff30);
    uVar25 = 0x10 - (uVar24 & 0xf) >> 2;
    uVar21 = param_3 - uVar25;
    *param_2 = (int)auVar29._0_4_;
    param_2[1] = (int)auVar29._4_4_;
    param_2[2] = (int)auVar29._8_4_;
    param_2[3] = (int)auVar29._12_4_;
    auVar10 = _DAT_0002ff30;
    auVar28 = _DAT_0002ff20;
    fVar9 = _UNK_0002ff1c;
    fVar8 = _UNK_0002ff18;
    fVar7 = _UNK_0002ff14;
    fVar6 = _DAT_0002ff10;
    fVar5 = _UNK_0002ff0c;
    fVar4 = _UNK_0002ff08;
    fVar3 = _UNK_0002ff04;
    fVar2 = _DAT_0002ff00;
    fVar13 = _DAT_0002ff00;
    fVar14 = _UNK_0002ff04;
    fVar15 = _UNK_0002ff08;
    fVar16 = _UNK_0002ff0c;
    fVar17 = _DAT_0002ff10;
    fVar18 = _UNK_0002ff14;
    fVar19 = _UNK_0002ff18;
    fVar20 = _UNK_0002ff1c;
    auVar29 = _DAT_0002ff20;
    auVar31 = _DAT_0002ff30;
    if (((ulong)(param_1 + uVar25) & 0xf) != 0) {
      if (3 < uVar21) {
        uVar21 = 0x10 - (uVar24 & 0xf) >> 2;
        piVar26 = param_2 + uVar21;
        pfVar27 = param_1 + uVar21;
        uVar21 = param_3 - uVar21;
        do {
          auVar31._0_4_ = *pfVar27 * fVar2 + fVar6;
          auVar31._4_4_ = pfVar27[1] * fVar3 + fVar7;
          auVar31._8_4_ = pfVar27[2] * fVar4 + fVar8;
          auVar31._12_4_ = pfVar27[3] * fVar5 + fVar9;
          auVar29 = maxps(auVar31,auVar28);
          auVar29 = minps(auVar29,auVar10);
          *piVar26 = (int)auVar29._0_4_;
          piVar26[1] = (int)auVar29._4_4_;
          piVar26[2] = (int)auVar29._8_4_;
          piVar26[3] = (int)auVar29._12_4_;
          piVar26 = piVar26 + 4;
          pfVar27 = pfVar27 + 4;
          uVar21 = uVar21 - 4;
          fVar13 = _DAT_0002ff00;
          fVar14 = _UNK_0002ff04;
          fVar15 = _UNK_0002ff08;
          fVar16 = _UNK_0002ff0c;
          fVar17 = _DAT_0002ff10;
          fVar18 = _UNK_0002ff14;
          fVar19 = _UNK_0002ff18;
          fVar20 = _UNK_0002ff1c;
          auVar29 = _DAT_0002ff20;
          auVar31 = _DAT_0002ff30;
        } while (3 < uVar21);
      }
      goto LAB_0002d282;
    }
    piVar26 = param_2 + uVar25;
    pfVar27 = param_1 + uVar25;
  }
  for (; auVar10 = _DAT_0002ff30, auVar28 = _DAT_0002ff20, fVar9 = _UNK_0002ff1c,
      fVar8 = _UNK_0002ff18, fVar7 = _UNK_0002ff14, fVar6 = _DAT_0002ff10, fVar5 = _UNK_0002ff0c,
      fVar4 = _UNK_0002ff08, fVar3 = _UNK_0002ff04, fVar2 = _DAT_0002ff00, 3 < uVar21;
      uVar21 = uVar21 - 4) {
    auVar30._0_4_ = *pfVar27 * _DAT_0002ff00 + _DAT_0002ff10;
    auVar30._4_4_ = pfVar27[1] * _UNK_0002ff04 + _UNK_0002ff14;
    auVar30._8_4_ = pfVar27[2] * _UNK_0002ff08 + _UNK_0002ff18;
    auVar30._12_4_ = pfVar27[3] * _UNK_0002ff0c + _UNK_0002ff1c;
    auVar30 = maxps(auVar30,_DAT_0002ff20);
    auVar30 = minps(auVar30,_DAT_0002ff30);
    _DAT_0002ff00 = fVar13;
    _UNK_0002ff04 = fVar14;
    _UNK_0002ff08 = fVar15;
    _UNK_0002ff0c = fVar16;
    _DAT_0002ff10 = fVar17;
    _UNK_0002ff14 = fVar18;
    _UNK_0002ff18 = fVar19;
    _UNK_0002ff1c = fVar20;
    _DAT_0002ff20 = auVar29;
    _DAT_0002ff30 = auVar31;
    *piVar26 = (int)auVar30._0_4_;
    piVar26[1] = (int)auVar30._4_4_;
    piVar26[2] = (int)auVar30._8_4_;
    piVar26[3] = (int)auVar30._12_4_;
    piVar26 = piVar26 + 4;
    pfVar27 = pfVar27 + 4;
    fVar13 = _DAT_0002ff00;
    fVar14 = _UNK_0002ff04;
    fVar15 = _UNK_0002ff08;
    fVar16 = _UNK_0002ff0c;
    fVar17 = _DAT_0002ff10;
    fVar18 = _UNK_0002ff14;
    fVar19 = _UNK_0002ff18;
    fVar20 = _UNK_0002ff1c;
    auVar29 = _DAT_0002ff20;
    auVar31 = _DAT_0002ff30;
    _DAT_0002ff30 = auVar10;
    _DAT_0002ff20 = auVar28;
    _UNK_0002ff1c = fVar9;
    _UNK_0002ff18 = fVar8;
    _UNK_0002ff14 = fVar7;
    _DAT_0002ff10 = fVar6;
    _UNK_0002ff0c = fVar5;
    _UNK_0002ff08 = fVar4;
    _UNK_0002ff04 = fVar3;
    _DAT_0002ff00 = fVar2;
  }
LAB_0002d282:
  _DAT_0002ff30 = auVar31;
  _DAT_0002ff20 = auVar29;
  _UNK_0002ff1c = fVar20;
  _UNK_0002ff18 = fVar19;
  _UNK_0002ff14 = fVar18;
  _DAT_0002ff10 = fVar17;
  _UNK_0002ff0c = fVar16;
  _UNK_0002ff08 = fVar15;
  _UNK_0002ff04 = fVar14;
  _DAT_0002ff00 = fVar13;
  if (uVar21 != 0) {
    param_1 = param_1 + ((ulong)param_3 - 4);
    auVar28._0_4_ = *param_1 * _DAT_0002ff00 + _DAT_0002ff10;
    auVar28._4_4_ = param_1[1] * _UNK_0002ff04 + _UNK_0002ff14;
    auVar28._8_4_ = param_1[2] * _UNK_0002ff08 + _UNK_0002ff18;
    auVar28._12_4_ = param_1[3] * _UNK_0002ff0c + _UNK_0002ff1c;
    auVar29 = maxps(auVar28,_DAT_0002ff20);
    auVar29 = minps(auVar29,_DAT_0002ff30);
    param_2 = param_2 + ((ulong)param_3 - 4);
    *param_2 = (int)auVar29._0_4_;
    param_2[1] = (int)auVar29._4_4_;
    param_2[2] = (int)auVar29._8_4_;
    param_2[3] = (int)auVar29._12_4_;
  }
  return;
}
