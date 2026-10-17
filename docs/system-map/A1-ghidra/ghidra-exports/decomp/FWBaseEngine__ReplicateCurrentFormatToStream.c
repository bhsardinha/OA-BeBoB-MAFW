// m_audio_b_FWBaseEngine::ReplicateCurrentFormatToStream
// Entry: 00003cb4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::ReplicateCurrentFormatToStream(IOAudioStream*, IOAudioStream*) */

void __thiscall
m_audio_b_FWBaseEngine::ReplicateCurrentFormatToStream
          (m_audio_b_FWBaseEngine *this,IOAudioStream *param_1,IOAudioStream *param_2)

{
  (**(code **)(*(long *)param_2 + 0xa68))(param_2);
  (**(code **)(*(long *)param_2 + 0xa58))(param_2,param_1 + 0xb0,this + 0x8c,this + 0x8c,0,0);
                    /* WARNING: Could not recover jumptable at 0x00003d1b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_2 + 0xa38))(param_2,param_1 + 0xb0,0);
  return;
}
