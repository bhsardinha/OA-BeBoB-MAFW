// _RearrangeFrameLLRR10WithMIDI
// Entry: 0002b951
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

void _RearrangeFrameLLRR10WithMIDI(long param_1,long param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int *piVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  long local_80;
  
  puVar6 = (uint *)(param_1 + 0x24);
  piVar5 = (int *)(param_2 + 0x24);
  local_80 = (ulong)(param_3 - 1) + 1;
  do {
    uVar1 = puVar6[-9];
    uVar2 = puVar6[-8];
    uVar3 = puVar6[-7];
    uVar7 = puVar6[-6];
    uVar8 = puVar6[-5];
    uVar9 = puVar6[-4];
    uVar10 = puVar6[-3];
    uVar4 = puVar6[-2];
    uVar11 = puVar6[-1];
    uVar12 = *puVar6;
    if (0x7fffff < (int)uVar11) {
      uVar11 = 0x7fffff;
    }
    if ((int)uVar11 < -0x800000) {
      uVar11 = 0xff800000;
    }
    if (0x7fffff < (int)uVar1) {
      uVar1 = 0x7fffff;
    }
    if ((int)uVar1 < -0x800000) {
      uVar1 = 0xff800000;
    }
    if (0x7fffff < (int)uVar3) {
      uVar3 = 0x7fffff;
    }
    if ((int)uVar3 < -0x800000) {
      uVar3 = 0xff800000;
    }
    if (0x7fffff < (int)uVar8) {
      uVar8 = 0x7fffff;
    }
    if ((int)uVar8 < -0x800000) {
      uVar8 = 0xff800000;
    }
    if (0x7fffff < (int)uVar10) {
      uVar10 = 0x7fffff;
    }
    if ((int)uVar10 < -0x800000) {
      uVar10 = 0xff800000;
    }
    if (0x7fffff < (int)uVar12) {
      uVar12 = 0x7fffff;
    }
    if ((int)uVar12 < -0x800000) {
      uVar12 = 0xff800000;
    }
    if (0x7fffff < (int)uVar2) {
      uVar2 = 0x7fffff;
    }
    if ((int)uVar2 < -0x800000) {
      uVar2 = 0xff800000;
    }
    if (0x7fffff < (int)uVar7) {
      uVar7 = 0x7fffff;
    }
    if ((int)uVar7 < -0x800000) {
      uVar7 = 0xff800000;
    }
    if (0x7fffff < (int)uVar9) {
      uVar9 = 0x7fffff;
    }
    if ((int)uVar9 < -0x800000) {
      uVar9 = 0xff800000;
    }
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    puVar6 = puVar6 + 10;
    piVar5[-9] = uVar11 * 0x1000000 + (uVar11 & 0xff00) * 0x100 + (uVar11 >> 8 & 0xff00) + 0x40;
    piVar5[-8] = uVar1 * 0x1000000 + (uVar1 & 0xff00) * 0x100 + (uVar1 >> 8 & 0xff00) + 0x40;
    piVar5[-7] = uVar3 * 0x1000000 + (uVar3 & 0xff00) * 0x100 + (uVar3 >> 8 & 0xff00) + 0x40;
    piVar5[-6] = uVar8 * 0x1000000 + (uVar8 & 0xff00) * 0x100 + (uVar8 >> 8 & 0xff00) + 0x40;
    piVar5[-5] = uVar10 * 0x1000000 + (uVar10 & 0xff00) * 0x100 + (uVar10 >> 8 & 0xff00) + 0x40;
    piVar5[-4] = uVar12 * 0x1000000 + (uVar12 & 0xff00) * 0x100 + (uVar12 >> 8 & 0xff00) + 0x40;
    piVar5[-3] = uVar2 * 0x1000000 + (uVar2 & 0xff00) * 0x100 + (uVar2 >> 8 & 0xff00) + 0x40;
    piVar5[-2] = uVar7 * 0x1000000 + (uVar7 & 0xff00) * 0x100 + (uVar7 >> 8 & 0xff00) + 0x40;
    piVar5[-1] = uVar9 * 0x1000000 + (uVar9 & 0xff00) * 0x100 + (uVar9 >> 8 & 0xff00) + 0x40;
    *piVar5 = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    piVar5 = piVar5 + 0xb;
    local_80 = local_80 + -1;
  } while (local_80 != 0);
  return;
}
