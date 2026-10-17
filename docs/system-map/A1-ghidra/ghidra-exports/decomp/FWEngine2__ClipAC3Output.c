// com_m_audio_FWEngine2::ClipAC3Output
// Entry: 00011c44
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::ClipAC3Output(float*, int*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*) */

undefined8 __thiscall
com_m_audio_FWEngine2::ClipAC3Output
          (com_m_audio_FWEngine2 *this,float *param_1,int *param_2,uint param_3,uint param_4,
          _IOAudioStreamFormat *param_5)

{
  int iVar1;
  undefined8 uVar2;
  void *pvVar3;
  uint uVar4;
  uint uVar5;
  uint local_34;
  
  iVar1 = (**(code **)(*(long *)this + 0xc68))(this);
  if ((((*(uint *)(this + 0x2d8) < (param_4 + param_3) * 4) ||
       (param_5[0x10] != (_IOAudioStreamFormat)0x0)) || (*(int *)param_5 != 2)) ||
     (((0x2000 < param_4 + param_3 || (param_1 == (float *)0x0)) || (param_2 == (int *)0x0)))) {
    uVar2 = 0xe00002bc;
  }
  else {
    iVar1 = iVar1 + *(int *)(this + 0x2e8);
    LocalRearrangeSampleFrame16Bit2
              ((ushort *)this,(ushort *)param_1,(int *)((long)param_1 + (ulong)(param_4 * 2) * 2),
               (uint)param_2,param_4,SUB41(iVar1,0));
    if (param_4 != 0) {
      uVar5 = 0;
      do {
        pvVar3 = (void *)m_audio_b_FWDCLOutputProgram::GetPacketFrameContainingSampleIndex
                                   (*(m_audio_b_FWDCLOutputProgram **)(this + 0x350),uVar5 + param_3
                                    ,*(uint *)(this + 0x224),&local_34,(int **)0x0,(uint *)0x0);
        uVar4 = local_34;
        if (param_4 < local_34 + uVar5) {
          uVar4 = local_34 + (uVar5 - param_4);
        }
        _memcpy(pvVar3,param_2 + uVar5 * iVar1,(ulong)(uVar4 * iVar1) << 2);
        uVar5 = uVar5 + uVar4;
      } while (uVar5 < param_4);
    }
    uVar2 = 0;
  }
  return uVar2;
}
