// com_m_audio_FWUserClient::EntrySendMIDI
// Entry: 000073fa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::EntrySendMIDI(MAMIDIPacket*, unsigned int*) */

undefined8 __thiscall
com_m_audio_FWUserClient::EntrySendMIDI
          (com_m_audio_FWUserClient *this,MAMIDIPacket *param_1,uint *param_2)

{
  long *plVar1;
  undefined8 uVar2;
  
  plVar1 = (long *)(**(code **)(**(long **)(this + 0x290) + 0x688))();
  if (plVar1 == (long *)0x0) {
    uVar2 = 0xe00002d8;
  }
  else {
    uVar2 = (**(code **)(*plVar1 + 0x1a0))
                      (plVar1,ActionSendMIDI,*(undefined8 *)(*(long *)(this + 0x290) + 0xe8),param_1
                       ,param_2,0,0);
  }
  return uVar2;
}
