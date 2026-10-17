// m_audio_b_FWPairEngine::stopEngineAtPosition
// Entry: 0001ace6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::stopEngineAtPosition(IOAudioEnginePosition*) */

void __thiscall
m_audio_b_FWPairEngine::stopEngineAtPosition
          (m_audio_b_FWPairEngine *this,IOAudioEnginePosition *param_1)

{
  if (param_1 == (IOAudioEnginePosition *)0x0) {
    return;
  }
  (**(code **)(**(long **)(this + 0x118) + 0xc60))
            (*(long **)(this + 0x118),1L << ((byte)this[300] & 0x3f),param_1);
                    /* WARNING: Could not recover jumptable at 0x0001ad2b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xa48))(this);
  return;
}
