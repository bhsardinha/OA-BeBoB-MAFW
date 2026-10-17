// FWSettingsABSwitch::ResetToFactorySettings
// Entry: 0001df88
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::ResetToFactorySettings() */

undefined8 __thiscall FWSettingsABSwitch::ResetToFactorySettings(FWSettingsABSwitch *this)

{
  m_audio_b_FWBaseArrayProp *this_00;
  
  *(undefined4 *)(this + 0x104) = 0;
  *(undefined4 *)(this + 0xd0) = 0;
  this_00 = (m_audio_b_FWBaseArrayProp *)(this + 0x70);
  m_audio_b_FWBaseArrayProp::Set(this_00,0,1);
  m_audio_b_FWBaseArrayProp::Set(this_00,1,2);
  m_audio_b_FWBaseArrayProp::Set(this_00,2,3);
  m_audio_b_FWBaseArrayProp::Set(this_00,3,4);
  *(undefined4 *)(this + 0x144) = 1;
  *(undefined4 *)(this + 0x184) = 1;
  return 0;
}
