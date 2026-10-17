// m_audio_b_FWPairEngine::init
// Entry: 0001afb8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::init() */

bool __thiscall m_audio_b_FWPairEngine::init(m_audio_b_FWPairEngine *this)

{
  char cVar1;
  
  this[0x128] = (m_audio_b_FWPairEngine)0x1;
  cVar1 = (*(code *)PTR_init_000425c8)(this,0);
  if (cVar1 != '\0') {
    this[0x128] = (m_audio_b_FWPairEngine)0x0;
  }
  return cVar1 != '\0';
}
