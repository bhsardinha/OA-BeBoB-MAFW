// FWSettingsRotaryControl::Init
// Entry: 00020ed2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsRotaryControl::Init(m_audio_b_FWAudioDevice*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*) */

undefined8 __thiscall
FWSettingsRotaryControl::Init
          (FWSettingsRotaryControl *this,m_audio_b_FWAudioDevice *param_1,BaseChannelGroup *param_2,
          BaseChannelGroup *param_3,BaseChannelGroup *param_4,BaseChannelGroup *param_5,
          BaseChannelGroup *param_6)

{
  undefined8 uVar1;
  undefined8 *puVar2;
  
  uVar1 = (*(code *)PTR_Init_000470c0)(this,param_1);
  if ((int)uVar1 == 0) {
    *(BaseChannelGroup **)(this + 0x10) = param_2;
    *(BaseChannelGroup **)(this + 0x18) = param_3;
    *(BaseChannelGroup **)(this + 0x20) = param_4;
    *(BaseChannelGroup **)(this + 0x28) = param_5;
    *(BaseChannelGroup **)(this + 0x30) = param_6;
    uVar1 = *(undefined8 *)(param_1 + 0xf8);
    *(undefined8 *)(this + 0xc0) = uVar1;
    if (param_1 != (m_audio_b_FWAudioDevice *)0x0) {
      if (*(void **)(this + 0x78) != (void *)0x0) {
        operator_delete(*(void **)(this + 0x78));
      }
      puVar2 = operator_new(0x20);
      if (puVar2 == (undefined8 *)0x0) {
        *(undefined8 *)(this + 0x78) = 0;
      }
      else {
        *puVar2 = &PTR_Call_00048d90;
        puVar2[1] = param_1;
        puVar2[2] = m_audio_b_FWAudioDevice::NullSetProperty64;
        puVar2[3] = 0;
        *(undefined8 **)(this + 0x78) = puVar2;
        uVar1 = (*(code *)PTR_init_00037a68)(this + 0x38,uVar1,"MARotaryControlV2");
        if ((int)uVar1 != 0) {
          return uVar1;
        }
        uVar1 = *(undefined8 *)(this + 0xc0);
        if (*(void **)(this + 0xb8) != (void *)0x0) {
          operator_delete(*(void **)(this + 0xb8));
        }
        if (this == (FWSettingsRotaryControl *)0x0) {
          section_000000b8.sectname[0] = '\0';
          section_000000b8.sectname[1] = '\0';
          section_000000b8.sectname[2] = '\0';
          section_000000b8.sectname[3] = '\0';
          section_000000b8.sectname[4] = '\0';
          section_000000b8.sectname[5] = '\0';
          section_000000b8.sectname[6] = '\0';
          section_000000b8.sectname[7] = '\0';
        }
        else {
          puVar2 = operator_new(0x20);
          if (puVar2 != (undefined8 *)0x0) {
            *puVar2 = &PTR_Call_00048db0;
            puVar2[1] = this;
            puVar2[2] = VirtualRotateKnob;
            puVar2[3] = 0;
            *(undefined8 **)(this + 0xb8) = puVar2;
                    /* WARNING: Could not recover jumptable at 0x00021055. Too many branches */
                    /* WARNING: Treating indirect jump as call */
            uVar1 = (**(code **)(*(long *)(this + 0x80) + 0x148))
                              (this + 0x80,uVar1,"MAMetaVirtualRotateDataKnob");
            return uVar1;
          }
          *(undefined8 *)(this + 0xb8) = 0;
        }
      }
    }
    uVar1 = 0xe00002bc;
  }
  return uVar1;
}
