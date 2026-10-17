// m_audio_b_FWAVCUnit::AVCCommandWithErrorResponse
// Entry: 00001508
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::AVCCommandWithErrorResponse(unsigned char const*, unsigned int, unsigned
   char*, unsigned int*) */

void __thiscall
m_audio_b_FWAVCUnit::AVCCommandWithErrorResponse
          (m_audio_b_FWAVCUnit *this,uchar *param_1,uint param_2,uchar *param_3,uint *param_4)

{
  int iVar1;
  undefined4 in_register_00000014;
  uint uVar2;
  
  iVar1 = (**(code **)(*(long *)this + 0x8a8))
                    (this,param_1,CONCAT44(in_register_00000014,param_2),param_3);
  uVar2 = 0;
  while (iVar1 == -0x1fff7fec) {
    _IOSleep(1000);
    uVar2 = uVar2 + 1;
    if (0x19 < uVar2) break;
    iVar1 = (**(code **)(*(long *)this + 0x8a8))(this,param_1,param_2,param_3,param_4);
  }
  CheckAVCError(this,iVar1,param_3);
  return;
}
