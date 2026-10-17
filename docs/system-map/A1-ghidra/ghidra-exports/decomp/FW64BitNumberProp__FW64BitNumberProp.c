// m_audio_b_FW64BitNumberProp::m_audio_b_FW64BitNumberProp
// Entry: 0000fe24
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FW64BitNumberProp::m_audio_b_FW64BitNumberProp() */

void __thiscall
m_audio_b_FW64BitNumberProp::m_audio_b_FW64BitNumberProp(m_audio_b_FW64BitNumberProp *this)

{
  m_audio_b_FWBaseProp::m_audio_b_FWBaseProp
            ((m_audio_b_FWBaseProp *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FW64BitNumberProp_00037a90;
  *(undefined ***)(this + 0x38) = &PTR__TFunctor_00037620;
  *(undefined8 *)(this + 0x40) = 0;
  OSMetaClass::instanceConstructed();
  return;
}
