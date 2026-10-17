// m_audio_b_FWAVCAudioDevice::SystemWokeUp
// Entry: 00022d76
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2b)

/* m_audio_b_FWAVCAudioDevice::SystemWokeUp() */

void __thiscall m_audio_b_FWAVCAudioDevice::SystemWokeUp(m_audio_b_FWAVCAudioDevice *this)

{
  _IOSleep(500);
  if (*(long *)(this + 0xe8) == 0) {
    return;
  }
  (**(code **)(*(long *)this + 0xb00))(this);
  (**(code **)(*(long *)this + 0xb58))(this);
  (**(code **)(**(long **)(this + 0xe8) + 0xcb8))();
  (**(code **)(*(long *)this + 0xb90))(this);
                    /* WARNING: Could not recover jumptable at 0x00022de1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(this + 0xe8) + 0xa58))();
  return;
}
