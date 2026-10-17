// m_audio_FWb::ConcreteChannelGroup<2u>::ChannelAtIndex
// Entry: 0000ac9e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<2u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<2u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<2u>::ChannelAtIndex(ConcreteChannelGroup<2u> *this,uint param_1)

{
  if (param_1 < 2) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
