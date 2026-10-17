// m_audio_FWb::GroupChannelGroup::Count
// Entry: 0001edb4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::GroupChannelGroup::Count() const */

int __thiscall m_audio_FWb::GroupChannelGroup::Count(GroupChannelGroup *this)

{
  long *plVar1;
  int iVar2;
  int iVar3;
  
  plVar1 = *(long **)(this + 0x48);
  if (plVar1 == (long *)0x0) {
    iVar3 = 0;
  }
  else {
    iVar3 = (**(code **)(*plVar1 + 0x10))(plVar1);
  }
  if (*(long **)(this + 0x50) != (long *)0x0) {
    iVar2 = (**(code **)(**(long **)(this + 0x50) + 0x10))();
    iVar3 = iVar3 + iVar2;
  }
  if (*(long **)(this + 0x58) != (long *)0x0) {
    iVar2 = (**(code **)(**(long **)(this + 0x58) + 0x10))();
    iVar3 = iVar2 + iVar3;
  }
  return iVar3;
}
