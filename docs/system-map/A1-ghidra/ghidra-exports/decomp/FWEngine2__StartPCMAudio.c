// com_m_audio_FWEngine2::StartPCMAudio
// Entry: 00011b20
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::StartPCMAudio() */

ulong __thiscall com_m_audio_FWEngine2::StartPCMAudio(com_m_audio_FWEngine2 *this)

{
  uint uVar1;
  ulong uVar2;
  
  if (this[0x2d3] == (com_m_audio_FWEngine2)0x0) {
    uVar2 = 0xe00002c2;
  }
  else if (((96000 < *(uint *)(this + 0x8c)) && (this[0x2d6] == (com_m_audio_FWEngine2)0x0)) ||
          (uVar2 = (**(code **)(*(long *)this + 0xd28))(this), (int)uVar2 == 0)) {
    uVar1 = (**(code **)(*(long *)this + 0xd20))(this);
    uVar2 = (ulong)uVar1;
    if (uVar1 != 0) {
      (**(code **)(*(long *)this + 0xd40))(this);
    }
  }
  return uVar2;
}
