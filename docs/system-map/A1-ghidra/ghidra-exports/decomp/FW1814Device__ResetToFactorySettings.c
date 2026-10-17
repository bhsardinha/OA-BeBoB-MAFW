// com_m_audio_FW1814Device::ResetToFactorySettings
// Entry: 0000d190
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ResetToFactorySettings() */

void __thiscall com_m_audio_FW1814Device::ResetToFactorySettings(com_m_audio_FW1814Device *this)

{
  (*(code *)PTR_ResetToFactorySettings_0003c1a8)(this);
  (*(code *)PTR_ResetToFactorySettings_000470f8)(this + 0x2d8);
  (*(code *)PTR_ResetToFactorySettings_00048dd8)(this + 0x470);
  FWSettingsAuxSend::ResetToFactorySettings((FWSettingsAuxSend *)(this + 0xf98));
  FWSettingsLevels::ResetToFactorySettings((FWSettingsLevels *)(this + 0x538));
  *(undefined4 *)(this + 0x144) = 0x40000003;
  return;
}
