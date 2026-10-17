// FWSettingsAuxSend::Init
// Entry: 00020854
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::Init(m_audio_b_FWAVCAudioDevice*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*, unsigned int) */

void __thiscall
FWSettingsAuxSend::Init
          (FWSettingsAuxSend *this,m_audio_b_FWAVCAudioDevice *param_1,BaseChannelGroup *param_2,
          BaseChannelGroup *param_3,uint param_4)

{
  long lVar1;
  undefined *puVar2;
  int iVar3;
  uint uVar4;
  
  iVar3 = (*(code *)PTR_Init_000470c0)(this,param_1);
  if (iVar3 == 0) {
    *(m_audio_b_FWAVCAudioDevice **)(this + 0x2d0) = param_1;
    *(undefined8 *)(this + 0x2c0) = *(undefined8 *)(param_1 + 0xf8);
    *(uint *)(this + 0x2c8) = param_4;
    *(BaseChannelGroup **)(this + 0x10) = param_2;
    *(BaseChannelGroup **)(this + 0x18) = param_3;
    *(undefined4 *)(param_2 + 0x14) = 0xffffff01;
    *(undefined4 *)(param_2 + 0x18) = 0;
    lVar1 = *(long *)(this + 0x18);
    *(undefined4 *)(lVar1 + 0x14) = 0xffffff01;
    *(undefined4 *)(lVar1 + 0x18) = 0;
    m_audio_FWb::
    TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
    ::Init<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int)>
              ((FWSettingsAuxSend *)(*(long *)(this + 0x10) + 0x20),(_func_int_uint_uint *)this);
    m_audio_FWb::
    TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
    ::Init<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int)>
              ((FWSettingsAuxSend *)(*(long *)(this + 0x18) + 0x20),(_func_int_uint_uint *)this);
    puVar2 = PTR_init_00035078;
    (*(code *)PTR_init_00035078)
              (this + 0x20,*(undefined8 *)(this + 0x2c0),"MAAuxSendLevels",
               *(undefined8 *)(this + 0x10),0,0);
    (*(code *)puVar2)(this + 0x98,*(undefined8 *)(this + 0x2c0),"MAAuxOutputLevels",
                      *(undefined8 *)(this + 0x18),0,0);
    (*(code *)puVar2)(this + 0x110,*(undefined8 *)(this + 0x2c0),"MAAuxOutputMute",
                      *(undefined8 *)(this + 0x18),8,0);
    (*(code *)puVar2)(this + 0x188,*(undefined8 *)(this + 0x2c0),"MAAuxOutputLink",
                      *(undefined8 *)(this + 0x18),0x18,0);
    uVar4 = (**(code **)(**(long **)(this + 0x18) + 0x10))();
    iVar3 = m_audio_b_FWBaseArrayProp::
            init<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int)>
                      ((m_audio_b_FWPropertyDict *)(this + 0x260),*(char **)(this + 0x2c0),0x2f1ce,
                       (FWSettingsAuxSend *)(ulong)uVar4,(_func_int_uint_uint *)this);
    if (iVar3 == 0) {
      m_audio_b_FWBaseArrayProp::
      init<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int)>
                ((m_audio_b_FWPropertyDict *)(this + 0x200),*(char **)(this + 0x2c0),0x2f1de,
                 (FWSettingsAuxSend *)(ulong)*(uint *)(this + 0x2c8),(_func_int_uint_uint *)this);
    }
  }
  return;
}
