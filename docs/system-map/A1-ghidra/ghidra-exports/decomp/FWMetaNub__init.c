// com_m_audio_FWMetaNub::init
// Entry: 0001b898
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::init(OSDictionary*) */

bool com_m_audio_FWMetaNub::init(OSDictionary *param_1)

{
  char cVar1;
  
  *(undefined4 *)(param_1 + 0x88) = 0;
  cVar1 = (**(code **)(PTR_vtable_000300e0 + 600))();
  return cVar1 != '\0';
}
