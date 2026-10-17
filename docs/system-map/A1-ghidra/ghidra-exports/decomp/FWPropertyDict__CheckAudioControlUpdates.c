// m_audio_b_FWPropertyDict::CheckAudioControlUpdates
// Entry: 000097d0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::CheckAudioControlUpdates() */

void __thiscall m_audio_b_FWPropertyDict::CheckAudioControlUpdates(m_audio_b_FWPropertyDict *this)

{
  int iVar1;
  long *plVar2;
  m_audio_b_FWBaseProp *this_00;
  
  if ((*(long **)(this + 0x78) != (long *)0x0) &&
     (iVar1 = (**(code **)(**(long **)(this + 0x78) + 0x130))(), iVar1 != 0)) {
    if (*(OSCollection **)(this + 0x78) == (OSCollection *)0x0) {
      plVar2 = (long *)0x0;
    }
    else {
      plVar2 = (long *)OSCollectionIterator::withCollection(*(OSCollection **)(this + 0x78));
      if (plVar2 != (long *)0x0) {
        this_00 = (m_audio_b_FWBaseProp *)(**(code **)(*plVar2 + 0x128))(plVar2);
        while (this_00 != (m_audio_b_FWBaseProp *)0x0) {
          m_audio_b_FWBaseProp::NotifyAudioControlsInWorkLoop(this_00);
          this_00 = (m_audio_b_FWBaseProp *)(**(code **)(*plVar2 + 0x128))(plVar2);
        }
      }
    }
    (**(code **)(**(long **)(this + 0x78) + 0x158))();
    if (plVar2 != (long *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00009859. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*plVar2 + 0x28))(plVar2);
      return;
    }
  }
  return;
}
