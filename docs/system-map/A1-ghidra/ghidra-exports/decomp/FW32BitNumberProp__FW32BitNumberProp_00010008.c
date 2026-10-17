// m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
// Entry: 00010008
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp(OSMetaClass const*) */

void __thiscall
m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
          (m_audio_b_FW32BitNumberProp *this,OSMetaClass *param_1)

{
  m_audio_b_FWBaseProp::m_audio_b_FWBaseProp((m_audio_b_FWBaseProp *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FW32BitNumberProp_00037c00;
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_000375f0;
  *(undefined8 *)(this + 0x38) = 0;
  _bzero(this,0x40);
  return;
}
