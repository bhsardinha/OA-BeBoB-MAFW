// com_m_audio_FWEngine2::ActivateAC3Output
// Entry: 00011fee
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::ActivateAC3Output() */

void __thiscall com_m_audio_FWEngine2::ActivateAC3Output(com_m_audio_FWEngine2 *this)

{
  int iVar1;
  
  iVar1 = SetAC3OutputState(this,true);
  if (iVar1 != 0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0001201e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xd20))();
  return;
}
