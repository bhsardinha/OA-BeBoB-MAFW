// m_audio_b_FWBaseEngine::updateChannelNumbers
// Entry: 00004ad2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::updateChannelNumbers() */

void __thiscall m_audio_b_FWBaseEngine::updateChannelNumbers(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  int iVar2;
  long *plVar3;
  long *plVar4;
  int iVar5;
  long *local_38;
  long *local_30;
  
  local_30 = (long *)0x0;
  local_38 = (long *)0x0;
  plVar3 = (long *)TCollectionIterator<m_audio_b_FWStream*>::start
                             ((TCollectionIterator<m_audio_b_FWStream*> *)&local_30,
                              *(OSCollection **)(this + 0xe0));
  if (plVar3 == (long *)0x0) {
    iVar5 = 0;
  }
  else {
    iVar5 = 0;
    do {
      iVar1 = (**(code **)(*plVar3 + 0xa80))();
      iVar5 = iVar5 + iVar1;
      plVar3 = (long *)(**(code **)(*local_30 + 0x128))();
    } while (plVar3 != (long *)0x0);
  }
  plVar3 = (long *)TCollectionIterator<m_audio_b_FWStream*>::start
                             ((TCollectionIterator<m_audio_b_FWStream*> *)&local_38,
                              *(OSCollection **)(this + 0xd0));
  if (plVar3 == (long *)0x0) {
    iVar1 = 0;
    plVar4 = local_38;
  }
  else {
    iVar1 = 0;
    do {
      iVar2 = (**(code **)(*plVar3 + 0xa80))();
      plVar4 = local_38;
      iVar1 = iVar1 + iVar2;
      plVar3 = (long *)(**(code **)(*local_38 + 0x128))(local_38);
    } while (plVar3 != (long *)0x0);
  }
  *(int *)(this + 0xe8) = iVar5;
  *(int *)(this + 0xd8) = iVar1;
  (*(code *)PTR_updateChannelNumbers_000407d0)(this);
  if (plVar4 != (long *)0x0) {
    (**(code **)(*plVar4 + 0x28))(plVar4);
  }
  if (local_30 != (long *)0x0) {
    (**(code **)(*local_30 + 0x28))();
  }
  return;
}
