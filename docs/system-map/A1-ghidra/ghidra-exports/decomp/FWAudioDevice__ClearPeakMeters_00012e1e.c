// m_audio_b_FWAudioDevice::ClearPeakMeters
// Entry: 00012e1e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWAudioDevice::ClearPeakMeters() */

void __thiscall m_audio_b_FWAudioDevice::ClearPeakMeters(m_audio_b_FWAudioDevice *this)

{
  uint uVar1;
  
  if (*(long *)(this + 0x108) == 0) {
    return;
  }
  uVar1 = (**(code **)(*(long *)this + 0xaf0))(this);
  _bzero(*(void **)(this + 0x108),(ulong)uVar1 << 2);
  uVar1 = (**(code **)(*(long *)this + 0xaf0))(this);
  _bzero(*(void **)(this + 0x110),(ulong)uVar1 << 2);
  return;
}
