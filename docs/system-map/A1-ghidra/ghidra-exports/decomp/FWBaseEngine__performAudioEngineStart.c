// m_audio_b_FWBaseEngine::performAudioEngineStart
// Entry: 0000556e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::performAudioEngineStart() */

undefined4 __thiscall m_audio_b_FWBaseEngine::performAudioEngineStart(m_audio_b_FWBaseEngine *this)

{
  undefined4 uVar1;
  long local_28;
  long local_20;
  undefined1 local_18 [8];
  
  _clock_get_uptime(&local_20);
  uVar1 = m_audio_b_FWDiscreteEngine::ActivateSubEngine((m_audio_b_FWDiscreteEngine *)this,1);
  _clock_get_uptime(&local_28);
  _absolutetime_to_nanoseconds(local_28 - local_20,local_18);
  return uVar1;
}
