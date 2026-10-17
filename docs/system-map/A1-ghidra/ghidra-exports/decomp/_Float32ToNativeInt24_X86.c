// _Float32ToNativeInt24_X86
// Entry: 0002d32b
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Removing unreachable block (ram,0x0002d5e2) */
/* WARNING: Removing unreachable block (ram,0x0002d5cc) */
/* WARNING: Removing unreachable block (ram,0x0002d5b6) */
/* WARNING: Removing unreachable block (ram,0x0002d53f) */
/* WARNING: Removing unreachable block (ram,0x0002d529) */
/* WARNING: Removing unreachable block (ram,0x0002d513) */
/* WARNING: Removing unreachable block (ram,0x0002d430) */
/* WARNING: Removing unreachable block (ram,0x0002d427) */
/* WARNING: Removing unreachable block (ram,0x0002d41e) */
/* WARNING: Removing unreachable block (ram,0x0002d3d8) */
/* WARNING: Removing unreachable block (ram,0x0002d3c2) */
/* WARNING: Removing unreachable block (ram,0x0002d3ac) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _Float32ToNativeInt24_X86(float *param_1,undefined1 (*param_2) [16],uint param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  double dVar9;
  double dVar10;
  uint uVar11;
  int iVar12;
  ulong uVar13;
  undefined1 *puVar14;
  undefined1 (*pauVar15) [16];
  float *pfVar16;
  undefined1 auVar17 [16];
  undefined1 auVar18 [16];
  undefined1 auVar19 [16];
  undefined1 auVar20 [16];
  undefined1 auVar21 [16];
  undefined1 auVar22 [16];
  undefined1 auVar23 [16];
  undefined1 auVar24 [16];
  double dVar25;
  undefined1 auVar26 [16];
  undefined1 auVar27 [16];
  undefined1 auVar28 [16];
  undefined1 auVar29 [16];
  
  dVar10 = DAT_0002fff0;
  dVar9 = DAT_0002ffc0;
  if (param_3 < 6) {
    if (param_3 != 0) {
      puVar14 = *param_2 + 2;
      uVar13 = (ulong)param_3;
      do {
        dVar25 = (double)*param_1 * dVar9 + dVar10;
        if (dVar25 < _DAT_0002fff8) {
          iVar12 = (int)dVar25;
        }
        else {
          iVar12 = 0x7fffffff;
        }
        puVar14[-2] = (char)((uint)iVar12 >> 8);
        puVar14[-1] = (char)((uint)iVar12 >> 0x10);
        *puVar14 = (char)((uint)iVar12 >> 0x18);
        puVar14 = puVar14 + 3;
        param_1 = param_1 + 1;
        uVar13 = uVar13 - 1;
      } while (uVar13 != 0);
    }
  }
  else {
    pauVar15 = param_2;
    pfVar16 = param_1;
    uVar11 = param_3;
    if (((ulong)param_1 & 0xf) != 0) {
      auVar19._0_4_ = *param_1 * _DAT_0002ff40 + _DAT_0002ff50;
      auVar19._4_4_ = param_1[1] * _UNK_0002ff44 + _UNK_0002ff54;
      auVar19._8_4_ = param_1[2] * _UNK_0002ff48 + _UNK_0002ff58;
      auVar19._12_4_ = param_1[3] * _UNK_0002ff4c + _UNK_0002ff5c;
      auVar19 = maxps(auVar19,_DAT_0002ff60);
      auVar19 = minps(auVar19,_DAT_0002ff70);
      auVar17._0_4_ = (int)auVar19._0_4_;
      auVar17._4_4_ = (int)auVar19._4_4_;
      auVar17._8_4_ = (int)auVar19._8_4_;
      auVar17._12_4_ = (int)auVar19._12_4_;
      auVar28._8_8_ = 0xff;
      auVar28._0_8_ = 0xffff000000000000;
      uVar11 = 0x10 - ((uint)param_1 & 0xf) >> 2;
      *param_2 = ZEXT816(0xffffff00) << 0x40 & auVar17 >> 0x20 |
                 auVar17 >> 0x18 & auVar28 |
                 auVar17 >> 0x10 & ZEXT816(0xffffff000000) | auVar17 >> 8 & ZEXT816(0xffffff);
      pauVar15 = (undefined1 (*) [16])(*param_2 + uVar11 * 3);
      pfVar16 = param_1 + uVar11;
      uVar11 = param_3 - uVar11;
    }
    auVar17 = _DAT_0002ff70;
    auVar19 = _DAT_0002ff60;
    fVar8 = _UNK_0002ff5c;
    fVar7 = _UNK_0002ff58;
    fVar6 = _UNK_0002ff54;
    fVar5 = _DAT_0002ff50;
    fVar4 = _UNK_0002ff4c;
    fVar3 = _UNK_0002ff48;
    fVar2 = _UNK_0002ff44;
    fVar1 = _DAT_0002ff40;
    if (5 < uVar11) {
      auVar26._8_8_ = 0xff;
      auVar26._0_8_ = 0xffff000000000000;
      do {
        auVar27._0_4_ = *pfVar16 * fVar1 + fVar5;
        auVar27._4_4_ = pfVar16[1] * fVar2 + fVar6;
        auVar27._8_4_ = pfVar16[2] * fVar3 + fVar7;
        auVar27._12_4_ = pfVar16[3] * fVar4 + fVar8;
        auVar28 = maxps(auVar27,auVar19);
        auVar28 = minps(auVar28,auVar17);
        auVar29._0_4_ = (int)auVar28._0_4_;
        auVar29._4_4_ = (int)auVar28._4_4_;
        auVar29._8_4_ = (int)auVar28._8_4_;
        auVar29._12_4_ = (int)auVar28._12_4_;
        pfVar16 = pfVar16 + 4;
        uVar11 = uVar11 - 4;
        *pauVar15 = auVar29 >> 0x20 & ZEXT816(0xffffff00) << 0x40 |
                    auVar29 >> 0x18 & auVar26 |
                    auVar29 >> 0x10 & ZEXT816(0xffffff000000) | auVar29 >> 8 & ZEXT816(0xffffff);
        pauVar15 = (undefined1 (*) [16])(*pauVar15 + 0xc);
      } while (5 < uVar11);
    }
    if (3 < uVar11) {
      auVar18._0_4_ = *pfVar16 * _DAT_0002ff40 + _DAT_0002ff50;
      auVar18._4_4_ = pfVar16[1] * _UNK_0002ff44 + _UNK_0002ff54;
      auVar18._8_4_ = pfVar16[2] * _UNK_0002ff48 + _UNK_0002ff58;
      auVar18._12_4_ = pfVar16[3] * _UNK_0002ff4c + _UNK_0002ff5c;
      auVar19 = maxps(auVar18,_DAT_0002ff60);
      auVar19 = minps(auVar19,_DAT_0002ff70);
      auVar20._0_4_ = (int)auVar19._0_4_;
      auVar20._4_4_ = (int)auVar19._4_4_;
      auVar20._8_4_ = (int)auVar19._8_4_;
      auVar20._12_4_ = (int)auVar19._12_4_;
      auVar23._8_8_ = 0xff;
      auVar23._0_8_ = 0xffff000000000000;
      auVar19 = ZEXT816(0xffffff00) << 0x40 & auVar20 >> 0x20 |
                auVar20 >> 0x18 & auVar23 |
                auVar20 >> 0x10 & ZEXT816(0xffffff000000) | auVar20 >> 8 & ZEXT816(0xffffff);
      uVar11 = uVar11 - 4;
      *(int *)*pauVar15 = auVar19._0_4_;
      *(int *)(*pauVar15 + 4) = auVar19._4_4_;
      *(int *)(*pauVar15 + 8) = auVar19._8_4_;
    }
    if (uVar11 != 0) {
      uVar13 = (ulong)(param_3 * 3);
      param_1 = param_1 + ((ulong)param_3 - 4);
      auVar21._0_4_ = *param_1 * _DAT_0002ff40 + _DAT_0002ff50;
      auVar21._4_4_ = param_1[1] * _UNK_0002ff44 + _UNK_0002ff54;
      auVar21._8_4_ = param_1[2] * _UNK_0002ff48 + _UNK_0002ff58;
      auVar21._12_4_ = param_1[3] * _UNK_0002ff4c + _UNK_0002ff5c;
      auVar19 = maxps(auVar21,_DAT_0002ff60);
      auVar19 = minps(auVar19,_DAT_0002ff70);
      auVar22._0_4_ = (int)auVar19._0_4_;
      auVar22._4_4_ = (int)auVar19._4_4_;
      auVar22._8_4_ = (int)auVar19._8_4_;
      auVar22._12_4_ = (int)auVar19._12_4_;
      auVar24._8_8_ = 0xff;
      auVar24._0_8_ = 0xffff000000000000;
      auVar19 = ZEXT816(0xffffff00) << 0x40 & auVar22 >> 0x20 |
                auVar22 >> 0x18 & auVar24 |
                auVar22 >> 0x10 & ZEXT816(0xffffff000000) | auVar22 >> 8 & ZEXT816(0xffffff);
      *(int *)(param_2[-1] + uVar13 + 4) = auVar19._0_4_;
      *(int *)(param_2[-1] + uVar13 + 8) = auVar19._4_4_;
      *(int *)(param_2[-1] + uVar13 + 0xc) = auVar19._8_4_;
    }
  }
  return;
}
