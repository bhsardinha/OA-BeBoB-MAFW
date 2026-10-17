// m_audio_b_FWIsochChannel::stop
// Entry: 00006e7c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::stop() */

undefined8 __thiscall m_audio_b_FWIsochChannel::stop(m_audio_b_FWIsochChannel *this)

{
  undefined8 uVar1;
  
  if (this[0x78] == (m_audio_b_FWIsochChannel)0x0) {
    uVar1 = 0xe00002c2;
  }
  else {
    uVar1 = (**(code **)(PTR_vtable_000300c8 + 0x170))(this);
    this[0x78] = (m_audio_b_FWIsochChannel)0x0;
  }
  return uVar1;
}
