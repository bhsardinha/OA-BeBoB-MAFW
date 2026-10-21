// com_m_audio_FW410Device::GetSupportedInputSampleRates
// Entry: 00014032
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::GetSupportedInputSampleRates() const */

undefined1 * com_m_audio_FW410Device::GetSupportedInputSampleRates(void)

{
  return &kSupportedInputRatesArray;
}
