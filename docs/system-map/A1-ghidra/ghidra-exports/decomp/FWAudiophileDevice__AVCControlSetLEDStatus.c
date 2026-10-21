// com_m_audio_FWAudiophileDevice::AVCControlSetLEDStatus
// Entry: 0000b7d0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::AVCControlSetLEDStatus(unsigned int) */

void com_m_audio_FWAudiophileDevice::AVCControlSetLEDStatus(uint param_1)

{
  m_audio_b_FWAVCAudioDevice::AVCVendorCommand((uchar)param_1,'\x02','\0');
  return;
}
