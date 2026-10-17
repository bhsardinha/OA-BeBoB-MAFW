// com_m_audio_FW1814Device::GetSupportedOutputSampleRates
// Entry: 0000cf14
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FW1814Device::GetSupportedOutputSampleRates() const */

undefined1 * com_m_audio_FW1814Device::GetSupportedOutputSampleRates(void)

{
  return &kSupportedRatesArray;
}
