// m_audio_FWb::BaseChannelGroup::AdjustedLevels
// Entry: 0001f300
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::BaseChannelGroup::AdjustedLevels() */

void * __thiscall m_audio_FWb::BaseChannelGroup::AdjustedLevels(BaseChannelGroup *this)

{
  BaseChannelGroup BVar1;
  BaseChannelGroup BVar2;
  bool bVar3;
  uint uVar4;
  undefined4 uVar5;
  void *pvVar6;
  Channel *pCVar7;
  ulong uVar8;
  BaseChannelGroup *local_40;
  
  local_40 = *(BaseChannelGroup **)(this + 8);
  if (local_40 == (BaseChannelGroup *)0x0) {
    *(BaseChannelGroup **)(this + 8) = this;
    local_40 = this;
  }
  uVar4 = (**(code **)(*(long *)this + 0x10))(this);
  pvVar6 = operator_new__((ulong)uVar4 * 4);
  BVar1 = this[0x10];
  BVar2 = this[0x11];
  bVar3 = (bool)Solo(local_40);
  if ((pvVar6 != (void *)0x0) && (uVar4 != 0)) {
    uVar8 = 0;
    do {
      pCVar7 = (Channel *)(**(code **)(*(long *)this + 0x18))(this,uVar8 & 0xffffffff);
      uVar5 = AdjustedLevelForChannel(this,pCVar7,bVar3,(bool)BVar2,(bool)BVar1);
      *(undefined4 *)((long)pvVar6 + uVar8 * 4) = uVar5;
      uVar8 = uVar8 + 1;
    } while (uVar4 != uVar8);
  }
  return pvVar6;
}
