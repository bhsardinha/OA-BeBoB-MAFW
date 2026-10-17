// m_audio_b_FWBaseEngine::StartIsochStreams
// Entry: 000054b0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::StartIsochStreams() */

ulong __thiscall m_audio_b_FWBaseEngine::StartIsochStreams(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  ulong uVar2;
  long local_28;
  long local_20;
  undefined1 local_18 [8];
  
  _clock_get_uptime(&local_20);
  if (this[0x2d3] == (m_audio_b_FWBaseEngine)0x0) {
    uVar2 = 0xe00002bc;
  }
  else {
    iVar1 = *(int *)(this + 0x22c);
    *(int *)(this + 0x22c) = iVar1 + 1;
    if (iVar1 == 0) {
      this[0x250] = (m_audio_b_FWBaseEngine)0x0;
      this[0x251] = (m_audio_b_FWBaseEngine)0x0;
      *(undefined4 *)(this + 0x248) = 0;
      _bzero(*(void **)(this + 0x270),(ulong)*(uint *)(this + 0x278));
      *(undefined8 *)(this + 0x284) = 0;
      *(undefined8 *)(this + 0x27c) = 0;
      uVar2 = (**(code **)(*(long *)this + 0xcd0))(this);
      if ((int)uVar2 != 0) {
        *(int *)(this + 0x22c) = *(int *)(this + 0x22c) + -1;
        return uVar2;
      }
    }
    else {
      uVar2 = 0;
    }
    uVar2 = uVar2 & 0xffffffff;
    _clock_get_uptime(&local_28);
    _absolutetime_to_nanoseconds(local_28 - local_20,local_18);
  }
  return uVar2;
}
