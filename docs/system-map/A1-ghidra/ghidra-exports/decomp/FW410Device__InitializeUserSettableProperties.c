// com_m_audio_FW410Device::InitializeUserSettableProperties
// Entry: 00014fca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW410Device::InitializeUserSettableProperties() */

undefined8 __thiscall
com_m_audio_FW410Device::InitializeUserSettableProperties(com_m_audio_FW410Device *this)

{
  BaseChannelGroup *pBVar1;
  com_m_audio_FW410Device *pcVar2;
  com_m_audio_FW410Device *pcVar3;
  com_m_audio_FW410Device *pcVar4;
  uint uVar5;
  undefined8 uVar6;
  undefined8 *puVar7;
  int iVar8;
  com_m_audio_FW410Device *pcVar9;
  
  uVar6 = (*(code *)PTR_InitializeUserSettableProperties_0003b4d8)(this);
  if ((int)uVar6 == 0) {
    pBVar1 = (BaseChannelGroup *)(this + 0x17f0);
    pcVar2 = this + 0x13d8;
    pcVar3 = this + 0x1188;
    pcVar4 = this + 0x1310;
    pcVar9 = pcVar3;
    uVar6 = (*(code *)PTR_Init_00048df8)
                      (this + 0x2d8,this,pcVar2,pcVar3,pcVar4,(BaseChannelGroup *)(this + 0x1560),
                       pBVar1);
    iVar8 = (int)pcVar9;
    if ((int)uVar6 == 0) {
      m_audio_FWb::BaseChannelGroup::
      SetupBatchChangeMethod<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                (pcVar2,(_func_int_uint_uint_void_ptr *)this,0,iVar8);
      m_audio_FWb::BaseChannelGroup::
      SetupBatchChangeMethod<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                (pcVar3,(_func_int_uint_uint_void_ptr *)this,0x14,iVar8);
      m_audio_FWb::BaseChannelGroup::
      SetupBatchChangeMethod<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
                (pcVar4,(_func_int_uint_uint_void_ptr *)this,0x28,iVar8);
      uVar5 = (**(code **)(*(long *)(this + 0x1188) + 0x10))(pcVar3);
      uVar6 = (*(code *)PTR_Init_00048d68)(this + 0x3a0,this,this + 0x15e8,pBVar1,(uVar5 >> 1) + 1);
      if (((int)uVar6 == 0) &&
         (uVar6 = FWSettings410Levels::Init
                            ((FWSettings410Levels *)(this + 0x678),
                             (m_audio_b_FWAVCAudioDevice *)this,(BaseChannelGroup *)pcVar2,
                             (BaseChannelGroup *)pcVar3,(BaseChannelGroup *)pcVar4,
                             (BaseChannelGroup *)(this + 0x1560),pBVar1), (int)uVar6 == 0)) {
        if (this != (com_m_audio_FW410Device *)0x0) {
          uVar6 = *(undefined8 *)(this + 0xf8);
          if (*(void **)(this + 0x1180) != (void *)0x0) {
            operator_delete(*(void **)(this + 0x1180));
          }
          puVar7 = operator_new(0x20);
          if (puVar7 != (undefined8 *)0x0) {
            *puVar7 = &PTR_Call_0003c500;
            puVar7[1] = this;
            puVar7[2] = SetHeadphoneSource;
            puVar7[3] = 0;
            *(undefined8 **)(this + 0x1180) = puVar7;
            uVar6 = (*(code *)PTR_init_00037a68)(this + 0x1148,uVar6,"MAHeadphoneSourceV2");
            if ((int)uVar6 != 0) {
              return uVar6;
            }
                    /* WARNING: Could not recover jumptable at 0x00015195. Too many branches */
                    /* WARNING: Treating indirect jump as call */
            uVar6 = (**(code **)(*(long *)this + 0xba8))(this,0x28,2,this + 0x1100);
            return uVar6;
          }
          *(undefined8 *)(this + 0x1180) = 0;
        }
        uVar6 = 0xe00002bc;
      }
    }
  }
  return uVar6;
}
