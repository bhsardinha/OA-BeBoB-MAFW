// m_audio_b_FWBaseEngine::ConvertInputSamplesCore
// Entry: 0000496a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* m_audio_b_FWBaseEngine::ConvertInputSamplesCore(unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::ConvertInputSamplesCore
          (m_audio_b_FWBaseEngine *this,uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  if ((((param_2 != 0) && (this[0x2d4] == (m_audio_b_FWBaseEngine)0x0)) &&
      (*(int *)(this + 0x2ec) == *(int *)(*(long *)(this + 0x218) + 0xb0))) &&
     (*(int *)(this + 0x2f0) == *(int *)(*(long *)(this + 0x210) + 0xb0))) {
    IOSynchronizeDCLPrograms(this);
    if (param_2 + param_1 < 0x2001) {
      uVar2 = *(uint *)(this + 0x280) /
              (uint)((*(int *)(this + 0x2e8) + *(int *)(this + 0x2e4)) * 4);
      uVar1 = (param_1 - 1) + param_2;
      iVar5 = uVar2 - uVar1;
      if (uVar1 < uVar2) {
        iVar3 = iVar5 + -0x2000;
        iVar4 = iVar5;
      }
      else {
        iVar3 = iVar5 + 0x2000;
        iVar4 = -iVar5;
      }
      if (iVar4 < 0x801) {
        iVar3 = iVar5;
      }
      if (iVar3 < 0) {
        if (this[0x251] == (m_audio_b_FWBaseEngine)0x0) {
          this[0x250] = (m_audio_b_FWBaseEngine)0x1;
        }
      }
      else {
        IOSynchronizeDCLPrograms(this);
        (**(code **)(*(long *)this + 0xc98))
                  (this,param_2,
                   *(long *)(this + 0x290) +
                   (ulong)((*(int *)(this + 0x2e8) + *(int *)(this + 0x2e4)) * param_1) * 4,
                   *(undefined8 *)(this + 0x1d8),*(undefined8 *)(this + 0x1a8));
        *(uint *)(this + 0x33c) = param_2 + param_1;
      }
    }
  }
  return 0;
}
