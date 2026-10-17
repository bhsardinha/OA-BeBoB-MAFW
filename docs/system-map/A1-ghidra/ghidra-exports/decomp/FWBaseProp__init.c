// m_audio_b_FWBaseProp::init
// Entry: 00009144
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::init(char const*) */

undefined1 __thiscall m_audio_b_FWBaseProp::init(m_audio_b_FWBaseProp *this,char *param_1)

{
  undefined1 uVar1;
  long lVar2;
  
  *(undefined8 *)(this + 0x10) = 0;
  *(undefined8 *)(this + 0x20) = 0;
  this[0x28] = (m_audio_b_FWBaseProp)0x0;
  lVar2 = OSSymbol::withCString(param_1);
  *(long *)(this + 0x18) = lVar2;
  if (lVar2 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(PTR_vtable_000300d8 + 0x98))(this);
  }
  return uVar1;
}
