// com_m_audio_FWEngine2::StopAudio
// Entry: 00011ab2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::StopAudio() */

void __thiscall com_m_audio_FWEngine2::StopAudio(com_m_audio_FWEngine2 *this)

{
  if (this[0x238] != (com_m_audio_FWEngine2)0x0) {
    this[0x238] = (com_m_audio_FWEngine2)0x0;
    if (this[0x2d4] != (com_m_audio_FWEngine2)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00011b08. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*(long *)this + 0xd30))(this);
      return;
    }
    (**(code **)(*(long *)this + 0xd38))(this);
    if ((*(uint *)(this + 0x8c) < 0x17701) || (this[0x2d6] != (com_m_audio_FWEngine2)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x00011b1a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*(long *)this + 0xd40))(this);
      return;
    }
  }
  return;
}
