// FWRev2AudioDevice::PauseReceptionOfControlPackets
// Entry: 0001310c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::PauseReceptionOfControlPackets() */

void __thiscall FWRev2AudioDevice::PauseReceptionOfControlPackets(FWRev2AudioDevice *this)

{
  if (*(m_audio_b_FWHSCI **)(this + 0x2c8) != (m_audio_b_FWHSCI *)0x0) {
    m_audio_b_FWHSCI::Pause(*(m_audio_b_FWHSCI **)(this + 0x2c8));
  }
  m_audio_b_FWAudioDevice::ClearPeakMeters((m_audio_b_FWAudioDevice *)this);
  return;
}
