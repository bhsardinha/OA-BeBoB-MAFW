// com_m_audio_FWAudiophileDevice::ResetToFactorySettings
// Entry: 0000b078
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::ResetToFactorySettings() */

void __thiscall
com_m_audio_FWAudiophileDevice::ResetToFactorySettings(com_m_audio_FWAudiophileDevice *this)

{
  (*(code *)PTR_ResetToFactorySettings_0003b4b8)(this);
  (*(code *)PTR_ResetToFactorySettings_000470f8)(this + 0x2d0);
  (*(code *)PTR_ResetToFactorySettings_00048dd8)(this + 0x468);
  (*(code *)PTR_ResetToFactorySettings_00048d48)(this + 0x530);
  (*(code *)PTR_ResetToFactorySettings_00048e68)(this + 0x808);
  return;
}
