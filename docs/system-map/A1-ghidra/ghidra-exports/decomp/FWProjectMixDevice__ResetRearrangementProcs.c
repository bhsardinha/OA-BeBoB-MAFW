// com_m_audio_FWProjectMixDevice::ResetRearrangementProcs
// Entry: 0001a61e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* com_m_audio_FWProjectMixDevice::ResetRearrangementProcs(m_audio_b_FWBaseEngine*) */

void __thiscall
com_m_audio_FWProjectMixDevice::ResetRearrangementProcs
          (com_m_audio_FWProjectMixDevice *this,m_audio_b_FWBaseEngine *param_1)

{
  (*(code *)PTR_ResetRearrangementProcs_000374b8)(this,param_1);
  if ((ulong)*(uint *)(param_1 + 0x2e4) != 2) {
    *(code **)(param_1 + 0x310) = _ConvertInputFrames1212CS;
    *(ulong *)(param_1 + 0x318) = (ulong)*(uint *)(param_1 + 0x2e4);
  }
  return;
}
