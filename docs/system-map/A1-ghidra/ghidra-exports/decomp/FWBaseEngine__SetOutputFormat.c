// m_audio_b_FWBaseEngine::SetOutputFormat
// Entry: 0000427e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::SetOutputFormat(m_audio_b_FWAudioDevice::DeviceChannels&, unsigned int)
    */

void m_audio_b_FWBaseEngine::SetOutputFormat(DeviceChannels *param_1,uint param_2)

{
  SetStreamFormat((IOAudioStream *)param_1,*(DeviceChannels **)(param_1 + 0x218),param_2);
  return;
}
