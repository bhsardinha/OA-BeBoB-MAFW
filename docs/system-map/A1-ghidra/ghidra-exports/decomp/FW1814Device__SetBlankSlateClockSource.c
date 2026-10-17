// com_m_audio_FW1814Device::SetBlankSlateClockSource
// Entry: 0000e4f0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetBlankSlateClockSource() */

undefined4 __thiscall
com_m_audio_FW1814Device::SetBlankSlateClockSource(com_m_audio_FW1814Device *this)

{
  undefined4 uVar1;
  
  if (*(long *)(this + 0xe8) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(*(long *)this + 0xb60))
                      (this,(*(uint *)(this + 0x144) & 0x1c) + 0x40000003,1);
    _IOSleep(0x9c4);
  }
  return uVar1;
}
