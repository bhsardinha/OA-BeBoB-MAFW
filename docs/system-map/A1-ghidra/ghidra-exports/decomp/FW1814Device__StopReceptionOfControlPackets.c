// com_m_audio_FW1814Device::StopReceptionOfControlPackets
// Entry: 0000da7e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::StopReceptionOfControlPackets() */

void __thiscall
com_m_audio_FW1814Device::StopReceptionOfControlPackets(com_m_audio_FW1814Device *this)

{
  if (*(m_audio_b_FWHSCI **)(this + 0x1270) != (m_audio_b_FWHSCI *)0x0) {
    m_audio_b_FWHSCI::Stop(*(m_audio_b_FWHSCI **)(this + 0x1270));
  }
  m_audio_b_FWAudioDevice::ClearPeakMeters((m_audio_b_FWAudioDevice *)this);
  return;
}
