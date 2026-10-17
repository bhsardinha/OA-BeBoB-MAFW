// m_audio_b_FWAVCAudioDevice::stop
// Entry: 000229d8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e5)

/* m_audio_b_FWAVCAudioDevice::stop(IOService*) */

void m_audio_b_FWAVCAudioDevice::stop(IOService *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(param_1 + 0x268);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x600))(plVar1,1);
    *(undefined8 *)(param_1 + 0x268) = 0;
  }
  (*(code *)PTR_stop_00031738)(param_1);
  return;
}
