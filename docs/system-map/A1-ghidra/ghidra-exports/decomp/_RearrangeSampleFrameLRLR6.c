// _RearrangeSampleFrameLRLR6
// Entry: 0002b60c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _RearrangeSampleFrameLRLR6(long param_1,long param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  long local_50;
  
  piVar4 = (int *)(param_2 + 0x14);
  puVar5 = (uint *)(param_1 + 0x14);
  local_50 = (ulong)(param_3 - 1) + 1;
  do {
    uVar1 = puVar5[-5];
    uVar2 = puVar5[-4];
    uVar3 = puVar5[-3];
    uVar6 = puVar5[-2];
    uVar7 = puVar5[-1];
    uVar8 = *puVar5;
    if (0x7fffff < (int)uVar1) {
      uVar1 = 0x7fffff;
    }
    if ((int)uVar1 < -0x800000) {
      uVar1 = 0xff800000;
    }
    if (0x7fffff < (int)uVar2) {
      uVar2 = 0x7fffff;
    }
    if ((int)uVar2 < -0x800000) {
      uVar2 = 0xff800000;
    }
    if (0x7fffff < (int)uVar3) {
      uVar3 = 0x7fffff;
    }
    if ((int)uVar3 < -0x800000) {
      uVar3 = 0xff800000;
    }
    if (0x7fffff < (int)uVar6) {
      uVar6 = 0x7fffff;
    }
    if ((int)uVar6 < -0x800000) {
      uVar6 = 0xff800000;
    }
    if (0x7fffff < (int)uVar7) {
      uVar7 = 0x7fffff;
    }
    if ((int)uVar7 < -0x800000) {
      uVar7 = 0xff800000;
    }
    if (0x7fffff < (int)uVar8) {
      uVar8 = 0x7fffff;
    }
    if ((int)uVar8 < -0x800000) {
      uVar8 = 0xff800000;
    }
    piVar4[-5] = uVar1 * 0x1000000 + (uVar1 & 0xff00) * 0x100 + (uVar1 >> 8 & 0xff00) + 0x40;
    piVar4[-4] = uVar2 * 0x1000000 + (uVar2 & 0xff00) * 0x100 + (uVar2 >> 8 & 0xff00) + 0x40;
    piVar4[-3] = uVar3 * 0x1000000 + (uVar3 & 0xff00) * 0x100 + (uVar3 >> 8 & 0xff00) + 0x40;
    piVar4[-2] = uVar6 * 0x1000000 + (uVar6 & 0xff00) * 0x100 + (uVar6 >> 8 & 0xff00) + 0x40;
    piVar4[-1] = uVar7 * 0x1000000 + (uVar7 & 0xff00) * 0x100 + (uVar7 >> 8 & 0xff00) + 0x40;
    *piVar4 = uVar8 * 0x1000000 + (uVar8 & 0xff00) * 0x100 + (uVar8 >> 8 & 0xff00) + 0x40;
    piVar4 = piVar4 + 6;
    puVar5 = puVar5 + 6;
    local_50 = local_50 + -1;
  } while (local_50 != 0);
  return;
}
