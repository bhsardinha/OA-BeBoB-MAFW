// m_audio_b_FWPropertyDict::NoteChangesAvailable
// Entry: 00009cd4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::NoteChangesAvailable() */

void __thiscall m_audio_b_FWPropertyDict::NoteChangesAvailable(m_audio_b_FWPropertyDict *this)

{
  if (*(long *)(this + 0x30) == 0) {
    return;
  }
  IOEventSource::signalWorkAvailable();
  return;
}
