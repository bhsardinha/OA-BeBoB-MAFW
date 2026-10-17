// m_audio_b_FWDiscreteEngine::RemoveAllSubEngines
// Entry: 000189a6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::RemoveAllSubEngines() */

void __thiscall m_audio_b_FWDiscreteEngine::RemoveAllSubEngines(m_audio_b_FWDiscreteEngine *this)

{
  long *plVar1;
  long *plVar2;
  long *local_20;
  
  if (*(OSCollection **)(this + 0x118) != (OSCollection *)0x0) {
    local_20 = (long *)0x0;
    plVar2 = (long *)TCollectionIterator<m_audio_b_FWPairEngine*>::start
                               ((TCollectionIterator<m_audio_b_FWPairEngine*> *)&local_20,
                                *(OSCollection **)(this + 0x118));
    plVar1 = local_20;
    while (plVar2 != (long *)0x0) {
      (**(code **)(*plVar2 + 0x600))(plVar2,0);
      plVar1 = local_20;
      plVar2 = (long *)(**(code **)(*local_20 + 0x128))(local_20);
    }
    (**(code **)(**(long **)(this + 0x118) + 0x158))();
    if (plVar1 != (long *)0x0) {
      (**(code **)(*plVar1 + 0x28))(plVar1);
    }
  }
  return;
}
