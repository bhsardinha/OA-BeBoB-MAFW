// com_m_audio_FWAudiophileDevice::InitializeUserSettableProperties
// Entry: 0000b7e6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FWAudiophileDevice::InitializeUserSettableProperties() */

ulong __thiscall
com_m_audio_FWAudiophileDevice::InitializeUserSettableProperties
          (com_m_audio_FWAudiophileDevice *this)

{
  com_m_audio_FWAudiophileDevice *pcVar1;
  com_m_audio_FWAudiophileDevice *pcVar2;
  uint uVar3;
  ulong uVar4;
  undefined8 *puVar5;
  
  uVar4 = (*(code *)PTR_InitializeUserSettableProperties_0003b4d8)(this);
  if ((int)uVar4 == 0) {
    pcVar1 = this + 0x1758;
    pcVar2 = this + 0x1270;
    uVar4 = (*(code *)PTR_Init_00048df8)
                      (this + 0x468,this,this + 0x1440,pcVar2,this + 0x1378,this + 0x1548,pcVar1);
    if ((int)uVar4 == 0) {
      uVar3 = (**(code **)(*(long *)(this + 0x1270) + 0x10))(pcVar2);
      uVar4 = (*(code *)PTR_Init_00048d68)(this + 0x530,this,this + 0x15d0,pcVar1,uVar3 >> 1);
      if (((int)uVar4 == 0) &&
         (uVar4 = (*(code *)PTR_Init_00048e88)
                            (this + 0x808,this,this + 0x1440,pcVar2,this + 0x1378,this + 0x1548,
                             pcVar1), (int)uVar4 == 0)) {
        if (this == (com_m_audio_FWAudiophileDevice *)0x0) {
                    /* WARNING: Subroutine does not return */
          _panic(
                "\"device is NULL\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/Settings/FWSettingsABSwitch.h:75"
                );
        }
        uVar3 = (**(code **)(*(long *)(this + 0x2d0) + 0x10))
                          ((FWSettingsABSwitch *)(this + 0x2d0),this);
        uVar4 = (ulong)uVar3;
        if (uVar3 == 0) {
          if (*(void **)(this + 0x2e8) != (void *)0x0) {
            operator_delete(*(void **)(this + 0x2e8));
          }
          if (this == (com_m_audio_FWAudiophileDevice *)0x0) {
            section_000002e0.sectname[8] = '\0';
            section_000002e0.sectname[9] = '\0';
            section_000002e0.sectname[10] = '\0';
            section_000002e0.sectname[0xb] = '\0';
            section_000002e0.sectname[0xc] = '\0';
            section_000002e0.sectname[0xd] = '\0';
            section_000002e0.sectname[0xe] = '\0';
            section_000002e0.sectname[0xf] = '\0';
          }
          else {
            puVar5 = operator_new(0x20);
            if (puVar5 == (undefined8 *)0x0) {
              *(undefined8 *)(this + 0x2e8) = 0;
            }
            else {
              *puVar5 = &PTR_Call_00035560;
              puVar5[1] = this;
              puVar5[2] = 0xbc9;
              puVar5[3] = 0;
              *(undefined8 **)(this + 0x2e8) = puVar5;
              if (*(void **)(this + 0x2f8) != (void *)0x0) {
                operator_delete(*(void **)(this + 0x2f8));
              }
              puVar5 = operator_new(0x20);
              if (puVar5 != (undefined8 *)0x0) {
                *puVar5 = &PTR_Call_00035580;
                puVar5[1] = this;
                puVar5[2] = SetHeadphoneSelector;
                puVar5[3] = 0;
                *(undefined8 **)(this + 0x2f8) = puVar5;
                FWSettingsABSwitch::UpdateLEDs((FWSettingsABSwitch *)(this + 0x2d0));
                return 0;
              }
              *(undefined8 *)(this + 0x2f8) = 0;
            }
          }
          uVar4 = 0xe00002c2;
        }
      }
    }
  }
  return uVar4;
}
