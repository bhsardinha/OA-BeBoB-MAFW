// m_audio_b_FWAudioDevice::setProperties
// Entry: 0000299c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAudioDevice::setProperties(OSObject*) */

void __thiscall
m_audio_b_FWAudioDevice::setProperties(m_audio_b_FWAudioDevice *this,OSObject *param_1)

{
  if (*(m_audio_b_FWPropertyDict **)(this + 0xf8) != (m_audio_b_FWPropertyDict *)0x0) {
    m_audio_b_FWPropertyDict::SetProperties(*(m_audio_b_FWPropertyDict **)(this + 0xf8),param_1);
    return;
  }
  (*(code *)PTR_setProperties_00047590)();
  return;
}
