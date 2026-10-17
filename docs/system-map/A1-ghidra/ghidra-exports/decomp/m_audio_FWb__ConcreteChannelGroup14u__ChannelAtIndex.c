// m_audio_FWb::ConcreteChannelGroup<14u>::ChannelAtIndex
// Entry: 00013dd6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<14u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<14u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<14u>::ChannelAtIndex(ConcreteChannelGroup<14u> *this,uint param_1)

{
  if (param_1 < 0xe) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
