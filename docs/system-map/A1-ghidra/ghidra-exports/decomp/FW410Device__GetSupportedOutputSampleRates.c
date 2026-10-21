// com_m_audio_FW410Device::GetSupportedOutputSampleRates
// Entry: 00014024
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::GetSupportedOutputSampleRates() const */

undefined1 * com_m_audio_FW410Device::GetSupportedOutputSampleRates(void)

{
  return &kSupportedInputRatesArray;
}
