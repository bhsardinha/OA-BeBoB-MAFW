// m_audio_b_FWDiscreteEngine::resumeAudioEngine
// Entry: 000187f0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::resumeAudioEngine() */

void __thiscall m_audio_b_FWDiscreteEngine::resumeAudioEngine(m_audio_b_FWDiscreteEngine *this)

{
  if (**(int **)(this + 0x110) == 1) {
    *(uint *)(this + 0x128) = *(uint *)(this + 0x128) | *(uint *)(this + 0x134);
    *(undefined4 *)(this + 0x134) = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x0001881e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300b0 + 0xa68))();
  return;
}
