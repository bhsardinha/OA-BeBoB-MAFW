// com_m_audio_FW1814Device::InitializeUserSettableProperties
// Entry: 0000e6ca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::InitializeUserSettableProperties() */

ulong __thiscall
com_m_audio_FW1814Device::InitializeUserSettableProperties(com_m_audio_FW1814Device *this)

{
  com_m_audio_FW1814Device *pcVar1;
  com_m_audio_FW1814Device *pcVar2;
  com_m_audio_FW1814Device *pcVar3;
  com_m_audio_FW1814Device *pcVar4;
  BaseChannelGroup *pBVar5;
  uint uVar6;
  ulong uVar7;
  undefined8 *puVar8;
  int iVar9;
  com_m_audio_FW1814Device *pcVar10;
  
  uVar7 = (*(code *)PTR_InitializeUserSettableProperties_0003c1c8)(this);
  if ((int)uVar7 == 0) {
    pcVar1 = this + 0x1b00;
    pcVar2 = this + 0x1668;
    pcVar3 = this + 0x1318;
    pcVar4 = this + 0x13e0;
    pBVar5 = (BaseChannelGroup *)(this + 0x1730);
    uVar7 = (*(code *)PTR_Init_00048df8)(this + 0x470,this,pcVar2,pcVar3,pcVar4,pBVar5,pcVar1);
    if ((int)uVar7 == 0) {
      uVar6 = (**(code **)(*(long *)(this + 0x1318) + 0x10))(pcVar3);
      uVar7 = FWSettingsAuxSend::Init
                        ((FWSettingsAuxSend *)(this + 0xf98),(m_audio_b_FWAVCAudioDevice *)this,
                         (BaseChannelGroup *)(this + 0x17f8),(BaseChannelGroup *)pcVar1,
                         (uVar6 >> 1) + 1);
      if ((int)uVar7 == 0) {
        pcVar10 = pcVar3;
        uVar7 = FWSettingsLevels::Init
                          ((FWSettingsLevels *)(this + 0x538),(m_audio_b_FWAVCAudioDevice *)this,
                           (BaseChannelGroup *)pcVar2,(BaseChannelGroup *)pcVar3,
                           (BaseChannelGroup *)pcVar4,pBVar5,(BaseChannelGroup *)pcVar1);
        iVar9 = (int)pcVar10;
        if ((int)uVar7 == 0) {
          m_audio_FWb::BaseChannelGroup::
          SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                    (pcVar3,(_func_int_uint_uint_void_ptr *)this,8,iVar9);
          m_audio_FWb::BaseChannelGroup::
          SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                    (pcVar4,(_func_int_uint_uint_void_ptr *)this,0x10,iVar9);
          m_audio_FWb::BaseChannelGroup::
          SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                    (pcVar2,(_func_int_uint_uint_void_ptr *)this,0,iVar9);
          m_audio_FWb::BaseChannelGroup::
          SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                    ((com_m_audio_FW1814Device *)pBVar5,(_func_int_uint_uint_void_ptr *)this,0x38,
                     iVar9);
          *(undefined4 *)(this + 0x183c) = 0x80;
          *(undefined4 *)(this + 0x1838) = 100;
          if (*(void **)(this + 0x1830) != (void *)0x0) {
            operator_delete(*(void **)(this + 0x1830));
          }
          if (this == (com_m_audio_FW1814Device *)0x0) {
            uRam0000000000001830 = 0;
            m_audio_FWb::BaseChannelGroup::
            SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                      ((com_m_audio_FW1814Device *)0x1b00,(_func_int_uint_uint_void_ptr *)0x0,0x34,
                       iVar9);
          }
          else {
            puVar8 = operator_new(0x20);
            if (puVar8 != (undefined8 *)0x0) {
              *puVar8 = &PTR_Call_00036830;
              puVar8[1] = this;
              iVar9 = 0xe982;
              puVar8[2] = CustomMixerWriteWithRotatedAuxChannels;
              puVar8[3] = 0;
            }
            *(undefined8 **)(this + 0x1830) = puVar8;
            m_audio_FWb::BaseChannelGroup::
            SetupBatchChangeMethod<com_m_audio_FW1814Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                      (pcVar1,(_func_int_uint_uint_void_ptr *)this,0x34,iVar9);
            if (this != (com_m_audio_FW1814Device *)0x0) {
              uVar6 = (**(code **)(*(long *)(this + 0x2d8) + 0x10))
                                ((FWSettingsABSwitch *)(this + 0x2d8),this);
              if (uVar6 != 0) {
                return (ulong)uVar6;
              }
              if (*(void **)(this + 0x2f0) != (void *)0x0) {
                operator_delete(*(void **)(this + 0x2f0));
              }
              if (this == (com_m_audio_FW1814Device *)0x0) {
                section_000002e0.segname[0] = '\0';
                section_000002e0.segname[1] = '\0';
                section_000002e0.segname[2] = '\0';
                section_000002e0.segname[3] = '\0';
                section_000002e0.segname[4] = '\0';
                section_000002e0.segname[5] = '\0';
                section_000002e0.segname[6] = '\0';
                section_000002e0.segname[7] = '\0';
              }
              else {
                puVar8 = operator_new(0x20);
                if (puVar8 == (undefined8 *)0x0) {
                  *(undefined8 *)(this + 0x2f0) = 0;
                }
                else {
                  *puVar8 = &PTR_Call_00036850;
                  puVar8[1] = this;
                  puVar8[2] = 0xbe9;
                  puVar8[3] = 0;
                  *(undefined8 **)(this + 0x2f0) = puVar8;
                  if (*(void **)(this + 0x300) != (void *)0x0) {
                    operator_delete(*(void **)(this + 0x300));
                  }
                  puVar8 = operator_new(0x20);
                  if (puVar8 != (undefined8 *)0x0) {
                    *puVar8 = &PTR_Call_00036870;
                    puVar8[1] = this;
                    puVar8[2] = SetHeadphoneSelector;
                    puVar8[3] = 0;
                    *(undefined8 **)(this + 0x300) = puVar8;
                    FWSettingsABSwitch::UpdateLEDs((FWSettingsABSwitch *)(this + 0x2d8));
                    return 0;
                  }
                  *(undefined8 *)(this + 0x300) = 0;
                }
              }
              return 0xe00002c2;
            }
          }
                    /* WARNING: Subroutine does not return */
          _panic(
                "\"device is NULL\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/Settings/FWSettingsABSwitch.h:75"
                );
        }
      }
    }
  }
  return uVar7;
}
