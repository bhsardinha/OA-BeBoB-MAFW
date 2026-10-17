// com_m_audio_FWEngine2::StopMIDI
// Entry: 00011b86
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWEngine2::StopMIDI() */

undefined8 __thiscall com_m_audio_FWEngine2::StopMIDI(com_m_audio_FWEngine2 *this)

{
  if (this[0x298] != (com_m_audio_FWEngine2)0x0) {
    (*(code *)PTR_StopMIDI_00033708)(this);
    (**(code **)(*(long *)this + 0xc20))(this);
  }
  return 0;
}
