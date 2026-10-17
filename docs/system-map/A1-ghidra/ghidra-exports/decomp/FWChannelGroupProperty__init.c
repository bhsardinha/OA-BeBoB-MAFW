// m_audio_b_FWChannelGroupProperty::init
// Entry: 000093a4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWChannelGroupProperty::init(m_audio_b_FWPropertyDict*, char const*,
   m_audio_FWb::BaseChannelGroup&, unsigned int, m_audio_FWb::BaseChannelGroup*) */

void __thiscall
m_audio_b_FWChannelGroupProperty::init
          (m_audio_b_FWChannelGroupProperty *this,m_audio_b_FWPropertyDict *param_1,char *param_2,
          BaseChannelGroup *param_3,uint param_4,BaseChannelGroup *param_5)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = (**(code **)(*(long *)param_3 + 0x10))(param_3);
  iVar2 = (*(code *)PTR_init_000349e8)(this,param_1,param_2,uVar1);
  if (iVar2 == 0) {
    *(BaseChannelGroup **)(this + 0x60) = param_3;
    *(uint *)(this + 0x68) = param_4;
    *(BaseChannelGroup **)(this + 0x70) = param_5;
  }
  return;
}
