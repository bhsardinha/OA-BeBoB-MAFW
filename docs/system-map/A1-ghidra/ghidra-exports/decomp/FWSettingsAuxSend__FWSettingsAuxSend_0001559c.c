// FWSettingsAuxSend::FWSettingsAuxSend
// Entry: 0001559c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::FWSettingsAuxSend() */

void __thiscall FWSettingsAuxSend::FWSettingsAuxSend(FWSettingsAuxSend *this)

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
  return;
}
