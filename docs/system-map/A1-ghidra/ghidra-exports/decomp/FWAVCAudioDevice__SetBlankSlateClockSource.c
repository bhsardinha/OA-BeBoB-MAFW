// m_audio_b_FWAVCAudioDevice::SetBlankSlateClockSource
// Entry: 00022de8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e12)

/* m_audio_b_FWAVCAudioDevice::SetBlankSlateClockSource() */

undefined4 __thiscall
m_audio_b_FWAVCAudioDevice::SetBlankSlateClockSource(m_audio_b_FWAVCAudioDevice *this)

{
  undefined4 uVar1;
  
  if (*(long *)(this + 0xe8) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(*(long *)this + 0xb60))(this,0x40000000,1);
  }
  _IOSleep(0x9c4);
  return uVar1;
}
