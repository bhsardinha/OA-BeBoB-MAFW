// m_audio_b_FWPairStream::m_audio_b_FWPairStream
// Entry: 0001b6b6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWPairStream::m_audio_b_FWPairStream() */

void __thiscall m_audio_b_FWPairStream::m_audio_b_FWPairStream(m_audio_b_FWPairStream *this)

{
  IOAudioStream::IOAudioStream((IOAudioStream *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWPairStream_000457e0;
  OSMetaClass::instanceConstructed();
  return;
}
