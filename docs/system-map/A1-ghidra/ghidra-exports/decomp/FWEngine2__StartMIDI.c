// com_m_audio_FWEngine2::StartMIDI
// Entry: 000120d4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWEngine2::StartMIDI() */

ulong __thiscall com_m_audio_FWEngine2::StartMIDI(com_m_audio_FWEngine2 *this)

{
  uint uVar1;
  ulong uVar2;
  
  if (this[0x298] == (com_m_audio_FWEngine2)0x0) {
    if ((*(int *)(this + 0x2a0) == 0) && (*(int *)(this + 0x29c) == 0)) {
      uVar2 = 0xe00002c7;
    }
    else {
      uVar2 = (**(code **)(*(long *)this + 0xc18))(this);
      if ((int)uVar2 == 0) {
        if (*(int *)(this + 0x22c) == 1) {
          _IOSleep(600);
        }
        uVar1 = (*(code *)PTR_StartMIDI_00033700)(this);
        uVar2 = (ulong)uVar1;
        if (uVar1 != 0) {
          (**(code **)(*(long *)this + 0xc20))(this);
        }
      }
    }
  }
  else {
    uVar2 = 0xe00002d2;
  }
  return uVar2;
}
