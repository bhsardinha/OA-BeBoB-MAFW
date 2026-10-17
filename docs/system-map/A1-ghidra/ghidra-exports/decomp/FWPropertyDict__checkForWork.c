// m_audio_b_FWPropertyDict::checkForWork
// Entry: 00009d38
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::checkForWork() */

undefined8 __thiscall m_audio_b_FWPropertyDict::checkForWork(m_audio_b_FWPropertyDict *this)

{
  int iVar1;
  
  CheckAudioControlUpdates(this);
  CheckModifiedProperties(this,true);
  if (*(long **)(this + 0x58) != (long *)0x0) {
    iVar1 = (**(code **)(**(long **)(this + 0x58) + 0x130))();
    if ((iVar1 != 0) && (*(long *)(this + 0x30) != 0)) {
      IOEventSource::signalWorkAvailable();
    }
  }
  return 0;
}
