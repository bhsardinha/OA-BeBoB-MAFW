// com_m_audio_FW410Device::GetDeviceChannelsForSampleRate
// Entry: 00014006
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::GetDeviceChannelsForSampleRate(unsigned int) const */

undefined1 * __thiscall
com_m_audio_FW410Device::GetDeviceChannelsForSampleRate(com_m_audio_FW410Device *this,uint param_1)

{
  undefined1 *puVar1;
  
  puVar1 = &sDeviceChannels;
  if (96000 < param_1) {
    puVar1 = &sDeviceChannels192;
  }
  return puVar1;
}
