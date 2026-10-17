// _ConvertLLRRInputToReverseSInt32
// Entry: 000294d4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _ConvertLLRRInputToReverseSInt32(uint param_1,undefined8 param_2,long param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  long lVar3;
  
  _ConvertLRLRInputToSInt32(param_1,param_2,param_3);
  if (param_1 != 0) {
    lVar3 = 0;
    do {
      uVar1 = *(undefined4 *)(param_3 + lVar3 * 4);
      uVar2 = *(undefined4 *)(param_3 + 8 + lVar3 * 4);
      *(undefined4 *)(param_3 + lVar3 * 4) = *(undefined4 *)(param_3 + 4 + lVar3 * 4);
      *(undefined4 *)(param_3 + 4 + lVar3 * 4) = *(undefined4 *)(param_3 + 0xc + lVar3 * 4);
      *(undefined4 *)(param_3 + 8 + lVar3 * 4) = uVar1;
      *(undefined4 *)(param_3 + 0xc + lVar3 * 4) = uVar2;
      lVar3 = lVar3 + 4;
    } while ((uint)lVar3 < param_1);
  }
  return;
}
