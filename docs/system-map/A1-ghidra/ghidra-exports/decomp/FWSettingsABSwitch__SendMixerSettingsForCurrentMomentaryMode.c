// FWSettingsABSwitch::SendMixerSettingsForCurrentMomentaryMode
// Entry: 0001e36a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SendMixerSettingsForCurrentMomentaryMode() */

void __thiscall
FWSettingsABSwitch::SendMixerSettingsForCurrentMomentaryMode(FWSettingsABSwitch *this)

{
  OSMetaClass *pOVar1;
  undefined4 uVar2;
  uint uVar3;
  OSMetaClassBase *pOVar4;
  long *plVar5;
  long *plVar6;
  long *plVar7;
  OSObject *pOVar8;
  uint uVar9;
  OSObject *local_40;
  long *local_38;
  
  if (*(int *)(this + 0x5c) == 0) {
    uVar2 = m_audio_b_FWBaseArrayProp::Get
                      ((m_audio_b_FWBaseArrayProp *)(this + 0x70),*(uint *)(this + 0xd0));
    *(undefined4 *)(this + 0x144) = uVar2;
    SubmitHeadphoneSourceChange(this);
  }
  else {
    local_40 = (OSObject *)0x0;
    local_38 = (long *)0x0;
    pOVar4 = (OSMetaClassBase *)
             m_audio_b_FWPropertyDict::CopyProperty
                       (*(m_audio_b_FWPropertyDict **)(*(long *)(this + 8) + 0xf8),
                        "MAInputRoutingV2");
    plVar5 = (long *)OSMetaClassBase::safeMetaCast(pOVar4,*(OSMetaClass **)PTR_metaClass_00030078);
    if (plVar5 == (long *)0x0) {
      uVar9 = 0;
      uVar3 = 0;
    }
    else {
      uVar3 = 0;
      pOVar4 = (OSMetaClassBase *)(**(code **)(*plVar5 + 0x1e8))(plVar5,0);
      pOVar1 = *(OSMetaClass **)PTR_metaClass_00030080;
      plVar6 = (long *)OSMetaClassBase::safeMetaCast(pOVar4,pOVar1);
      pOVar4 = (OSMetaClassBase *)(**(code **)(*plVar5 + 0x1e8))(plVar5,1);
      plVar7 = (long *)OSMetaClassBase::safeMetaCast(pOVar4,pOVar1);
      if (plVar6 != (long *)0x0) {
        uVar3 = (**(code **)(*plVar6 + 0x148))();
      }
      if (plVar7 == (long *)0x0) {
        uVar9 = 0;
      }
      else {
        uVar9 = (**(code **)(*plVar7 + 0x148))(plVar7);
      }
      (**(code **)(*plVar5 + 0x28))(plVar5);
    }
    if (*(int *)(this + 0xd0) == 0) {
      local_40 = (OSObject *)OSNumber::withNumber((ulong)(uVar3 & 0xfffffffe),0x20);
      uVar9 = uVar9 & 0xffffffed;
    }
    else {
      local_40 = (OSObject *)OSNumber::withNumber((ulong)(uVar3 | 1),0x20);
      uVar9 = uVar9 | 0x12;
    }
    local_38 = (long *)OSNumber::withNumber((ulong)uVar9,0x20);
    pOVar8 = (OSObject *)OSArray::withObjects(&local_40,2,0);
    if (((local_40 != (OSObject *)0x0) && (local_38 != (long *)0x0)) && (pOVar8 != (OSObject *)0x0))
    {
      m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
                (*(m_audio_b_FWPropertyDict **)(*(long *)(this + 8) + 0xf8),"MAInputRoutingV2",
                 pOVar8);
    }
    if (local_40 != (OSObject *)0x0) {
      (**(code **)(*(long *)local_40 + 0x28))();
    }
    if (local_38 != (long *)0x0) {
      (**(code **)(*local_38 + 0x28))();
    }
    if (pOVar8 != (OSObject *)0x0) {
      (**(code **)(*(long *)pOVar8 + 0x28))(pOVar8);
    }
  }
  return;
}
