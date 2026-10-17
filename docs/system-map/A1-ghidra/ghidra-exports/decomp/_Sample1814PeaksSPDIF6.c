// _Sample1814PeaksSPDIF6
// Entry: 00028c62
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

void _Sample1814PeaksSPDIF6(long param_1,int *param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  ulong uVar3;
  short sVar4;
  ushort uVar5;
  undefined4 *puVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  
  if (7 < param_3) {
    iVar1 = *param_2;
    iVar2 = param_2[1];
    iVar7 = param_2[2];
    iVar8 = param_2[3];
    iVar9 = param_2[4];
    iVar10 = param_2[5];
    puVar6 = (undefined4 *)(param_1 + 0x14);
    uVar3 = (ulong)(param_3 >> 3);
    do {
      sVar4 = (short)((uint)puVar6[-5] >> 8);
      uVar5 = sVar4 >> 0xf;
      uVar5 = sVar4 + uVar5 ^ uVar5;
      if ((short)uVar5 < (short)(ushort)iVar1) {
        uVar5 = (ushort)iVar1;
      }
      iVar1 = (int)(short)uVar5;
      *param_2 = iVar1;
      sVar4 = (short)((uint)puVar6[-4] >> 8);
      uVar5 = sVar4 >> 0xf;
      uVar5 = sVar4 + uVar5 ^ uVar5;
      if ((short)uVar5 < (short)(ushort)iVar2) {
        uVar5 = (ushort)iVar2;
      }
      iVar2 = (int)(short)uVar5;
      param_2[1] = iVar2;
      sVar4 = (short)((uint)puVar6[-3] >> 8);
      uVar5 = sVar4 >> 0xf;
      uVar5 = sVar4 + uVar5 ^ uVar5;
      if ((short)uVar5 < (short)(ushort)iVar7) {
        uVar5 = (ushort)iVar7;
      }
      iVar7 = (int)(short)uVar5;
      param_2[2] = iVar7;
      sVar4 = (short)((uint)puVar6[-2] >> 8);
      uVar5 = sVar4 >> 0xf;
      uVar5 = sVar4 + uVar5 ^ uVar5;
      if ((short)uVar5 < (short)(ushort)iVar8) {
        uVar5 = (ushort)iVar8;
      }
      iVar8 = (int)(short)uVar5;
      param_2[3] = iVar8;
      sVar4 = (short)((uint)puVar6[-1] >> 8);
      uVar5 = sVar4 >> 0xf;
      uVar5 = sVar4 + uVar5 ^ uVar5;
      if ((short)uVar5 < (short)(ushort)iVar9) {
        uVar5 = (ushort)iVar9;
      }
      iVar9 = (int)(short)uVar5;
      param_2[4] = iVar9;
      sVar4 = (short)((uint)*puVar6 >> 8);
      uVar5 = sVar4 >> 0xf;
      uVar5 = sVar4 + uVar5 ^ uVar5;
      if ((short)uVar5 < (short)(ushort)iVar10) {
        uVar5 = (ushort)iVar10;
      }
      iVar10 = (int)(short)uVar5;
      puVar6 = puVar6 + 0x30;
      param_2[5] = iVar10;
      uVar3 = uVar3 - 1;
    } while (uVar3 != 0);
  }
  return;
}
