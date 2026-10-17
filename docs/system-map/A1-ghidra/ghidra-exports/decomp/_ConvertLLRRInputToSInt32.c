// _ConvertLLRRInputToSInt32
// Entry: 0002951f
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _ConvertLLRRInputToSInt32(uint param_1,undefined8 param_2,long param_3)

{
  uint uVar1;
  undefined4 uVar2;
  uint uVar3;
  uint uVar4;
  
  _ConvertLRLRInputToSInt32(param_1,param_2,param_3);
  if (param_1 != 0) {
    uVar3 = 0;
    do {
      uVar4 = uVar3 + 1;
      uVar2 = *(undefined4 *)(param_3 + (ulong)uVar4 * 4);
      uVar1 = uVar3 + 2;
      uVar3 = uVar3 + 4;
      *(undefined4 *)(param_3 + (ulong)uVar4 * 4) = *(undefined4 *)(param_3 + (ulong)uVar1 * 4);
      *(undefined4 *)(param_3 + (ulong)uVar1 * 4) = uVar2;
    } while (uVar3 < param_1);
  }
  return;
}
