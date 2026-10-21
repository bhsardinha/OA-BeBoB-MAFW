// com_m_audio_FWAudiophileDevice::GetSupportedInputSampleRates
// Entry: 0000afc2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::GetSupportedInputSampleRates() const */

undefined1 * com_m_audio_FWAudiophileDevice::GetSupportedInputSampleRates(void)

{
  return &kSupportedSampleRatesArray;
}
