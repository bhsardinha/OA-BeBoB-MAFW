// m_audio_b_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket
// Entry: 0002525c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_b_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket(int*, unsigned int) */

void m_audio_b_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket(int *param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  long lVar4;
  int *piVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  undefined4 in_register_00000034;
  
  iVar1 = param_1[0x19];
  iVar7 = param_1[0x1a] + iVar1;
  iVar2 = param_1[299];
  if (iVar2 * iVar7 != 0) {
    uVar6 = 0;
    uVar8 = uVar6;
    uVar9 = uVar6;
    do {
      uVar3 = *(undefined4 *)(CONCAT44(in_register_00000034,param_2) + (ulong)(iVar1 + uVar6) * 4);
      if (((char)uVar3 == -0x7f) && (uVar9 < *(uint *)(*(long *)(param_1 + 0x20) + 0x5c))) {
        lVar4 = *(long *)(*(long *)(param_1 + 0x20) + 0x70);
        piVar5 = *(int **)(lVar4 + (ulong)uVar9 * 0x28);
        *(char *)((long)piVar5 + (long)(*piVar5 % 0x200) + 4) = (char)((uint)uVar3 >> 8);
        piVar5 = *(int **)(lVar4 + (ulong)uVar9 * 0x28);
        *piVar5 = *piVar5 + 1;
      }
      uVar10 = uVar9 + 1;
      uVar9 = 0;
      if (uVar10 < 8) {
        uVar9 = uVar10;
      }
      uVar6 = uVar6 + iVar7;
      uVar8 = uVar8 + iVar7;
    } while (uVar8 < (uint)(iVar2 * iVar7));
  }
  return;
}
