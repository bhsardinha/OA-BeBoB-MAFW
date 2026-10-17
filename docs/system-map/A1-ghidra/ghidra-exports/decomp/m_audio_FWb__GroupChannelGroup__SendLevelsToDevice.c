// m_audio_FWb::GroupChannelGroup::SendLevelsToDevice
// Entry: 0001ee16
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::GroupChannelGroup::SendLevelsToDevice(int*) */

void m_audio_FWb::GroupChannelGroup::SendLevelsToDevice(int *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(param_1 + 0x12);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x28))(plVar1,0);
  }
  plVar1 = *(long **)(param_1 + 0x14);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x28))(plVar1,0);
  }
  plVar1 = *(long **)(param_1 + 0x16);
  if (plVar1 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0001ee63. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x28))(plVar1,0);
  return;
}
