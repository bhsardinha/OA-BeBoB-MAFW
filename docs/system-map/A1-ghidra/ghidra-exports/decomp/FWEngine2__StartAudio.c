// com_m_audio_FWEngine2::StartAudio
// Entry: 00012024
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::StartAudio() */

int __thiscall com_m_audio_FWEngine2::StartAudio(com_m_audio_FWEngine2 *this)

{
  int iVar1;
  long local_30;
  long local_28;
  undefined1 local_20 [8];
  
  _clock_get_uptime(&local_28);
  if (this[0x238] == (com_m_audio_FWEngine2)0x0) {
    if (this[0x2d4] == (com_m_audio_FWEngine2)0x0) {
      iVar1 = (**(code **)(*(long *)this + 0xd08))(this);
    }
    else {
      *(undefined4 *)(this + 0x338) = 0;
      iVar1 = (**(code **)(*(long *)this + 0xd10))(this);
    }
    if (iVar1 == 0) {
      this[0x238] = (com_m_audio_FWEngine2)0x1;
      _clock_get_uptime(&local_30);
      _absolutetime_to_nanoseconds(local_30 - local_28,local_20);
      return 0;
    }
    this[0x238] = (com_m_audio_FWEngine2)0x0;
  }
  else {
    iVar1 = -0x1ffffd3e;
  }
  _clock_get_uptime(&local_30);
  _absolutetime_to_nanoseconds(local_30 - local_28,local_20);
  return iVar1;
}
