// m_audio_b_FWBaseArrayProp::Get32BitValueForArrayElement
// Entry: 00007f72
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::Get32BitValueForArrayElement(unsigned int) */

undefined4 __thiscall
m_audio_b_FWBaseArrayProp::Get32BitValueForArrayElement
          (m_audio_b_FWBaseArrayProp *this,uint param_1)

{
  if (param_1 < *(uint *)(this + 0x2c)) {
    return *(undefined4 *)(*(long *)(this + 0x50) + (ulong)param_1 * 4);
  }
  return 0;
}
