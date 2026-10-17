// m_audio_b_FWChannelGroupProperty::Set32BitValueFromArrayElement
// Entry: 00009310
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWChannelGroupProperty::Set32BitValueFromArrayElement(unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWChannelGroupProperty::Set32BitValueFromArrayElement
          (m_audio_b_FWChannelGroupProperty *this,uint param_1,uint param_2)

{
  long lVar1;
  
  lVar1 = (**(code **)(**(long **)(this + 0x60) + 0x18))();
  *(int *)(lVar1 + (ulong)*(uint *)(this + 0x68)) = (int)(short)param_2;
  return;
}
