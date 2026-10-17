// FWSettingsLevels::Init
// Entry: 00021dd8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::Init(m_audio_b_FWAVCAudioDevice*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*) */

int __thiscall
FWSettingsLevels::Init
          (FWSettingsLevels *this,m_audio_b_FWAVCAudioDevice *param_1,BaseChannelGroup *param_2,
          BaseChannelGroup *param_3,BaseChannelGroup *param_4,BaseChannelGroup *param_5,
          BaseChannelGroup *param_6)

{
  m_audio_b_FWBaseProp *this_00;
  IOAudioEngine *pIVar1;
  long lVar2;
  int iVar3;
  uint uVar4;
  BaseChannelGroup *pBVar5;
  FWSettingsLevels *this_01;
  IOAudioControl *local_78;
  IOAudioControl *local_70;
  IOAudioControl *local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  
  pIVar1 = *(IOAudioEngine **)(param_1 + 0xe8);
  iVar3 = (*(code *)PTR_Init_000470c0)(this,param_1);
  if (iVar3 != 0) {
    return iVar3;
  }
  *(m_audio_b_FWAVCAudioDevice **)(this + 0xa58) = param_1;
  *(BaseChannelGroup **)(this + 0x8b0) = param_2;
  *(BaseChannelGroup **)(this + 0x8b8) = param_3;
  *(BaseChannelGroup **)(this + 0x8c0) = param_4;
  *(BaseChannelGroup **)(this + 0x8c8) = param_5;
  *(undefined8 *)(this + 0xa50) = *(undefined8 *)(param_1 + 0xf8);
  if (param_2 != (BaseChannelGroup *)0x0) {
    *(undefined4 *)(param_2 + 0x14) = 0xffffff01;
    *(undefined4 *)(param_2 + 0x18) = 0;
    m_audio_FWb::
    TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
    ::Init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
              ((FWSettingsLevels *)(*(long *)(this + 0x8b0) + 0x20),(_func_int_uint_uint *)this);
    iVar3 = InitGroupProp(this,"SWReturn",*(BaseChannelGroup **)(this + 0x8b0),
                          (ChannelGroupProperties *)(this + 0x4f0),true,
                          *(BaseChannelGroup **)(this + 0x8c0));
    if (iVar3 != 0) {
      return iVar3;
    }
    local_60 = 0x71;
    local_58 = 0;
    uVar4 = (**(code **)(**(long **)(this + 0x8b0) + 0x10))();
    iVar3 = m_audio_b_FWBaseArrayProp::
            init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
                      ((m_audio_b_FWPropertyDict *)(this + 0x9f0),*(char **)(this + 0xa50),0x2f27e,
                       (FWSettingsLevels *)(ulong)(uVar4 >> 1),(_func_int_uint_uint *)this);
    if (iVar3 != 0) {
      return iVar3;
    }
  }
  if (param_3 == (BaseChannelGroup *)0x0) {
    if (param_2 == (BaseChannelGroup *)0x0) goto LAB_00022096;
    m_audio_b_FWBaseProp::EstablishVolumeControl
              ((m_audio_b_FWBaseProp *)(this + 0x4f0),pIVar1,0x6f757470,0,0,&local_70,-0x80,0,-0x40,
               0);
    m_audio_b_FWBaseArrayProp::EstablishVolumeControls
              ((m_audio_b_FWBaseArrayProp *)(this + 0x4f0),pIVar1,0x6f757470,1,-0x80,0,-0x40,0);
    this_01 = this + 0x568;
  }
  else {
    lVar2 = *(long *)(this + 0x8b8);
    *(undefined4 *)(lVar2 + 0x14) = 0xffffff01;
    *(undefined4 *)(lVar2 + 0x18) = 0;
    m_audio_FWb::
    TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
    ::Init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
              ((FWSettingsLevels *)(*(long *)(this + 0x8b8) + 0x20),(_func_int_uint_uint *)this);
    this_00 = (m_audio_b_FWBaseProp *)(this + 0x130);
    iVar3 = InitGroupProp(this,"Output",*(BaseChannelGroup **)(this + 0x8b8),
                          (ChannelGroupProperties *)this_00,true,(BaseChannelGroup *)0x0);
    if (iVar3 != 0) {
      return iVar3;
    }
    m_audio_b_FWBaseProp::EstablishVolumeControl
              (this_00,pIVar1,0x6f757470,0,0,&local_68,-0x80,0,-0x40,0);
    m_audio_b_FWBaseArrayProp::EstablishVolumeControls
              ((m_audio_b_FWBaseArrayProp *)this_00,pIVar1,0x6f757470,1,-0x80,0,-0x40,0);
    this_01 = this + 0x1a8;
  }
  m_audio_b_FWBaseArrayProp::EstablishMuteControls
            ((m_audio_b_FWBaseArrayProp *)this_01,pIVar1,0x6f757470,1);
LAB_00022096:
  if (param_4 != (BaseChannelGroup *)0x0) {
    lVar2 = *(long *)(this + 0x8c0);
    *(undefined4 *)(lVar2 + 0x14) = 0xffffff01;
    *(undefined4 *)(lVar2 + 0x18) = 0;
    m_audio_FWb::
    TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
    ::Init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
              ((FWSettingsLevels *)(*(long *)(this + 0x8c0) + 0x20),(_func_int_uint_uint *)this);
    iVar3 = InitGroupProp(this,"Input",*(BaseChannelGroup **)(this + 0x8c0),
                          (ChannelGroupProperties *)(this + 0x310),true,
                          *(BaseChannelGroup **)(this + 0x8b0));
    if (iVar3 != 0) {
      return iVar3;
    }
    m_audio_b_FWBaseArrayProp::EstablishVolumeControls
              ((m_audio_b_FWBaseArrayProp *)(this + 0x310),pIVar1,0x696e7074,1,-0x80,0,-0x40,0);
    m_audio_b_FWBaseArrayProp::EstablishMuteControls
              ((m_audio_b_FWBaseArrayProp *)(this + 0x388),pIVar1,0x696e7074,1);
    local_50 = 0x59;
    local_48 = 0;
    uVar4 = (**(code **)(**(long **)(this + 0x8c0) + 0x10))();
    iVar3 = m_audio_b_FWBaseArrayProp::
            init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
                      ((m_audio_b_FWPropertyDict *)(this + 0x10),*(char **)(this + 0xa50),0x2f292,
                       (FWSettingsLevels *)(ulong)uVar4,(_func_int_uint_uint *)this);
    if (iVar3 != 0) {
      return iVar3;
    }
    *(undefined4 *)(this + 0x11c) = 1;
    iVar3 = m_audio_b_FW32BitNumberProp::
            init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int)>
                      ((m_audio_b_FW32BitNumberProp *)(this + 0xf0),
                       *(m_audio_b_FWPropertyDict **)(this + 0xa50),"MADirectMonitoringActive",this,
                       SetDirectMonitoringActive);
    if (iVar3 != 0) {
      return iVar3;
    }
    local_40 = 0x69;
    local_38 = 0;
    uVar4 = (**(code **)(**(long **)(this + 0x8c0) + 0x10))();
    iVar3 = m_audio_b_FWBaseArrayProp::
            init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
                      ((m_audio_b_FWPropertyDict *)(this + 0x990),*(char **)(this + 0xa50),0x2f010,
                       (FWSettingsLevels *)(ulong)(uVar4 >> 1),(_func_int_uint_uint *)this);
    if (iVar3 != 0) {
      return iVar3;
    }
  }
  if (param_5 != (BaseChannelGroup *)0x0) {
    lVar2 = *(long *)(this + 0x8c8);
    *(undefined4 *)(lVar2 + 0x14) = 0xffffff01;
    *(undefined4 *)(lVar2 + 0x18) = 0;
    m_audio_FWb::
    TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
    ::Init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
              ((FWSettingsLevels *)(*(long *)(this + 0x8c8) + 0x20),(_func_int_uint_uint *)this);
    iVar3 = InitGroupProp(this,"Headphone",*(BaseChannelGroup **)(this + 0x8c8),
                          (ChannelGroupProperties *)(this + 0x6d0),false,(BaseChannelGroup *)0x0);
    if (iVar3 != 0) {
      return iVar3;
    }
  }
  iVar3 = 0;
  if (*(BaseChannelGroup **)(this + 0x8b0) != (BaseChannelGroup *)0x0 ||
      *(BaseChannelGroup **)(this + 0x8c0) != (BaseChannelGroup *)0x0) {
    m_audio_FWb::GroupChannelGroup::Init
              ((GroupChannelGroup *)(this + 0x8d0),*(BaseChannelGroup **)(this + 0x8b0),
               *(BaseChannelGroup **)(this + 0x8c0),(BaseChannelGroup *)0x0);
  }
  pBVar5 = *(BaseChannelGroup **)(this + 0x8b8);
  if ((pBVar5 != (BaseChannelGroup *)0x0 || param_6 != (BaseChannelGroup *)0x0) ||
     (*(BaseChannelGroup **)(this + 0x8c8) != (BaseChannelGroup *)0x0)) {
    m_audio_FWb::GroupChannelGroup::Init
              ((GroupChannelGroup *)(this + 0x930),pBVar5,param_6,
               *(BaseChannelGroup **)(this + 0x8c8));
    pBVar5 = *(BaseChannelGroup **)(this + 0x8b8);
  }
  if (pBVar5 != (BaseChannelGroup *)0x0) {
    local_78 = (IOAudioControl *)0x0;
    iVar3 = m_audio_b_FW32BitNumberProp::
            init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int)>
                      ((m_audio_b_FW32BitNumberProp *)(this + 0xb0),
                       *(m_audio_b_FWPropertyDict **)(this + 0xa50),"MAGlobalDim",this,SetGlobalDim)
    ;
    if (iVar3 == 0) {
      iVar3 = m_audio_b_FW32BitNumberProp::
              init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int)>
                        ((m_audio_b_FW32BitNumberProp *)(this + 0x70),
                         *(m_audio_b_FWPropertyDict **)(this + 0xa50),"MAGlobalMute",this,
                         SetGlobalMute);
      if (iVar3 == 0) {
        m_audio_b_FWBaseProp::EstablishMuteControl
                  ((m_audio_b_FWBaseProp *)(this + 0x70),pIVar1,0x6f757470,0,0,&local_78);
      }
    }
  }
  return iVar3;
}
