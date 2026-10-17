// m_audio_b_FWAudioDevice::BusReset
// Entry: 00001d3c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e8)

/* m_audio_b_FWAudioDevice::BusReset() */

void __thiscall m_audio_b_FWAudioDevice::BusReset(m_audio_b_FWAudioDevice *this)

{
  if (*(long **)(this + 0xf0) == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00001d52. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(this + 0xf0) + 0x130))();
  return;
}
