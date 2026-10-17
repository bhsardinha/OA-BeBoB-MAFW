// m_audio_b_FWIsochChannel::releaseChannel
// Entry: 00006dfc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::releaseChannel() */

void m_audio_b_FWIsochChannel::releaseChannel(void)

{
                    /* WARNING: Could not recover jumptable at 0x00006e08. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300c8 + 0x160))();
  return;
}
