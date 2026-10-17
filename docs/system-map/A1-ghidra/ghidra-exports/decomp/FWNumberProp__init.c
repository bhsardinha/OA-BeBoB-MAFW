// m_audio_b_FWNumberProp::init
// Entry: 0000f8c4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWNumberProp::init(m_audio_b_FWPropertyDict*, char const*) */

undefined4 __thiscall
m_audio_b_FWNumberProp::init
          (m_audio_b_FWNumberProp *this,m_audio_b_FWPropertyDict *param_1,char *param_2)

{
  undefined4 uVar1;
  
  (*(code *)PTR_init_00034dc8)(this,param_2);
  uVar1 = m_audio_b_FWPropertyDict::InstallProperty(param_1,(m_audio_b_FWBaseProp *)this);
  (**(code **)(*(long *)this + 0x28))(this);
  return uVar1;
}
