// m_audio_FWb::ConcreteChannelGroup<18u>::ChannelAtIndex
// Entry: 0000c934
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<18u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<18u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<18u>::ChannelAtIndex(ConcreteChannelGroup<18u> *this,uint param_1)

{
  if (param_1 < 0x12) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
