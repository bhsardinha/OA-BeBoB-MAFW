// com_m_audio_FW410Device::ResetToFactorySettings
// Entry: 000142de
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::ResetToFactorySettings() */

void __thiscall com_m_audio_FW410Device::ResetToFactorySettings(com_m_audio_FW410Device *this)

{
  (*(code *)PTR_ResetToFactorySettings_0003b4b8)(this);
  (*(code *)PTR_ResetToFactorySettings_00048dd8)(this + 0x2d8);
  (*(code *)PTR_ResetToFactorySettings_00048d48)(this + 0x3a0);
  FWSettingsLevels::ResetToFactorySettings((FWSettingsLevels *)(this + 0x678));
  *(undefined4 *)(this + 0x1174) = 1;
  return;
}
