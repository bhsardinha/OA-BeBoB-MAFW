// com_m_audio_FWEngine2::LocalRearrangeSampleFrame16Bit2
// Entry: 000118be
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2b)

/* com_m_audio_FWEngine2::LocalRearrangeSampleFrame16Bit2(unsigned short*, unsigned short*, int*,
   unsigned int, unsigned int, bool) */

void com_m_audio_FWEngine2::LocalRearrangeSampleFrame16Bit2
               (ushort *param_1,ushort *param_2,int *param_3,uint param_4,uint param_5,bool param_6)

{
  ushort uVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  undefined4 in_register_0000000c;
  undefined4 *puVar5;
  ulong uVar6;
  long lVar7;
  long lVar8;
  undefined4 *puVar9;
  uint uVar10;
  undefined7 in_register_00000089;
  undefined4 *puVar11;
  long lVar12;
  uint local_38;
  uint local_34;
  
  puVar5 = (undefined4 *)CONCAT44(in_register_0000000c,param_4);
  uVar10 = (uint)CONCAT71(in_register_00000089,param_6);
  uVar3 = *(uint *)(param_1 + 0x174);
  (**(code **)(**(long **)(param_1 + 0x9c) + 0xaa0))(*(long **)(param_1 + 0x9c),&local_34,&local_38)
  ;
  uVar4 = uVar10 - uVar3;
  lVar7 = 1;
  if (1 < uVar4) {
    lVar7 = (ulong)(uVar4 - 1) + 1;
  }
  uVar6 = CONCAT71(in_register_00000089,param_6) & 0xffffffff;
  lVar8 = 0;
  puVar9 = puVar5;
  do {
    if (param_3 <= param_2) {
      return;
    }
    uVar1 = param_2[1];
    uVar2 = *param_2;
    lVar12 = lVar7;
    puVar11 = puVar9;
    if (uVar3 != uVar10) {
      do {
        *puVar11 = 0x40;
        lVar12 = lVar12 + -1;
        puVar11 = puVar11 + 1;
      } while (lVar12 != 0);
    }
    puVar5[(ulong)local_34 + lVar8] = (CONCAT22(uVar2,uVar2) & 0xffff00) + 0x40;
    puVar5[(ulong)local_38 + lVar8] = (CONCAT22(uVar1,uVar1) & 0xffff00) + 0x40;
    if (uVar4 < uVar10) {
      puVar11 = puVar9 + uVar4;
      lVar12 = (ulong)(uVar3 - 1) + 1;
      do {
        *puVar11 = 0x80;
        puVar11 = puVar11 + 1;
        lVar12 = lVar12 + -1;
      } while (lVar12 != 0);
    }
    lVar8 = lVar8 + uVar6;
    puVar9 = puVar9 + uVar6;
    param_2 = param_2 + 2;
    param_5 = param_5 - 1;
  } while (param_5 != 0);
  return;
}
