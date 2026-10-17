// com_m_audio_FWEngine2::ActivatePCMOutput
// Entry: 00011a8c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::ActivatePCMOutput() */

undefined8 __thiscall com_m_audio_FWEngine2::ActivatePCMOutput(com_m_audio_FWEngine2 *this)

{
  undefined8 uVar1;
  
  if (this[0x2d3] == (com_m_audio_FWEngine2)0x0) {
    return 0xe00002bc;
  }
                    /* WARNING: Could not recover jumptable at 0x00011aab. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (**(code **)(**(long **)(this + 0x350) + 0x138))();
  return uVar1;
}
