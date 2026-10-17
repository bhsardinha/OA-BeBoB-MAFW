// m_audio_b_FWBaseArrayProp::CallGetter
// Entry: 000083d6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::CallGetter(char const*, OSObject*&) */

void __thiscall
m_audio_b_FWBaseArrayProp::CallGetter
          (m_audio_b_FWBaseArrayProp *this,char *param_1,OSObject **param_2)

{
  SeriatimValuesToOSArray(this,*(uint *)(this + 0x2c),param_2);
  return;
}
