// m_audio_b_FWFunctionProp::CallSetter
// Entry: 000100fa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWFunctionProp::CallSetter(OSSymbol const*, OSObject*) */

undefined8 __thiscall
m_audio_b_FWFunctionProp::CallSetter
          (m_audio_b_FWFunctionProp *this,OSSymbol *param_1,OSObject *param_2)

{
  undefined8 *puVar1;
  undefined8 uVar2;
  
  puVar1 = *(undefined8 **)(this + 0x38);
  if (puVar1 == (undefined8 *)0x0) {
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x00010115. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar2 = (**(code **)*puVar1)(puVar1,param_2);
  return uVar2;
}
