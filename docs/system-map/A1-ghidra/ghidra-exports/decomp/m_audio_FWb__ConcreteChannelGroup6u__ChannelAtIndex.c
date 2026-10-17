// m_audio_FWb::ConcreteChannelGroup<6u>::ChannelAtIndex
// Entry: 0000ae6a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<6u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<6u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<6u>::ChannelAtIndex(ConcreteChannelGroup<6u> *this,uint param_1)

{
  if (param_1 < 6) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
