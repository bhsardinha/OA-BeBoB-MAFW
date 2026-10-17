// com_m_audio_FWEngine2::DeactivateAC3Output
// Entry: 00011fc6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::DeactivateAC3Output() */

void __thiscall com_m_audio_FWEngine2::DeactivateAC3Output(com_m_audio_FWEngine2 *this)

{
  (**(code **)(*(long *)this + 0xd38))(this);
  SetAC3OutputState(this,false);
  return;
}
