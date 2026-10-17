// _SamplePeaks4
// Entry: 00027c6c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _SamplePeaks4(long param_1,int *param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  ulong uVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  short sVar7;
  ushort uVar8;
  
  if (7 < param_3) {
    iVar1 = *param_2;
    iVar2 = param_2[1];
    iVar5 = param_2[2];
    iVar6 = param_2[3];
    puVar4 = (undefined4 *)(param_1 + 0xc);
    uVar3 = (ulong)(param_3 >> 3);
    do {
      sVar7 = (short)((uint)puVar4[-3] >> 8);
      uVar8 = sVar7 >> 0xf;
      uVar8 = sVar7 + uVar8 ^ uVar8;
      if ((short)uVar8 < (short)(ushort)iVar1) {
        uVar8 = (ushort)iVar1;
      }
      iVar1 = (int)(short)uVar8;
      *param_2 = iVar1;
      sVar7 = (short)((uint)puVar4[-2] >> 8);
      uVar8 = sVar7 >> 0xf;
      uVar8 = sVar7 + uVar8 ^ uVar8;
      if ((short)uVar8 < (short)(ushort)iVar2) {
        uVar8 = (ushort)iVar2;
      }
      iVar2 = (int)(short)uVar8;
      param_2[1] = iVar2;
      sVar7 = (short)((uint)puVar4[-1] >> 8);
      uVar8 = sVar7 >> 0xf;
      uVar8 = sVar7 + uVar8 ^ uVar8;
      if ((short)uVar8 < (short)(ushort)iVar5) {
        uVar8 = (ushort)iVar5;
      }
      iVar5 = (int)(short)uVar8;
      param_2[2] = iVar5;
      sVar7 = (short)((uint)*puVar4 >> 8);
      uVar8 = sVar7 >> 0xf;
      uVar8 = sVar7 + uVar8 ^ uVar8;
      if ((short)uVar8 < (short)(ushort)iVar6) {
        uVar8 = (ushort)iVar6;
      }
      iVar6 = (int)(short)uVar8;
      puVar4 = puVar4 + 0x20;
      param_2[3] = iVar6;
      uVar3 = uVar3 - 1;
    } while (uVar3 != 0);
  }
  return;
}
