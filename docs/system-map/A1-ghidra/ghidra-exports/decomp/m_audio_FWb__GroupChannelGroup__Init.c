// m_audio_FWb::GroupChannelGroup::Init
// Entry: 00021720
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::GroupChannelGroup::Init(m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*) */

void __thiscall
m_audio_FWb::GroupChannelGroup::Init
          (GroupChannelGroup *this,BaseChannelGroup *param_1,BaseChannelGroup *param_2,
          BaseChannelGroup *param_3)

{
  this[0x12] = (GroupChannelGroup)0x1;
  *(undefined8 *)(this + 8) = 0;
  this[0x10] = (GroupChannelGroup)0x0;
  this[0x11] = (GroupChannelGroup)0x0;
  *(BaseChannelGroup **)(this + 0x48) = param_1;
  *(BaseChannelGroup **)(this + 0x50) = param_2;
  *(BaseChannelGroup **)(this + 0x58) = param_3;
  if (param_1 != (BaseChannelGroup *)0x0) {
    *(GroupChannelGroup **)(param_1 + 8) = this;
    param_2 = *(BaseChannelGroup **)(this + 0x50);
  }
  if (param_2 != (BaseChannelGroup *)0x0) {
    *(GroupChannelGroup **)(param_2 + 8) = this;
  }
  if (*(long *)(this + 0x58) != 0) {
    *(GroupChannelGroup **)(*(long *)(this + 0x58) + 8) = this;
  }
  return;
}
