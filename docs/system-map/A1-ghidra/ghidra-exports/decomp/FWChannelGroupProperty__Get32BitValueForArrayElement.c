// m_audio_b_FWChannelGroupProperty::Get32BitValueForArrayElement
// Entry: 000092ec
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWChannelGroupProperty::Get32BitValueForArrayElement(unsigned int) */

undefined4 m_audio_b_FWChannelGroupProperty::Get32BitValueForArrayElement(uint param_1)

{
  long lVar1;
  undefined4 in_register_0000003c;
  
  lVar1 = (**(code **)(**(long **)(CONCAT44(in_register_0000003c,param_1) + 0x60) + 0x18))();
  return *(undefined4 *)(lVar1 + (ulong)*(uint *)(CONCAT44(in_register_0000003c,param_1) + 0x68));
}
