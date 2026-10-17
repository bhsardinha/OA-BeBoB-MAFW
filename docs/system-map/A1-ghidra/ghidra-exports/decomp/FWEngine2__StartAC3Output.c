// com_m_audio_FWEngine2::StartAC3Output
// Entry: 0001185e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::StartAC3Output() */

void __thiscall com_m_audio_FWEngine2::StartAC3Output(com_m_audio_FWEngine2 *this)

{
  if (this[0x298] != (com_m_audio_FWEngine2)0x0) {
    (**(code **)(*(long *)this + 0xcf8))(this);
  }
                    /* WARNING: Could not recover jumptable at 0x0001188b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xd18))(this);
  return;
}
