// m_audio_b_FWAudioDevice::WakeAfterSleep
// Entry: 00001e7c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2b)

/* m_audio_b_FWAudioDevice::WakeAfterSleep() */

void __thiscall m_audio_b_FWAudioDevice::WakeAfterSleep(m_audio_b_FWAudioDevice *this)

{
                    /* WARNING: Could not recover jumptable at 0x00001e84. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xb10))();
  return;
}
