// m_audio_b_FWBaseEngine::CompletePendingStateChanges
// Entry: 00003ae6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::CompletePendingStateChanges() */

void __thiscall m_audio_b_FWBaseEngine::CompletePendingStateChanges(m_audio_b_FWBaseEngine *this)

{
  long *plVar1;
  
  plVar1 = (long *)(**(code **)(*(long *)this + 0x688))(this);
  (**(code **)(*plVar1 + 0x1a0))(plVar1,CompletePendingStateChangesAction,this,0,0,0,0);
  return;
}
