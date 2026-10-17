// m_audio_b_FWBaseArrayProp::Fill
// Entry: 00007da8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::Fill(unsigned int) */

void __thiscall m_audio_b_FWBaseArrayProp::Fill(m_audio_b_FWBaseArrayProp *this,uint param_1)

{
  uint uVar1;
  
  if (*(int *)(this + 0x2c) != 0) {
    uVar1 = 0;
    do {
      *(uint *)(*(long *)(this + 0x50) + (ulong)uVar1 * 4) = param_1;
      uVar1 = uVar1 + 1;
    } while (uVar1 < *(uint *)(this + 0x2c));
  }
  return;
}
