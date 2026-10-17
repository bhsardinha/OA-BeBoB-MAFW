// FWSettings1814AuxSend::FWSettings1814AuxSend
// Entry: 0000efda
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings1814AuxSend::FWSettings1814AuxSend() */

void __thiscall FWSettings1814AuxSend::FWSettings1814AuxSend(FWSettings1814AuxSend *this)

{
  *(undefined ***)this = &PTR_SendToDevice_00048d40;
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x20));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x98));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x110));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x188));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x200));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x260));
  *(undefined ***)this = &PTR_SendToDevice_00036670;
  return;
}
