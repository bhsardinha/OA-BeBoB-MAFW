// m_audio_b_FWIsochChannel::handleBusReset
// Entry: 00006db8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::handleBusReset() */

void m_audio_b_FWIsochChannel::handleBusReset(void)

{
                    /* WARNING: Could not recover jumptable at 0x00006dc4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300c8 + 0x140))();
  return;
}
