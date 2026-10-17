// m_audio_FWb::ConcreteChannelGroup<10u>::ChannelAtIndex
// Entry: 0000ad84
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<10u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<10u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<10u>::ChannelAtIndex(ConcreteChannelGroup<10u> *this,uint param_1)

{
  if (param_1 < 10) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
