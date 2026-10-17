// com_m_audio_FW1814Device::ResendCurrentMixerSettings
// Entry: 0000d014
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ResendCurrentMixerSettings() */

void __thiscall com_m_audio_FW1814Device::ResendCurrentMixerSettings(com_m_audio_FW1814Device *this)

{
  (*(code *)PTR_ResendCurrentMixerSettings_0003c1a0)(this);
  (*(code *)PTR_SendToDevice_000470f0)(this + 0x2d8);
  (*(code *)PTR_SendToDevice_00048dd0)(this + 0x470);
  FWSettingsAuxSend::SendToDevice((FWSettingsAuxSend *)(this + 0xf98));
  FWSettingsLevels::SendToDevice((FWSettingsLevels *)(this + 0x538));
  return;
}
