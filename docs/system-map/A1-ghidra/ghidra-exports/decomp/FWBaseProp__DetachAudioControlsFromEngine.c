// m_audio_b_FWBaseProp::DetachAudioControlsFromEngine
// Entry: 00008e7e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::DetachAudioControlsFromEngine(IOAudioEngine*) */

void __thiscall
m_audio_b_FWBaseProp::DetachAudioControlsFromEngine
          (m_audio_b_FWBaseProp *this,IOAudioEngine *param_1)

{
  long *plVar1;
  long lVar2;
  long *local_28;
  
  if (*(OSCollection **)(this + 0x10) != (OSCollection *)0x0) {
    local_28 = (long *)0x0;
    lVar2 = TCollectionIterator<IOAudioControl*>::start
                      ((TCollectionIterator<IOAudioControl*> *)&local_28,
                       *(OSCollection **)(this + 0x10));
    plVar1 = local_28;
    while (lVar2 != 0) {
      (**(code **)(*(long *)param_1 + 0xb20))(param_1,lVar2);
      lVar2 = (**(code **)(*plVar1 + 0x128))(plVar1);
    }
    (**(code **)(**(long **)(this + 0x10) + 0x158))();
    if (plVar1 != (long *)0x0) {
      (**(code **)(*plVar1 + 0x28))(plVar1);
    }
  }
  return;
}
