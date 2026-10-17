// m_audio_b_FWBaseArrayProp::CallSetter
// Entry: 00008188
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::CallSetter(OSSymbol const*, OSObject*) */

int __thiscall
m_audio_b_FWBaseArrayProp::CallSetter
          (m_audio_b_FWBaseArrayProp *this,OSSymbol *param_1,OSObject *param_2)

{
  int iVar1;
  
  iVar1 = OSArrayToSeriatimValues(this,param_2,*(uint *)(this + 0x2c));
  if ((iVar1 == 0) && (*(undefined8 **)(this + 0x48) != (undefined8 *)0x0)) {
    (**(code **)**(undefined8 **)(this + 0x48))();
  }
  return iVar1;
}
