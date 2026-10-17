// m_audio_FWb::GroupChannelGroup::ChannelAtIndex
// Entry: 0001f1b6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::GroupChannelGroup::ChannelAtIndex(unsigned int) const */

void __thiscall m_audio_FWb::GroupChannelGroup::ChannelAtIndex(GroupChannelGroup *this,uint param_1)

{
  uint uVar1;
  int iVar2;
  long lVar3;
  long *plVar4;
  
  plVar4 = *(long **)(this + 0x48);
  if (plVar4 != (long *)0x0) {
    uVar1 = (**(code **)(*plVar4 + 0x10))(plVar4);
    plVar4 = *(long **)(this + 0x48);
    lVar3 = *plVar4;
    if (param_1 < uVar1) goto LAB_0001f1df;
    iVar2 = (**(code **)(lVar3 + 0x10))();
    param_1 = param_1 - iVar2;
  }
  if (*(long **)(this + 0x50) != (long *)0x0) {
    uVar1 = (**(code **)(**(long **)(this + 0x50) + 0x10))();
    plVar4 = *(long **)(this + 0x50);
    lVar3 = *plVar4;
    if (param_1 < uVar1) goto LAB_0001f1df;
    iVar2 = (**(code **)(lVar3 + 0x10))();
    param_1 = param_1 - iVar2;
  }
  if (*(long **)(this + 0x58) != (long *)0x0) {
    uVar1 = (**(code **)(**(long **)(this + 0x58) + 0x10))();
    if (param_1 < uVar1) {
      plVar4 = *(long **)(this + 0x58);
      lVar3 = *plVar4;
LAB_0001f1df:
                    /* WARNING: Could not recover jumptable at 0x0001f1e9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(lVar3 + 0x18))(plVar4,param_1);
      return;
    }
  }
                    /* WARNING: Subroutine does not return */
  _panic(
        "\"no channel at index\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/../Shared/KernelPropertyIPC/TChannelGroup.h:444"
        );
}
