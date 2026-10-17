// FWSettingsLevels::ResetToFactorySettings
// Entry: 00021bc4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::ResetToFactorySettings() */

undefined8 __thiscall FWSettingsLevels::ResetToFactorySettings(FWSettingsLevels *this)

{
  uint uVar1;
  bool bVar2;
  int iVar3;
  long *plVar4;
  uint uVar5;
  
  *(undefined4 *)(this + 0x11c) = 1;
  *(undefined1 *)(*(long *)(this + 0x8c0) + 0x11) = 0;
  if (this[0x942] != (FWSettingsLevels)0x0) {
    iVar3 = (int)(this + 0x930);
    m_audio_FWb::BaseChannelGroup::SetAllLevels(iVar3);
    m_audio_FWb::BaseChannelGroup::SetAllBalance(iVar3);
    bVar2 = SUB81(this + 0x930,0);
    m_audio_FWb::BaseChannelGroup::SetAllMute(bVar2);
    m_audio_FWb::BaseChannelGroup::SetAllSolo(bVar2);
    m_audio_FWb::BaseChannelGroup::SetAllLinked(bVar2);
  }
  if (this[0x8e2] != (FWSettingsLevels)0x0) {
    iVar3 = (int)(this + 0x8d0);
    m_audio_FWb::BaseChannelGroup::SetAllLevels(iVar3);
    m_audio_FWb::BaseChannelGroup::SetAllBalance(iVar3);
    bVar2 = SUB81(this + 0x8d0,0);
    m_audio_FWb::BaseChannelGroup::SetAllMute(bVar2);
    m_audio_FWb::BaseChannelGroup::SetAllSolo(bVar2);
    m_audio_FWb::BaseChannelGroup::SetAllLinked(bVar2);
  }
  plVar4 = *(long **)(this + 0x8c0);
  if (plVar4 != (long *)0x0) {
    if (*(int *)(this + 0x3c) != 0) {
      uVar5 = 0;
      do {
        m_audio_b_FWBaseArrayProp::Set((m_audio_b_FWBaseArrayProp *)(this + 0x10),uVar5,0xffffff01);
        m_audio_b_FWBaseArrayProp::Set((m_audio_b_FWBaseArrayProp *)(this + 0x10),uVar5 + 1,0xff);
        uVar5 = uVar5 + 2;
      } while (uVar5 < *(uint *)(this + 0x3c));
      plVar4 = *(long **)(this + 0x8c0);
    }
    uVar5 = (**(code **)(*plVar4 + 0x10))();
    if (1 < uVar5) {
      uVar5 = 0;
      do {
        m_audio_b_FWBaseArrayProp::Set((m_audio_b_FWBaseArrayProp *)(this + 0x990),uVar5,0);
        uVar1 = (**(code **)(**(long **)(this + 0x8c0) + 0x10))();
        uVar5 = uVar5 + 1;
      } while (uVar5 < uVar1 >> 1);
    }
  }
  if (*(long **)(this + 0x8b0) != (long *)0x0) {
    uVar5 = (**(code **)(**(long **)(this + 0x8b0) + 0x10))();
    if (1 < uVar5) {
      uVar5 = 0;
      do {
        m_audio_b_FWBaseArrayProp::Set
                  ((m_audio_b_FWBaseArrayProp *)(this + 0x9f0),uVar5,
                   (uint)(1L << ((byte)uVar5 & 0x3f)));
        uVar1 = (**(code **)(**(long **)(this + 0x8b0) + 0x10))();
        uVar5 = uVar5 + 1;
      } while (uVar5 < uVar1 >> 1);
    }
  }
  return 0;
}
