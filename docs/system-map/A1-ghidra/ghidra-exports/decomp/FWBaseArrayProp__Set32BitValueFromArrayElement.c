// m_audio_b_FWBaseArrayProp::Set32BitValueFromArrayElement
// Entry: 00007f4a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::Set32BitValueFromArrayElement(unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWBaseArrayProp::Set32BitValueFromArrayElement
          (m_audio_b_FWBaseArrayProp *this,uint param_1,uint param_2)

{
  undefined8 *puVar1;
  
  if (param_1 < *(uint *)(this + 0x2c)) {
    *(uint *)(*(long *)(this + 0x50) + (ulong)param_1 * 4) = param_2;
    puVar1 = *(undefined8 **)(this + 0x38);
    if (puVar1 != (undefined8 *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00007f70. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)*puVar1)(puVar1,param_1 + 1);
      return;
    }
  }
  return;
}
