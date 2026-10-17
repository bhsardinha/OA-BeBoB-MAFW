// m_audio_b_FWDiscreteEngine::stop
// Entry: 0001867c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::stop(IOService*) */

void m_audio_b_FWDiscreteEngine::stop(IOService *param_1)

{
  (**(code **)(PTR_vtable_000300b0 + 0x5d8))(param_1);
  if (*(long **)(param_1 + 0x118) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0x118) + 0x28))();
    *(undefined8 *)(param_1 + 0x118) = 0;
  }
  return;
}
