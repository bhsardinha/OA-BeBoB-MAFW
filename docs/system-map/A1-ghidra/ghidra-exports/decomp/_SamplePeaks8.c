// _SamplePeaks8
// Entry: 00027f38
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _SamplePeaks8(long param_1,int *param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  ulong uVar3;
  int iVar4;
  undefined4 *puVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  short sVar11;
  ushort uVar12;
  
  if (7 < param_3) {
    iVar1 = *param_2;
    iVar2 = param_2[1];
    iVar6 = param_2[2];
    iVar7 = param_2[3];
    iVar8 = param_2[4];
    iVar9 = param_2[5];
    iVar4 = param_2[6];
    iVar10 = param_2[7];
    puVar5 = (undefined4 *)(param_1 + 0x1c);
    uVar3 = (ulong)(param_3 >> 3);
    do {
      sVar11 = (short)((uint)puVar5[-7] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar1) {
        uVar12 = (ushort)iVar1;
      }
      iVar1 = (int)(short)uVar12;
      *param_2 = iVar1;
      sVar11 = (short)((uint)puVar5[-6] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar2) {
        uVar12 = (ushort)iVar2;
      }
      iVar2 = (int)(short)uVar12;
      param_2[1] = iVar2;
      sVar11 = (short)((uint)puVar5[-5] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar6) {
        uVar12 = (ushort)iVar6;
      }
      iVar6 = (int)(short)uVar12;
      param_2[2] = iVar6;
      sVar11 = (short)((uint)puVar5[-4] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar7) {
        uVar12 = (ushort)iVar7;
      }
      iVar7 = (int)(short)uVar12;
      param_2[3] = iVar7;
      sVar11 = (short)((uint)puVar5[-3] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar8) {
        uVar12 = (ushort)iVar8;
      }
      iVar8 = (int)(short)uVar12;
      param_2[4] = iVar8;
      sVar11 = (short)((uint)puVar5[-2] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar9) {
        uVar12 = (ushort)iVar9;
      }
      iVar9 = (int)(short)uVar12;
      param_2[5] = iVar9;
      sVar11 = (short)((uint)puVar5[-1] >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar4) {
        uVar12 = (ushort)iVar4;
      }
      iVar4 = (int)(short)uVar12;
      param_2[6] = iVar4;
      sVar11 = (short)((uint)*puVar5 >> 8);
      uVar12 = sVar11 >> 0xf;
      uVar12 = sVar11 + uVar12 ^ uVar12;
      if ((short)uVar12 < (short)(ushort)iVar10) {
        uVar12 = (ushort)iVar10;
      }
      iVar10 = (int)(short)uVar12;
      puVar5 = puVar5 + 0x40;
      param_2[7] = iVar10;
      uVar3 = uVar3 - 1;
    } while (uVar3 != 0);
  }
  return;
}
