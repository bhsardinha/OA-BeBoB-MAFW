// m_audio_b_FWIsochChannel::start
// Entry: 00006eb0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWIsochChannel::start() */

undefined8 __thiscall m_audio_b_FWIsochChannel::start(m_audio_b_FWIsochChannel *this)

{
  undefined8 uVar1;
  
  if (this[0x78] == (m_audio_b_FWIsochChannel)0x0) {
    uVar1 = (**(code **)(PTR_vtable_000300c8 + 0x168))(this);
    this[0x78] = (m_audio_b_FWIsochChannel)((int)uVar1 == 0);
  }
  else {
    uVar1 = 0xe00002c2;
  }
  return uVar1;
}
