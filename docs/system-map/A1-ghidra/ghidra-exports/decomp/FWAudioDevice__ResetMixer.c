// m_audio_b_FWAudioDevice::ResetMixer
// Entry: 00002660
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAudioDevice::ResetMixer(OSObject*) */

undefined8 m_audio_b_FWAudioDevice::ResetMixer(OSObject *param_1)

{
  (**(code **)(*(long *)param_1 + 0xb08))(param_1);
  (**(code **)(*(long *)param_1 + 0xb00))(param_1);
  if (*(m_audio_b_FWPropertyDict **)(param_1 + 0xf8) != (m_audio_b_FWPropertyDict *)0x0) {
    m_audio_b_FWPropertyDict::SendAllPropertiesToUserClients
              (*(m_audio_b_FWPropertyDict **)(param_1 + 0xf8));
  }
  return 0;
}
