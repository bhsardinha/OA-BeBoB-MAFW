// FWRev2AudioDevice::ResumeReceptionOfControlPackets
// Entry: 000130f4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::ResumeReceptionOfControlPackets() */

void __thiscall FWRev2AudioDevice::ResumeReceptionOfControlPackets(FWRev2AudioDevice *this)

{
  if (*(m_audio_b_FWHSCI **)(this + 0x2c8) == (m_audio_b_FWHSCI *)0x0) {
    return;
  }
  m_audio_b_FWHSCI::Resume(*(m_audio_b_FWHSCI **)(this + 0x2c8));
  return;
}
