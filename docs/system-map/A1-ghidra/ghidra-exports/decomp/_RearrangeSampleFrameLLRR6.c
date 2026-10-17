// _RearrangeSampleFrameLLRR6
// Entry: 0002c0e5
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _RearrangeSampleFrameLLRR6(long param_1,long param_2,int param_3)

{
  long lVar1;
  int *piVar2;
  uint *puVar3;
  uint uVar4;
  
  puVar3 = (uint *)(param_1 + 0x14);
  piVar2 = (int *)(param_2 + 0x14);
  lVar1 = (ulong)(param_3 - 1) + 1;
  do {
    uVar4 = puVar3[-5];
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    piVar2[-5] = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    uVar4 = puVar3[-3];
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    piVar2[-4] = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    uVar4 = puVar3[-1];
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    piVar2[-3] = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    uVar4 = puVar3[-4];
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    piVar2[-2] = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    uVar4 = puVar3[-2];
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    piVar2[-1] = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    uVar4 = *puVar3;
    if (0x7fffff < (int)uVar4) {
      uVar4 = 0x7fffff;
    }
    if ((int)uVar4 < -0x800000) {
      uVar4 = 0xff800000;
    }
    puVar3 = puVar3 + 6;
    *piVar2 = uVar4 * 0x1000000 + (uVar4 & 0xff00) * 0x100 + (uVar4 >> 8 & 0xff00) + 0x40;
    piVar2 = piVar2 + 7;
    lVar1 = lVar1 + -1;
  } while (lVar1 != 0);
  return;
}
