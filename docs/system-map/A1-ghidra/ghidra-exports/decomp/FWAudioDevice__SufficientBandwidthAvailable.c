// m_audio_b_FWAudioDevice::SufficientBandwidthAvailable
// Entry: 000022e8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAudioDevice::SufficientBandwidthAvailable() */

byte __thiscall m_audio_b_FWAudioDevice::SufficientBandwidthAvailable(m_audio_b_FWAudioDevice *this)

{
  uint uVar1;
  
  MaximumBandwidthBytesSupported(this);
  uVar1 = GetBandwidthBytesAtSampleRate((uint)this);
  return -(uVar1 < 0x186b) & 1;
}
