// com_m_audio_FWEngine2::DisconnectAllPlugs
// Entry: 00011bfa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::DisconnectAllPlugs() */

void __thiscall com_m_audio_FWEngine2::DisconnectAllPlugs(com_m_audio_FWEngine2 *this)

{
  if (this[0x2d3] == (com_m_audio_FWEngine2)0x0) {
    return;
  }
  this[0x2d3] = (com_m_audio_FWEngine2)0x0;
  (**(code **)(**(long **)(this + 0x358) + 0x128))();
                    /* WARNING: Could not recover jumptable at 0x00011c3d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(this + 0x350) + 0x128))();
  return;
}
