// com_m_audio_FWAudiophileDevice::ResendCurrentMixerSettings
// Entry: 0000b00e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::ResendCurrentMixerSettings() */

void __thiscall
com_m_audio_FWAudiophileDevice::ResendCurrentMixerSettings(com_m_audio_FWAudiophileDevice *this)

{
  (*(code *)PTR_ResendCurrentMixerSettings_0003b4b0)(this);
  (*(code *)PTR_SendToDevice_000470f0)(this + 0x2d0);
  (*(code *)PTR_SendToDevice_00048dd0)(this + 0x468);
  (*(code *)PTR_SendToDevice_00048d40)(this + 0x530);
  (*(code *)PTR_SendToDevice_00048e60)(this + 0x808);
  return;
}
