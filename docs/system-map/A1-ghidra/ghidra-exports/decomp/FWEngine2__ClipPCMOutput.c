// com_m_audio_FWEngine2::ClipPCMOutput
// Entry: 00011d72
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2b)

/* com_m_audio_FWEngine2::ClipPCMOutput(float*, int*, unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FWEngine2::ClipPCMOutput
          (com_m_audio_FWEngine2 *this,float *param_1,int *param_2,uint param_3,uint param_4)

{
  void *pvVar1;
  long lVar2;
  uint uVar3;
  int *piVar4;
  uint uVar5;
  uint local_40;
  uint local_3c;
  int *local_38;
  
  _Float32ToNativeInt32_X86(param_1,param_2,*(int *)(this + 0x2ec) * param_4);
  if (*(long *)(this + 0x308) != 0) {
    lVar2 = (*(long **)(this + 0x138))[0x22];
    uVar3 = (**(code **)(**(long **)(this + 0x138) + 0xa80))();
    pvVar1 = (void *)(lVar2 + (ulong)uVar3 * 4);
    uVar3 = (**(code **)(**(long **)(this + 0x138) + 0xa88))();
    _bzero(pvVar1,(ulong)uVar3 << 2);
    (**(code **)(this + 0x308))(param_2,pvVar1,param_4);
  }
  if (param_4 != 0) {
    uVar3 = 0;
    do {
      piVar4 = (int *)m_audio_b_FWDCLOutputProgram::GetPacketFrameContainingSampleIndex
                                (*(m_audio_b_FWDCLOutputProgram **)(this + 0x350),uVar3 + param_3,
                                 *(uint *)(this + 0x224),&local_3c,&local_38,&local_40);
      uVar5 = local_3c;
      if (param_4 < local_3c + uVar3) {
        uVar5 = local_3c + (uVar3 - param_4);
      }
      if (*(uint **)(this + 0x328) == (uint *)0x0) {
        if (*(code **)(this + 0x300) != (code *)0x0) {
          (**(code **)(this + 0x300))(param_2 + *(int *)(this + 0x2ec) * uVar3,piVar4,uVar5);
        }
      }
      else {
        RemapSampleFramesForOutput
                  (param_2 + *(uint *)(this + 0x2ec) * uVar3,piVar4,*(uint *)(this + 0x2e0),
                   *(uint *)(this + 0x2ec),uVar5,*(uint **)(this + 0x328),*(uint **)(this + 0x330),
                   *(int *)(this + 0x2e8) != 0);
      }
      uVar3 = uVar5 + uVar3;
    } while (uVar3 < param_4);
  }
  return 0;
}
