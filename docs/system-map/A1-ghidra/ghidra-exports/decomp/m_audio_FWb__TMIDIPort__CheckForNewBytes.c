// m_audio_FWb::TMIDIPort::CheckForNewBytes
// Entry: 0001e6e8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_FWb::TMIDIPort::CheckForNewBytes(m_audio_FWb::MIDIMemVector&, unsigned int) */

undefined8 __thiscall
m_audio_FWb::TMIDIPort::CheckForNewBytes(TMIDIPort *this,MIDIMemVector *param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar1 = **(uint **)this;
  *(undefined4 *)param_1 = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0x18) = 0;
  uVar2 = *(uint *)(this + 8);
  if (uVar1 != uVar2) {
    if (uVar2 < uVar1) {
      uVar4 = uVar1 - uVar2;
    }
    else {
      uVar4 = uVar1 + 1 + ~uVar2;
    }
    if (0x200 < uVar4) {
      uVar4 = 0x200;
    }
    if (param_2 < uVar4) {
      uVar4 = param_2;
    }
    uVar3 = uVar2 & 0x1ff;
    if (uVar4 + uVar3 < 0x201) {
      *(undefined4 *)param_1 = 1;
      *(uint *)(param_1 + 8) = uVar4;
      *(ulong *)(param_1 + 0x10) = *(long *)this + 4 + (ulong)uVar3;
      *(undefined4 *)(param_1 + 0x18) = 0;
    }
    else {
      *(undefined4 *)param_1 = 2;
      *(uint *)(param_1 + 8) = 0x200 - uVar3;
      *(ulong *)(param_1 + 0x10) = *(long *)this + 4 + (ulong)uVar3;
      *(uint *)(param_1 + 0x18) = (uVar2 | 0xfffffe00) + uVar4;
      *(long *)(param_1 + 0x20) = *(long *)this + 4;
    }
    *(uint *)(this + 8) = uVar1;
    return 1;
  }
  return 0;
}
