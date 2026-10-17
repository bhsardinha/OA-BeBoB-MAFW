// FWSettingsLevels::FWSettingsLevels
// Entry: 00017c80
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::FWSettingsLevels() */

void __thiscall FWSettingsLevels::FWSettingsLevels(FWSettingsLevels *this)

{
  *(undefined ***)this = &PTR_SendToDevice_00048e60;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x10));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x70));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xb0));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xf0));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x130));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x310));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x4f0));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x6d0));
  *(undefined ***)(this + 0x8f0) = &PTR__TFunctor_0003ecf0;
  *(undefined8 *)(this + 0x8f8) = 0;
  *(undefined ***)(this + 0x900) = &PTR__TFunctor_0003ed20;
  *(undefined8 *)(this + 0x908) = 0;
  this[0x8e2] = (FWSettingsLevels)0x0;
  *(undefined ***)(this + 0x8d0) = &PTR__GroupChannelGroup_00047fc0;
  *(undefined ***)(this + 0x950) = &PTR__TFunctor_0003ecf0;
  *(undefined8 *)(this + 0x958) = 0;
  *(undefined ***)(this + 0x960) = &PTR__TFunctor_0003ed20;
  *(undefined8 *)(this + 0x968) = 0;
  this[0x942] = (FWSettingsLevels)0x0;
  *(undefined ***)(this + 0x930) = &PTR__GroupChannelGroup_00047fc0;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x990));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x9f0));
  return;
}
