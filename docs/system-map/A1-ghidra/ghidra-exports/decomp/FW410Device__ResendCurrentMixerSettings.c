// com_m_audio_FW410Device::ResendCurrentMixerSettings
// Entry: 00014548
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::ResendCurrentMixerSettings() */

void __thiscall com_m_audio_FW410Device::ResendCurrentMixerSettings(com_m_audio_FW410Device *this)

{
  (*(code *)PTR_ResendCurrentMixerSettings_0003b4b0)(this);
  (*(code *)PTR_SendToDevice_00048dd0)(this + 0x2d8);
  (*(code *)PTR_SendToDevice_00048d40)(this + 0x3a0);
  FWSettings410Levels::SendToDevice((FWSettings410Levels *)(this + 0x678));
  SetHeadphoneSource(this,*(uint *)(this + 0x1174));
  return;
}
