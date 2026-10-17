// m_audio_FWb::ConcreteChannelGroup<22u>::ChannelAtIndex
// Entry: 0000c768
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<22u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<22u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<22u>::ChannelAtIndex(ConcreteChannelGroup<22u> *this,uint param_1)

{
  if (param_1 < 0x16) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}
