// m_audio_FWb::ConcreteChannelGroup<4u>::ChannelAtIndex
// Entry: 00016a2e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<4u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<4u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<4u>::ChannelAtIndex(ConcreteChannelGroup<4u> *this,uint param_1)

{
  if (param_1 < 4) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
