// m_audio_b_FWArrayProp::m_audio_b_FWArrayProp
// Entry: 00008aa2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWArrayProp::m_audio_b_FWArrayProp(OSMetaClass const*) */

void __thiscall
m_audio_b_FWArrayProp::m_audio_b_FWArrayProp(m_audio_b_FWArrayProp *this,OSMetaClass *param_1)

{
  m_audio_b_FWBaseProp::m_audio_b_FWBaseProp((m_audio_b_FWBaseProp *)this,param_1);
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_00034660;
  *(undefined8 *)(this + 0x38) = 0;
  *(undefined ***)(this + 0x40) = &PTR__TFunctor_00034690;
  *(undefined8 *)(this + 0x48) = 0;
  *(undefined ***)this = &PTR__m_audio_b_FWArrayProp_00034a20;
  _bzero(this,0x60);
  return;
}
