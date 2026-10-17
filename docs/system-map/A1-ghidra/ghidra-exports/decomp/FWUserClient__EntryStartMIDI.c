// com_m_audio_FWUserClient::EntryStartMIDI
// Entry: 0000732c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::EntryStartMIDI() */

undefined8 __thiscall com_m_audio_FWUserClient::EntryStartMIDI(com_m_audio_FWUserClient *this)

{
  long *plVar1;
  undefined8 uVar2;
  
  plVar1 = (long *)(**(code **)(**(long **)(this + 0x290) + 0x688))();
  if (plVar1 == (long *)0x0) {
    uVar2 = 0xe00002d8;
  }
  else {
    uVar2 = (**(code **)(*plVar1 + 0x1a0))
                      (plVar1,ActionStartMIDI,*(undefined8 *)(*(long *)(this + 0x290) + 0xe8),0,0,0,
                       0);
    if ((int)uVar2 == 0) {
      this[0x140] = (com_m_audio_FWUserClient)0x1;
    }
  }
  return uVar2;
}
