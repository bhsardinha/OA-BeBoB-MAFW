// com_m_audio_FWAudiophileDevice::GetSupportedOutputSampleRates
// Entry: 0000afd0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::GetSupportedOutputSampleRates() const */

undefined1 * com_m_audio_FWAudiophileDevice::GetSupportedOutputSampleRates(void)

{
  return &kSupportedSampleRatesArray;
}
