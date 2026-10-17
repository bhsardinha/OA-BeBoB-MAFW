// _CopyChannelPairFromMultichannelBuffer
// Entry: 00028eac
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

void _CopyChannelPairFromMultichannelBuffer
               (uint param_1,uint param_2,uint param_3,long param_4,long param_5)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_1 & 0x7fffffff) != 0) {
    iVar2 = param_1 * -2;
    do {
      uVar1 = *(undefined4 *)(param_4 + (ulong)(param_3 + 1) * 4);
      *(undefined4 *)(param_5 + (ulong)param_3 * 4) = *(undefined4 *)(param_4 + (ulong)param_3 * 4);
      *(undefined4 *)(param_5 + (ulong)(param_3 + 1) * 4) = uVar1;
      param_5 = param_5 + 8;
      param_4 = param_4 + (ulong)param_2 * 4;
      iVar2 = iVar2 + 2;
    } while (iVar2 != 0);
  }
  return;
}
