// m_audio_FWb::BaseChannelGroup::SetAllLinked
// Entry: 000216d6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::BaseChannelGroup::SetAllLinked(bool) */

void m_audio_FWb::BaseChannelGroup::SetAllLinked(bool param_1)

{
  int iVar1;
  long lVar2;
  undefined7 in_register_00000039;
  long *plVar3;
  int iVar4;
  
  plVar3 = (long *)CONCAT71(in_register_00000039,param_1);
  iVar1 = (**(code **)(*plVar3 + 0x10))(plVar3);
  if (iVar1 != 0) {
    iVar4 = 0;
    do {
      lVar2 = (**(code **)(*plVar3 + 0x18))(plVar3,iVar4);
      *(undefined8 *)(lVar2 + 0x18) = 1;
      iVar4 = iVar4 + 1;
    } while (iVar1 != iVar4);
  }
  return;
}
