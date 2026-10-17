// FWSettingsABSwitch::Init
// Entry: 0001e00a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::Init(m_audio_b_FWAudioDevice*) */

int __thiscall FWSettingsABSwitch::Init(FWSettingsABSwitch *this,m_audio_b_FWAudioDevice *param_1)

{
  m_audio_b_FWPropertyDict *pmVar1;
  long lVar2;
  int iVar3;
  undefined8 *puVar4;
  
  *(m_audio_b_FWAudioDevice **)(this + 8) = param_1;
  pmVar1 = *(m_audio_b_FWPropertyDict **)(param_1 + 0xf8);
  if (pmVar1 == (m_audio_b_FWPropertyDict *)0x0) {
    return -0x1ffffd3e;
  }
  if (*(void **)(this + 0xa8) != (void *)0x0) {
    operator_delete(*(void **)(this + 0xa8));
  }
  if (this == (FWSettingsABSwitch *)0x0) {
    section_00000068.flags = 0;
    section_00000068.reserved1 = 0;
  }
  else {
    puVar4 = operator_new(0x20);
    if (puVar4 == (undefined8 *)0x0) {
      *(undefined8 *)(this + 0xa8) = 0;
    }
    else {
      *puVar4 = &PTR_Call_00047050;
      puVar4[1] = this;
      puVar4[2] = SetMomentarySwitchRoutingSource;
      puVar4[3] = 0;
      *(undefined8 **)(this + 0xa8) = puVar4;
      iVar3 = (**(code **)(*(long *)(this + 0x70) + 0x148))
                        (this + 0x70,pmVar1,"MAMomentarySwitchRoutings",4);
      if (iVar3 != 0) {
        return iVar3;
      }
      iVar3 = m_audio_b_FW32BitNumberProp::
              init<FWSettingsABSwitch*,int(FWSettingsABSwitch::*)(unsigned_int)>
                        ((m_audio_b_FW32BitNumberProp *)(this + 0x118),pmVar1,"MAHeadphoneSourceV2",
                         this,SetHeadphoneSource);
      if (iVar3 != 0) {
        return iVar3;
      }
      iVar3 = m_audio_b_FW32BitNumberProp::
              init<FWSettingsABSwitch*,int(FWSettingsABSwitch::*)(unsigned_int)>
                        ((m_audio_b_FW32BitNumberProp *)(this + 0x158),pmVar1,
                         "MAHeadphoneSecondSource",this,SetHeadphoneSecondSource);
      if (iVar3 != 0) {
        return iVar3;
      }
      iVar3 = m_audio_b_FW32BitNumberProp::
              init<FWSettingsABSwitch*,int(FWSettingsABSwitch::*)(unsigned_int)>
                        ((m_audio_b_FW32BitNumberProp *)(this + 0x30),pmVar1,
                         "MAMomentarySwitchSource",this,SetMomentarySwitchSource);
      if (iVar3 != 0) {
        return iVar3;
      }
      lVar2 = *(long *)(this + 8);
      if (lVar2 != 0) {
        if (*(void **)(this + 0x110) != (void *)0x0) {
          operator_delete(*(void **)(this + 0x110));
        }
        puVar4 = operator_new(0x20);
        if (puVar4 != (undefined8 *)0x0) {
          *puVar4 = &PTR_Call_00047090;
          puVar4[1] = lVar2;
          puVar4[2] = m_audio_b_FWAudioDevice::NullSetProperty32;
          puVar4[3] = 0;
          *(undefined8 **)(this + 0x110) = puVar4;
          iVar3 = (*(code *)PTR_init_00037a68)(this + 0xd8,pmVar1,"MAMomentarySwitchLatent");
          return iVar3;
        }
        *(undefined8 *)(this + 0x110) = 0;
      }
    }
  }
  return -0x1ffffd44;
}
