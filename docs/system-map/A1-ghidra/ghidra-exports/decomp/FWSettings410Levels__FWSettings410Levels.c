// FWSettings410Levels::FWSettings410Levels
// Entry: 00015642
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings410Levels::FWSettings410Levels() */

void __thiscall FWSettings410Levels::FWSettings410Levels(FWSettings410Levels *this)

{
  *(undefined ***)this = &PTR_SendToDevice_00048e60;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x10));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x70));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xb0));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xf0));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x130));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x310));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x4f0));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x6d0));
  *(undefined ***)(this + 0x8f0) = &PTR__TFunctor_0003c460;
  *(undefined8 *)(this + 0x8f8) = 0;
  *(undefined ***)(this + 0x900) = &PTR__TFunctor_0003c490;
  *(undefined8 *)(this + 0x908) = 0;
  this[0x8e2] = (FWSettings410Levels)0x0;
  *(undefined ***)(this + 0x8d0) = &PTR__GroupChannelGroup_00047fc0;
  *(undefined ***)(this + 0x950) = &PTR__TFunctor_0003c460;
  *(undefined8 *)(this + 0x958) = 0;
  *(undefined ***)(this + 0x960) = &PTR__TFunctor_0003c490;
  *(undefined8 *)(this + 0x968) = 0;
  this[0x942] = (FWSettings410Levels)0x0;
  *(undefined ***)(this + 0x930) = &PTR__GroupChannelGroup_00047fc0;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x990));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x9f0));
  *(undefined ***)this = &PTR_SendToDevice_0003c3c0;
  return;
}
