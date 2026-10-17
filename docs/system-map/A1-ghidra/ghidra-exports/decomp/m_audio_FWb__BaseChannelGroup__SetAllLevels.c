// m_audio_FWb::BaseChannelGroup::SetAllLevels
// Entry: 000215b0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::BaseChannelGroup::SetAllLevels(int) */

void m_audio_FWb::BaseChannelGroup::SetAllLevels(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 in_register_0000003c;
  long *plVar3;
  int iVar4;
  
  plVar3 = (long *)CONCAT44(in_register_0000003c,param_1);
  iVar1 = (**(code **)(*plVar3 + 0x10))(plVar3);
  if (iVar1 != 0) {
    iVar4 = 0;
    do {
      puVar2 = (undefined4 *)(**(code **)(*plVar3 + 0x18))(plVar3,iVar4);
      *puVar2 = 0;
      iVar4 = iVar4 + 1;
    } while (iVar1 != iVar4);
  }
  return;
}
