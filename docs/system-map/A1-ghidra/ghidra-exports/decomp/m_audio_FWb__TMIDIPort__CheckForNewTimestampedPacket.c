// m_audio_FWb::TMIDIPort::CheckForNewTimestampedPacket
// Entry: 0001e7fa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_FWb::TMIDIPort::CheckForNewTimestampedPacket(DigiFWMIDIPacket*&, unsigned long long,
   unsigned long long) */

undefined8 __thiscall
m_audio_FWb::TMIDIPort::CheckForNewTimestampedPacket
          (TMIDIPort *this,DigiFWMIDIPacket **param_1,ulonglong param_2,ulonglong param_3)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = *(int **)(this + 0x10);
  iVar1 = piVar2[1];
  if ((*piVar2 != iVar1) &&
     (*param_1 = (DigiFWMIDIPacket *)(piVar2 + (long)iVar1 * 4 + 2),
     (long)(*(long *)(piVar2 + (long)iVar1 * 4 + 2) - param_2) < (long)param_3)) {
    iVar1 = *(int *)(*(long *)(this + 0x10) + 4);
    *(uint *)(*(long *)(this + 0x10) + 4) =
         (iVar1 + 1) - (iVar1 + 1 + ((uint)(iVar1 + 1 >> 0x1f) >> 0x19) & 0xffffff80);
    return 1;
  }
  return 0;
}
