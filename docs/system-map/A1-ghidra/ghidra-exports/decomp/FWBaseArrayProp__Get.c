// m_audio_b_FWBaseArrayProp::Get
// Entry: 0000800e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::Get(unsigned int) */

undefined4 __thiscall m_audio_b_FWBaseArrayProp::Get(m_audio_b_FWBaseArrayProp *this,uint param_1)

{
  undefined4 uVar1;
  
  CheckInvariant(this,param_1);
  if (param_1 < *(uint *)(this + 0x2c)) {
    uVar1 = *(undefined4 *)(*(long *)(this + 0x50) + (ulong)param_1 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
