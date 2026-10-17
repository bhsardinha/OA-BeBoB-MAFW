// m_audio_b_FWBaseEngine::StartMIDI
// Entry: 00004118
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_b_FWBaseEngine::StartMIDI() */

undefined8 __thiscall m_audio_b_FWBaseEngine::StartMIDI(m_audio_b_FWBaseEngine *this)

{
  undefined8 uVar1;
  
  if (this[0x298] == (m_audio_b_FWBaseEngine)0x0) {
    if ((*(int *)(this + 0x29c) == 0) && (*(int *)(this + 0x2a0) == 0)) {
      uVar1 = 0xe00002c7;
    }
    else {
      uVar1 = (**(code **)(*(long *)this + 0xc18))(this);
      if ((int)uVar1 == 0) {
        this[0x298] = (m_audio_b_FWBaseEngine)0x1;
      }
    }
  }
  else {
    uVar1 = 0xe00002d2;
  }
  return uVar1;
}
