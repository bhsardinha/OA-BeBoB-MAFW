// m_audio_b_FWDiscreteEngine::pauseAudioEngine
// Entry: 000187b8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::pauseAudioEngine() */

void __thiscall m_audio_b_FWDiscreteEngine::pauseAudioEngine(m_audio_b_FWDiscreteEngine *this)

{
  if ((**(int **)(this + 0x110) == 0) &&
     (*(int *)(this + 0x134) = *(int *)(this + 0x128), *(int *)(this + 0x128) != 0)) {
    *(undefined4 *)(this + 0x128) = 1;
  }
                    /* WARNING: Could not recover jumptable at 0x000187ea. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300b0 + 0xa60))();
  return;
}
