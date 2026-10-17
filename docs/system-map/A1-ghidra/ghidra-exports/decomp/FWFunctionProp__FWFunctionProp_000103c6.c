// m_audio_b_FWFunctionProp::m_audio_b_FWFunctionProp
// Entry: 000103c6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWFunctionProp::m_audio_b_FWFunctionProp(OSMetaClass const*) */

void __thiscall
m_audio_b_FWFunctionProp::m_audio_b_FWFunctionProp
          (m_audio_b_FWFunctionProp *this,OSMetaClass *param_1)

{
  m_audio_b_FWBaseProp::m_audio_b_FWBaseProp((m_audio_b_FWBaseProp *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FWFunctionProp_00037ec0;
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_00037da0;
  *(undefined8 *)(this + 0x38) = 0;
  _bzero(this,0x40);
  return;
}
