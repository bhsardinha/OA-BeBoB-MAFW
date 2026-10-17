=== FWSettings1814Levels::FWSettings1814Levels @ 0000eec0 ===

/* FWSettings1814Levels::FWSettings1814Levels() */

void __thiscall FWSettings1814Levels::FWSettings1814Levels(FWSettings1814Levels *this)

{
  *(undefined ***)this = &PTR_SendToDevice_00048e60;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x10));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x70));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xb0));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xf0));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x130));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x310));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x4f0));
  FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties
            ((ChannelGroupProperties *)(this + 0x6d0));
  *(undefined ***)(this + 0x8f0) = &PTR__TFunctor_00036750;
  *(undefined8 *)(this + 0x8f8) = 0;
  *(undefined ***)(this + 0x900) = &PTR__TFunctor_00036780;
  *(undefined8 *)(this + 0x908) = 0;
  this[0x8e2] = (FWSettings1814Levels)0x0;
  *(undefined ***)(this + 0x8d0) = &PTR__GroupChannelGroup_00047fc0;
  *(undefined ***)(this + 0x950) = &PTR__TFunctor_00036750;
  *(undefined8 *)(this + 0x958) = 0;
  *(undefined ***)(this + 0x960) = &PTR__TFunctor_00036780;
  *(undefined8 *)(this + 0x968) = 0;
  this[0x942] = (FWSettings1814Levels)0x0;
  *(undefined ***)(this + 0x930) = &PTR__GroupChannelGroup_00047fc0;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x990));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x9f0));
  *(undefined ***)this = &PTR_SendToDevice_000366c0;
  return;
}


=== FWSettings1814AuxSend::FWSettings1814AuxSend @ 0000efda ===

/* FWSettings1814AuxSend::FWSettings1814AuxSend() */

void __thiscall FWSettings1814AuxSend::FWSettings1814AuxSend(FWSettings1814AuxSend *this)

{
  *(undefined ***)this = &PTR_SendToDevice_00048d40;
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x20));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x98));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x110));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x188));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x200));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x260));
  *(undefined ***)this = &PTR_SendToDevice_00036670;
  return;
}


=== FWSettings1814Levels::SetInputPan @ 0000d128 ===

/* FWSettings1814Levels::SetInputPan(unsigned int, unsigned int) */

void __thiscall
FWSettings1814Levels::SetInputPan(FWSettings1814Levels *this,uint param_1,uint param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW1814Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x0000d15a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0xbc0))(plVar1,param_1,param_2,*(code **)(*plVar1 + 0xbc0));
  return;
}


=== FWSettings1814Levels::SetInputRouting @ 0000d15c ===

/* FWSettings1814Levels::SetInputRouting(unsigned int, unsigned int) */

void __thiscall
FWSettings1814Levels::SetInputRouting(FWSettings1814Levels *this,uint param_1,uint param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW1814Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x0000d18e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 3000))(plVar1,param_1,param_2,*(code **)(*plVar1 + 3000));
  return;
}


=== FWSettings1814AuxSend::SetAuxMonitor @ 0000d0b4 ===

/* FWSettings1814AuxSend::SetAuxMonitor(unsigned int, unsigned int) */

void __thiscall
FWSettings1814AuxSend::SetAuxMonitor(FWSettings1814AuxSend *this,uint param_1,uint param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW1814Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x0000d0e6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0xbc8))(plVar1,param_1,param_2,*(code **)(*plVar1 + 0xbc8));
  return;
}


=== m_audio_b_FWChannelGroupProperty::init @ 000093a4 ===

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


=== m_audio_b_FWChannelGroupProperty::CallSetter @ 0000940e ===

/* m_audio_b_FWChannelGroupProperty::CallSetter(OSSymbol const*, OSObject*) */

int __thiscall
m_audio_b_FWChannelGroupProperty::CallSetter
          (m_audio_b_FWChannelGroupProperty *this,OSSymbol *param_1,OSObject *param_2)

{
  long *plVar1;
  int iVar2;
  void *pvVar3;
  void *pvVar4;
  
  pvVar3 = (void *)m_audio_FWb::BaseChannelGroup::AdjustedLevels
                             (*(BaseChannelGroup **)(this + 0x60));
  if (pvVar3 == (void *)0x0) {
    iVar2 = 0;
  }
  else {
    if (*(BaseChannelGroup **)(this + 0x70) == (BaseChannelGroup *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      pvVar4 = (void *)m_audio_FWb::BaseChannelGroup::AdjustedLevels
                                 (*(BaseChannelGroup **)(this + 0x70));
    }
    iVar2 = m_audio_b_FWBaseArrayProp::OSArrayToSeriatimValues
                      ((m_audio_b_FWBaseArrayProp *)this,param_2,*(uint *)(this + 0x2c));
    if (iVar2 == 0) {
      (**(code **)(**(long **)(this + 0x60) + 0x28))(*(long **)(this + 0x60),pvVar3);
      operator_delete__(pvVar3);
      plVar1 = *(long **)(this + 0x70);
      if (plVar1 != (long *)0x0) {
        (**(code **)(*plVar1 + 0x28))(plVar1,pvVar4);
        if (pvVar4 != (void *)0x0) {
          operator_delete__(pvVar4);
        }
      }
    }
  }
  return iVar2;
}


=== m_audio_FWb::BaseChannelGroup::SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)> @ 0000e184 ===

/* void m_audio_FWb::BaseChannelGroup::SetupBatchChangeMethod<com_m_audio_FW1814Device*, int
   (m_audio_b_FWAVCAudioDevice::*)(unsigned int, unsigned int, void*)>(com_m_audio_FW1814Device*,
   int (m_audio_b_FWAVCAudioDevice::*)(unsigned int, unsigned int, void*), unsigned int, int) */

void m_audio_FWb::BaseChannelGroup::
     SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
               (com_m_audio_FW1814Device *param_1,_func_int_uint_uint_void_ptr *param_2,uint param_3
               ,int param_4)

{
  undefined8 *puVar1;
  
  *(undefined4 *)(param_1 + 0x44) = 0x80;
  *(uint *)(param_1 + 0x40) = param_3;
  if (*(void **)(param_1 + 0x38) != (void *)0x0) {
    operator_delete(*(void **)(param_1 + 0x38));
  }
  if (param_2 == (_func_int_uint_uint_void_ptr *)0x0) {
    puVar1 = (undefined8 *)0x0;
  }
  else {
    puVar1 = operator_new(0x20);
    if (puVar1 != (undefined8 *)0x0) {
      *puVar1 = &PTR_Call_00036810;
      puVar1[1] = param_2;
      puVar1[2] = 0xba9;
      puVar1[3] = 0;
    }
  }
  *(undefined8 **)(param_1 + 0x38) = puVar1;
  return;
}


=== m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup @ 0000b3b4 ===

/* m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup() */

void __thiscall
m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup(ConcreteChannelGroup<6u> *this)

{
  long lVar1;
  
  *(undefined ***)(this + 0x20) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x28) = 0;
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x38) = 0;
  this[0x12] = (ConcreteChannelGroup<6u>)0x0;
  *(undefined ***)this = &PTR__ConcreteChannelGroup_00035430;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x48) = 0;
    *(undefined8 *)(this + lVar1 + 0x50) = 0;
    *(undefined8 *)(this + lVar1 + 0x58) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0xc0);
  this[0x12] = (ConcreteChannelGroup<6u>)0x1;
  *(undefined8 *)(this + 8) = 0;
  this[0x10] = (ConcreteChannelGroup<6u>)0x0;
  this[0x11] = (ConcreteChannelGroup<6u>)0x0;
  return;
}


=== m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup @ 0000b432 ===

/* m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup() */

void __thiscall
m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup(ConcreteChannelGroup<2u> *this)

{
  long lVar1;
  
  *(undefined ***)(this + 0x20) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x28) = 0;
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x38) = 0;
  this[0x12] = (ConcreteChannelGroup<2u>)0x0;
  *(undefined ***)this = &PTR__ConcreteChannelGroup_00035390;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x48) = 0;
    *(undefined8 *)(this + lVar1 + 0x50) = 0;
    *(undefined8 *)(this + lVar1 + 0x58) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x40);
  this[0x12] = (ConcreteChannelGroup<2u>)0x1;
  *(undefined8 *)(this + 8) = 0;
  this[0x10] = (ConcreteChannelGroup<2u>)0x0;
  this[0x11] = (ConcreteChannelGroup<2u>)0x0;
  return;
}


=== FWSettingsLevels::FWSettingsLevels @ 0000bd5a ===

/* FWSettingsLevels::FWSettingsLevels() */

void __thiscall FWSettingsLevels::FWSettingsLevels(FWSettingsLevels *this)

{
  *(undefined ***)this = &PTR_SendToDevice_00048e60;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x10));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x70));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xb0));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0xf0));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x130));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x310));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x4f0));
  ChannelGroupProperties::ChannelGroupProperties((ChannelGroupProperties *)(this + 0x6d0));
  *(undefined ***)(this + 0x8f0) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x8f8) = 0;
  *(undefined ***)(this + 0x900) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x908) = 0;
  this[0x8e2] = (FWSettingsLevels)0x0;
  *(undefined ***)(this + 0x8d0) = &PTR__GroupChannelGroup_00047fc0;
  *(undefined ***)(this + 0x950) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x958) = 0;
  *(undefined ***)(this + 0x960) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x968) = 0;
  this[0x942] = (FWSettingsLevels)0x0;
  *(undefined ***)(this + 0x930) = &PTR__GroupChannelGroup_00047fc0;
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x990));
  m_audio_b_FWArrayProp::m_audio_b_FWArrayProp((m_audio_b_FWArrayProp *)(this + 0x9f0));
  return;
}


=== FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties @ 0000bd1c ===

/* FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties() */

void __thiscall
FWSettingsLevels::ChannelGroupProperties::ChannelGroupProperties(ChannelGroupProperties *this)

{
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)this);
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x78));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0xf0));
  m_audio_b_FWChannelGroupProperty::m_audio_b_FWChannelGroupProperty
            ((m_audio_b_FWChannelGroupProperty *)(this + 0x168));
  return;
}


=== m_audio_FWb::ConcreteChannelGroup<2u>::ChannelAtIndex @ 0000ac9e ===

/* m_audio_FWb::ConcreteChannelGroup<2u>::ChannelAtIndex(unsigned int) const */

ConcreteChannelGroup<2u> * __thiscall
m_audio_FWb::ConcreteChannelGroup<2u>::ChannelAtIndex(ConcreteChannelGroup<2u> *this,uint param_1)

{
  if (param_1 < 2) {
    return this + (ulong)param_1 * 0x20 + 0x48;
  }
  return this + 0x48;
}


INFO  REPORT: Save succeeded for processed file: /M-AudioFireWireBeBoB.x86_64-slice (HeadlessAnalyzer)  
