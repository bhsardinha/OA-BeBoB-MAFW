// m_audio_FWb::TMIDIPort::CheckForNewRealtimePacket
// Entry: 0001e7b2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_FWb::TMIDIPort::CheckForNewRealtimePacket(DigiFWMIDIPacket*&) */

undefined8 __thiscall
m_audio_FWb::TMIDIPort::CheckForNewRealtimePacket(TMIDIPort *this,DigiFWMIDIPacket **param_1)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = *(int **)(this + 0x18);
  if (*piVar2 != piVar2[1]) {
    *param_1 = (DigiFWMIDIPacket *)(piVar2 + (long)piVar2[1] * 4 + 2);
    iVar1 = *(int *)(*(long *)(this + 0x18) + 4);
    *(uint *)(*(long *)(this + 0x18) + 4) =
         (iVar1 + 1) - (iVar1 + 1 + ((uint)(iVar1 + 1 >> 0x1f) >> 0x19) & 0xffffff80);
    return 1;
  }
  return 0;
}
