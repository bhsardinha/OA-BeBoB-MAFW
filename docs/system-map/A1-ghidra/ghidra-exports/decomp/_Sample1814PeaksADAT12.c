// _Sample1814PeaksADAT12
// Entry: 0002884a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

void _Sample1814PeaksADAT12(long param_1,int *param_2,uint param_3)

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
  short sVar10;
  ushort uVar11;
  int iVar12;
  int iVar13;
  int local_3c;
  int local_38;
  int local_34;
  
  if (7 < param_3) {
    local_34 = *param_2;
    local_38 = param_2[1];
    local_3c = param_2[2];
    iVar1 = param_2[3];
    iVar2 = param_2[6];
    iVar6 = param_2[7];
    iVar7 = param_2[8];
    iVar8 = param_2[9];
    iVar9 = param_2[10];
    iVar4 = param_2[0xb];
    iVar12 = param_2[0xc];
    iVar13 = param_2[0xd];
    puVar5 = (undefined4 *)(param_1 + 0x2c);
    uVar3 = (ulong)(param_3 >> 3);
    do {
      sVar10 = (short)((uint)puVar5[-0xb] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)local_34) {
        uVar11 = (ushort)local_34;
      }
      local_34 = (int)(short)uVar11;
      *param_2 = local_34;
      sVar10 = (short)((uint)puVar5[-10] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)local_38) {
        uVar11 = (ushort)local_38;
      }
      local_38 = (int)(short)uVar11;
      param_2[1] = local_38;
      sVar10 = (short)((uint)puVar5[-9] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)local_3c) {
        uVar11 = (ushort)local_3c;
      }
      local_3c = (int)(short)uVar11;
      param_2[2] = local_3c;
      sVar10 = (short)((uint)puVar5[-8] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar1) {
        uVar11 = (ushort)iVar1;
      }
      iVar1 = (int)(short)uVar11;
      param_2[3] = iVar1;
      sVar10 = (short)((uint)puVar5[-7] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar2) {
        uVar11 = (ushort)iVar2;
      }
      iVar2 = (int)(short)uVar11;
      param_2[6] = iVar2;
      sVar10 = (short)((uint)puVar5[-6] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar6) {
        uVar11 = (ushort)iVar6;
      }
      iVar6 = (int)(short)uVar11;
      param_2[7] = iVar6;
      sVar10 = (short)((uint)puVar5[-5] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar7) {
        uVar11 = (ushort)iVar7;
      }
      iVar7 = (int)(short)uVar11;
      param_2[8] = iVar7;
      sVar10 = (short)((uint)puVar5[-4] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar8) {
        uVar11 = (ushort)iVar8;
      }
      iVar8 = (int)(short)uVar11;
      param_2[9] = iVar8;
      sVar10 = (short)((uint)puVar5[-3] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar9) {
        uVar11 = (ushort)iVar9;
      }
      iVar9 = (int)(short)uVar11;
      param_2[10] = iVar9;
      sVar10 = (short)((uint)puVar5[-2] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar4) {
        uVar11 = (ushort)iVar4;
      }
      iVar4 = (int)(short)uVar11;
      param_2[0xb] = iVar4;
      sVar10 = (short)((uint)puVar5[-1] >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar12) {
        uVar11 = (ushort)iVar12;
      }
      iVar12 = (int)(short)uVar11;
      param_2[0xc] = iVar12;
      sVar10 = (short)((uint)*puVar5 >> 8);
      uVar11 = sVar10 >> 0xf;
      uVar11 = sVar10 + uVar11 ^ uVar11;
      if ((short)uVar11 < (short)(ushort)iVar13) {
        uVar11 = (ushort)iVar13;
      }
      iVar13 = (int)(short)uVar11;
      puVar5 = puVar5 + 0x60;
      param_2[0xd] = iVar13;
      uVar3 = uVar3 - 1;
    } while (uVar3 != 0);
  }
  return;
}
