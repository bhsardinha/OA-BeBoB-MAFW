// m_audio_FWb::BaseChannelGroup::AdjustedLevelForChannel
// Entry: 0001ee66
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::BaseChannelGroup::AdjustedLevelForChannel(m_audio_FWb::Channel&, bool, bool, bool)
    */

int __thiscall
m_audio_FWb::BaseChannelGroup::AdjustedLevelForChannel
          (BaseChannelGroup *this,Channel *param_1,bool param_2,bool param_3,bool param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  if ((param_3) || ((param_2 && (*(long *)(param_1 + 0x10) == 0)))) {
    iVar1 = *(int *)(this + 0x14);
  }
  else {
    if (*(long *)(param_1 + 8) == 0) {
      iVar1 = *(int *)(param_1 + 4) + *(int *)param_1;
      iVar3 = *(int *)(this + 0x14);
      iVar2 = iVar3;
      if ((iVar3 <= iVar1) && (iVar2 = iVar1, *(int *)(this + 0x18) < iVar1)) {
        iVar2 = *(int *)(this + 0x18);
      }
    }
    else {
      iVar3 = *(int *)(this + 0x14);
      iVar2 = iVar3;
    }
    iVar1 = iVar2;
    if ((param_4) && (iVar1 = iVar2 + -0x14, iVar2 + -0x14 < iVar3)) {
      iVar1 = iVar3;
    }
  }
  return iVar1;
}
