// m_audio_b_FWBaseEngine::ReplicateCurrentIOFormatsToStreams
// Entry: 00003d1e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::ReplicateCurrentIOFormatsToStreams(IOAudioStream*, IOAudioStream*) */

void __thiscall
m_audio_b_FWBaseEngine::ReplicateCurrentIOFormatsToStreams
          (m_audio_b_FWBaseEngine *this,IOAudioStream *param_1,IOAudioStream *param_2)

{
  ReplicateCurrentFormatToStream(this,*(IOAudioStream **)(this + 0x210),param_1);
  ReplicateCurrentFormatToStream(this,*(IOAudioStream **)(this + 0x218),param_2);
  return;
}
