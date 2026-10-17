// m_audio_b_FWDiscreteEngine::ResetStereoPairIOFormats
// Entry: 00018a2a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::ResetStereoPairIOFormats() */

void __thiscall
m_audio_b_FWDiscreteEngine::ResetStereoPairIOFormats(m_audio_b_FWDiscreteEngine *this)

{
  long *plVar1;
  long *plVar2;
  long *local_20;
  
  if (*(OSCollection **)(this + 0x118) != (OSCollection *)0x0) {
    local_20 = (long *)0x0;
    plVar1 = (long *)TCollectionIterator<m_audio_b_FWPairEngine*>::start
                               ((TCollectionIterator<m_audio_b_FWPairEngine*> *)&local_20,
                                *(OSCollection **)(this + 0x118));
    plVar2 = local_20;
    if (plVar1 != (long *)0x0) {
      do {
        (**(code **)(*plVar1 + 0xbf8))(plVar1,this + 0x8c);
        plVar2 = local_20;
        plVar1 = (long *)(**(code **)(*local_20 + 0x128))(local_20);
      } while (plVar1 != (long *)0x0);
    }
    if (plVar2 != (long *)0x0) {
      (**(code **)(*plVar2 + 0x28))(plVar2);
    }
  }
  return;
}
