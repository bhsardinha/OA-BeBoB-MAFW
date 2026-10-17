// m_audio_b_FWBaseArrayProp::Set
// Entry: 00008090
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::Set(unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWBaseArrayProp::Set(m_audio_b_FWBaseArrayProp *this,uint param_1,uint param_2)

{
  CheckInvariant(this,param_1);
  if (param_1 < *(uint *)(this + 0x2c)) {
    *(uint *)(*(long *)(this + 0x50) + (ulong)param_1 * 4) = param_2;
  }
  return;
}
