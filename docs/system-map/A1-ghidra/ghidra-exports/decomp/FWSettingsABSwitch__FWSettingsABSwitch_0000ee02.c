// FWSettingsABSwitch::FWSettingsABSwitch
// Entry: 0000ee02
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::FWSettingsABSwitch() */

void __thiscall FWSettingsABSwitch::FWSettingsABSwitch(FWSettingsABSwitch *this)

{
  *(undefined ***)this = &PTR_SendToDevice_000470f0;
  *(undefined ***)(this + 0x10) = &PTR__TFunctor_000367b0;
  *(undefined8 *)(this + 0x18) = 0;
  *(undefined ***)(this + 0x20) = &PTR__TFunctor_00036750;
  *(undefined8 *)(this + 0x28) = 0;
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x30));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x70));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xd8));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x118));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x158));
  return;
}
