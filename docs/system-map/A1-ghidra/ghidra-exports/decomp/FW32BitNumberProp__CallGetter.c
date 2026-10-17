// m_audio_b_FW32BitNumberProp::CallGetter
// Entry: 0000f892
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FW32BitNumberProp::CallGetter(char const*, OSObject*&) */

undefined8 __thiscall
m_audio_b_FW32BitNumberProp::CallGetter
          (m_audio_b_FW32BitNumberProp *this,char *param_1,OSObject **param_2)

{
  OSObject *pOVar1;
  undefined8 uVar2;
  
  pOVar1 = (OSObject *)OSNumber::withNumber((ulong)*(uint *)(this + 0x2c),0x20);
  if (pOVar1 == (OSObject *)0x0) {
    uVar2 = 0xe00002bd;
  }
  else {
    *param_2 = pOVar1;
    uVar2 = 0;
  }
  return uVar2;
}
