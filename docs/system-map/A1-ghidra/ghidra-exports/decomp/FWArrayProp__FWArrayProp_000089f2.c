// m_audio_b_FWArrayProp::m_audio_b_FWArrayProp
// Entry: 000089f2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWArrayProp::m_audio_b_FWArrayProp() */

void __thiscall m_audio_b_FWArrayProp::m_audio_b_FWArrayProp(m_audio_b_FWArrayProp *this)

{
  m_audio_b_FWBaseProp::m_audio_b_FWBaseProp
            ((m_audio_b_FWBaseProp *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_00034660;
  *(undefined8 *)(this + 0x38) = 0;
  *(undefined ***)(this + 0x40) = &PTR__TFunctor_00034690;
  *(undefined8 *)(this + 0x48) = 0;
  *(undefined ***)this = &PTR__m_audio_b_FWArrayProp_00034a20;
  OSMetaClass::instanceConstructed();
  return;
}
