// m_audio_FWb::TMIDIPort::Clear
// Entry: 00004d36
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_FWb::TMIDIPort::Clear() */

void __thiscall m_audio_FWb::TMIDIPort::Clear(TMIDIPort *this)

{
  *(undefined4 *)(this + 8) = 0;
  _bzero(*(void **)this,0x204);
  _bzero(*(void **)(this + 0x10),0x808);
  _bzero(*(void **)(this + 0x18),0x808);
  return;
}
