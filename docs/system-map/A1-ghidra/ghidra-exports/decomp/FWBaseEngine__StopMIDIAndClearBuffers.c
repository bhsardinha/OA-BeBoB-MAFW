// m_audio_b_FWBaseEngine::StopMIDIAndClearBuffers
// Entry: 00004d78
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_b_FWBaseEngine::StopMIDIAndClearBuffers() */

int __thiscall m_audio_b_FWBaseEngine::StopMIDIAndClearBuffers(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  uint uVar2;
  ulong uVar3;
  
  iVar1 = (**(code **)(*(long *)this + 0xcf8))(this);
  if (iVar1 == 0) {
    if (*(int *)(this + 0x29c) != 0) {
      uVar3 = 0;
      do {
        m_audio_FWb::TMIDIPort::Clear((TMIDIPort *)(*(long *)(this + 0x2b0) + uVar3 * 0x28));
        uVar2 = (int)uVar3 + 1;
        uVar3 = (ulong)uVar2;
      } while (uVar2 < *(uint *)(this + 0x29c));
    }
    if (*(int *)(this + 0x2a0) != 0) {
      uVar2 = 0;
      do {
        m_audio_FWb::TMIDIPort::Clear((TMIDIPort *)(*(long *)(this + 0x2a8) + (ulong)uVar2 * 0x28));
        uVar2 = uVar2 + 1;
      } while (uVar2 < *(uint *)(this + 0x2a0));
    }
  }
  return iVar1;
}
