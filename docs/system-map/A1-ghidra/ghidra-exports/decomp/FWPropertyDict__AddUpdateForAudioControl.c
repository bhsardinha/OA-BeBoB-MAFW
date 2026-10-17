// m_audio_b_FWPropertyDict::AddUpdateForAudioControl
// Entry: 00009ce8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::AddUpdateForAudioControl(m_audio_b_FWBaseProp*) */

void __thiscall
m_audio_b_FWPropertyDict::AddUpdateForAudioControl
          (m_audio_b_FWPropertyDict *this,m_audio_b_FWBaseProp *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x78);
  if (plVar1 == (long *)0x0) {
    plVar1 = (long *)OSSet::withCapacity(10);
    *(long **)(this + 0x78) = plVar1;
    if (plVar1 == (long *)0x0) {
      return;
    }
  }
  (**(code **)(*plVar1 + 0x1c0))(plVar1,param_1);
  if (*(long *)(this + 0x30) == 0) {
    return;
  }
  IOEventSource::signalWorkAvailable();
  return;
}
