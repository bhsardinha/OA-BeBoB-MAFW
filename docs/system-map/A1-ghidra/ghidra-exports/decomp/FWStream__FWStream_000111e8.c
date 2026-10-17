// m_audio_b_FWStream::m_audio_b_FWStream
// Entry: 000111e8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWStream::m_audio_b_FWStream() */

void __thiscall m_audio_b_FWStream::m_audio_b_FWStream(m_audio_b_FWStream *this)

{
  IOAudioStream::IOAudioStream((IOAudioStream *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWStream_000385a0;
  OSMetaClass::instanceConstructed();
  return;
}
