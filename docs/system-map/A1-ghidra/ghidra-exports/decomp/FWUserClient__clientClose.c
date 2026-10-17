// com_m_audio_FWUserClient::clientClose
// Entry: 000071a8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::clientClose() */

undefined8 __thiscall com_m_audio_FWUserClient::clientClose(com_m_audio_FWUserClient *this)

{
  m_audio_b_FWBaseEngine *pmVar1;
  
  *(undefined8 *)(this + 0x150) = 0;
  if (this[0x140] != (com_m_audio_FWUserClient)0x0) {
    if ((*(long *)(this + 0x290) != 0) &&
       (pmVar1 = *(m_audio_b_FWBaseEngine **)(*(long *)(this + 0x290) + 0xe8),
       pmVar1 != (m_audio_b_FWBaseEngine *)0x0)) {
      CoreStopMIDI(pmVar1);
    }
    this[0x140] = (com_m_audio_FWUserClient)0x0;
  }
  return 0;
}
