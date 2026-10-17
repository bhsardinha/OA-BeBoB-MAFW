// _RearrangeFrameLLRR4WithMIDI
// Entry: 0002bc6c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

void _RearrangeFrameLLRR4WithMIDI(long param_1,long param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  long lVar3;
  int *piVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  
  puVar5 = (uint *)(param_1 + 0xc);
  piVar4 = (int *)(param_2 + 0xc);
  lVar3 = (ulong)(param_3 - 1) + 1;
  do {
    uVar1 = puVar5[-3];
    uVar2 = puVar5[-2];
    uVar6 = puVar5[-1];
    uVar7 = *puVar5;
    if (0x7fffff < (int)uVar1) {
      uVar1 = 0x7fffff;
    }
    if ((int)uVar1 < -0x800000) {
      uVar1 = 0xff800000;
    }
    if (0x7fffff < (int)uVar6) {
      uVar6 = 0x7fffff;
    }
    if ((int)uVar6 < -0x800000) {
      uVar6 = 0xff800000;
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
    puVar5 = puVar5 + 4;
    piVar4[-3] = uVar1 * 0x1000000 + (uVar1 & 0xff00) * 0x100 + (uVar1 >> 8 & 0xff00) + 0x40;
    piVar4[-2] = uVar6 * 0x1000000 + (uVar6 & 0xff00) * 0x100 + (uVar6 >> 8 & 0xff00) + 0x40;
    piVar4[-1] = uVar2 * 0x1000000 + (uVar2 & 0xff00) * 0x100 + (uVar2 >> 8 & 0xff00) + 0x40;
    *piVar4 = uVar7 * 0x1000000 + (uVar7 & 0xff00) * 0x100 + (uVar7 >> 8 & 0xff00) + 0x40;
    piVar4 = piVar4 + 5;
    lVar3 = lVar3 + -1;
  } while (lVar3 != 0);
  return;
}
