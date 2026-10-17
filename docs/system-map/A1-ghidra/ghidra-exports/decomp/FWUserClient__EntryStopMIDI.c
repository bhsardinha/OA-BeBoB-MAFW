// com_m_audio_FWUserClient::EntryStopMIDI
// Entry: 00007398
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::EntryStopMIDI() */

undefined8 __thiscall com_m_audio_FWUserClient::EntryStopMIDI(com_m_audio_FWUserClient *this)

{
  long *plVar1;
  undefined8 uVar2;
  
  plVar1 = (long *)(**(code **)(**(long **)(this + 0x290) + 0x688))();
  if (plVar1 == (long *)0x0) {
    uVar2 = 0xe00002d8;
  }
  else {
    uVar2 = (**(code **)(*plVar1 + 0x1a0))
                      (plVar1,ActionStopMIDI,*(undefined8 *)(*(long *)(this + 0x290) + 0xe8),0,0,0,0
                      );
  }
  return uVar2;
}
