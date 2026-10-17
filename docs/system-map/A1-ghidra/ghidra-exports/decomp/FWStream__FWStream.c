// m_audio_b_FWStream::m_audio_b_FWStream
// Entry: 00011156
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWStream::m_audio_b_FWStream(OSMetaClass const*) */

void __thiscall
m_audio_b_FWStream::m_audio_b_FWStream(m_audio_b_FWStream *this,OSMetaClass *param_1)

{
  IOAudioStream::IOAudioStream((IOAudioStream *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FWStream_000385a0;
  return;
}
