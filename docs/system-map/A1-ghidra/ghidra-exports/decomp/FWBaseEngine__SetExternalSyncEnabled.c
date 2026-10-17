// m_audio_b_FWBaseEngine::SetExternalSyncEnabled
// Entry: 000053ec
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::SetExternalSyncEnabled(bool) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::SetExternalSyncEnabled(m_audio_b_FWBaseEngine *this,bool param_1)

{
  if (this[0x2d0] != (m_audio_b_FWBaseEngine)param_1) {
    if (*(int *)(this + 0x22c) != 0) {
      _IOLog("\nM-Audio FireWire: Cannot change sync mode while audio/MIDI streams are running.\n");
      return 0xe00002e6;
    }
    this[0x2d0] = (m_audio_b_FWBaseEngine)param_1;
  }
  return 0;
}
