// m_audio_b_FWBaseProp::NotifyAudioControlsOfChange
// Entry: 00008ca6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::NotifyAudioControlsOfChange(IOService*) */

void m_audio_b_FWBaseProp::NotifyAudioControlsOfChange(IOService *param_1)

{
  if (*(long *)(param_1 + 0x10) == 0) {
    return;
  }
  m_audio_b_FWPropertyDict::AddUpdateForAudioControl
            (*(m_audio_b_FWPropertyDict **)(param_1 + 0x20),(m_audio_b_FWBaseProp *)param_1);
  return;
}
