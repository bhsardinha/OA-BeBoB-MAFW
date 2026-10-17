// m_audio_b_FWBaseEngine::StopIsochStreams
// Entry: 0000416c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::StopIsochStreams() */

void __thiscall m_audio_b_FWBaseEngine::StopIsochStreams(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  
  if ((*(int *)(this + 0x22c) != 0) &&
     (iVar1 = *(int *)(this + 0x22c) + -1, *(int *)(this + 0x22c) = iVar1, iVar1 == 0)) {
                    /* WARNING: Could not recover jumptable at 0x0000418c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(long *)this + 0xcd8))();
    return;
  }
  return;
}
