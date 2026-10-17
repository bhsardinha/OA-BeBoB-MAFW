// ConvertSampleFramesFromInput
// Entry: 00027bb6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* ConvertSampleFramesFromInput(int const*, int*, unsigned int, unsigned int, unsigned int, unsigned
   int const*, bool) */

void ConvertSampleFramesFromInput
               (int *param_1,int *param_2,uint param_3,uint param_4,uint param_5,uint *param_6,
               bool param_7)

{
  uint uVar1;
  long lVar2;
  uint *puVar3;
  ulong uVar4;
  uint *puVar5;
  ulong uVar6;
  
  if ((param_5 != 0) && (param_3 != 0)) {
    uVar6 = (ulong)param_5;
    lVar2 = 0;
    uVar4 = (ulong)param_3;
    puVar5 = param_6;
    puVar3 = (uint *)param_1;
    do {
      do {
        if (*puVar5 < param_4) {
          uVar1 = *param_1;
          param_2[(ulong)*puVar5 + lVar2] =
               (int)(((uVar1 >> 8 & 0xff00) + (uVar1 >> 0x18) + (uVar1 & 0xff00) * 0x100) * 0x100)
               >> 8;
        }
        param_1 = param_1 + 1;
        uVar4 = uVar4 - 1;
        puVar5 = puVar5 + 1;
      } while (uVar4 != 0);
      param_1 = (int *)(puVar3 + (param_7 + param_3));
      lVar2 = lVar2 + (ulong)param_4;
      uVar6 = uVar6 - 1;
      uVar4 = (ulong)param_3;
      puVar5 = param_6;
      puVar3 = (uint *)param_1;
    } while (uVar6 != 0);
  }
  return;
}
