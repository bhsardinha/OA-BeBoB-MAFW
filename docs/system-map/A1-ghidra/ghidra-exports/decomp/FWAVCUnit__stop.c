// m_audio_b_FWAVCUnit::stop
// Entry: 0000165e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::stop(IOService*) */

void m_audio_b_FWAVCUnit::stop(IOService *param_1)

{
  if (param_1[0xe0] != (IOService)0x0) {
    (**(code **)(PTR_vtable_000300d0 + 0x5d8))(param_1);
    param_1[0xe0] = (IOService)0x0;
  }
  return;
}
