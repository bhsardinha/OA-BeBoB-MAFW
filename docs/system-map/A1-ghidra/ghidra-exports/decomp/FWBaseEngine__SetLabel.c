// m_audio_b_FWBaseEngine::SetLabel
// Entry: 00003974
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e7)

/* m_audio_b_FWBaseEngine::SetLabel(char const*) */

void m_audio_b_FWBaseEngine::SetLabel(char *param_1)

{
                    /* WARNING: Could not recover jumptable at 0x00003983. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_1 + 0x9f8))();
  return;
}
