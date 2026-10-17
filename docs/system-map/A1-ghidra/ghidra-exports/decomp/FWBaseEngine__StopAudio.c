// m_audio_b_FWBaseEngine::StopAudio
// Entry: 00004738
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::StopAudio() */

void __thiscall m_audio_b_FWBaseEngine::StopAudio(m_audio_b_FWBaseEngine *this)

{
  long *plVar1;
  long *plVar2;
  long *local_18;
  
  local_18 = (long *)0x0;
  plVar2 = (long *)TCollectionIterator<m_audio_b_FWDCLProgram*>::start
                             ((TCollectionIterator<m_audio_b_FWDCLProgram*> *)&local_18,
                              *(OSCollection **)(this + 0x178));
  plVar1 = local_18;
  while (plVar2 != (long *)0x0) {
    (**(code **)(*plVar2 + 0x140))();
    plVar1 = local_18;
    plVar2 = (long *)(**(code **)(*local_18 + 0x128))(local_18);
  }
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x28))(plVar1);
  }
  return;
}
