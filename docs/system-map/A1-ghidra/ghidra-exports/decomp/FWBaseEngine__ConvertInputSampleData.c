// m_audio_b_FWBaseEngine::ConvertInputSampleData
// Entry: 00004624
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* m_audio_b_FWBaseEngine::ConvertInputSampleData(unsigned int, int*, int*, float*) */

void __thiscall
m_audio_b_FWBaseEngine::ConvertInputSampleData
          (m_audio_b_FWBaseEngine *this,uint param_1,int *param_2,int *param_3,float *param_4)

{
  code *pcVar1;
  
  if (*(uint **)(this + 800) == (uint *)0x0) {
    pcVar1 = *(code **)(this + 0x310);
    if (pcVar1 == (code *)0x0) {
      if (this[0x2d5] == (m_audio_b_FWBaseEngine)0x0) {
        pcVar1 = _ConvertLLRRInputToSInt32;
      }
      else {
        pcVar1 = _ConvertLRLRInputToSInt32;
      }
      *(code **)(this + 0x310) = pcVar1;
    }
    (*pcVar1)(*(int *)(this + 0x2e4) * param_1,param_2,param_3,*(undefined8 *)(this + 0x318));
  }
  else {
    ConvertSampleFramesFromInput
              (param_2,param_3,*(uint *)(this + 0x2e4),*(uint *)(this + 0x2f0),param_1,
               *(uint **)(this + 800),*(int *)(this + 0x2e8) != 0);
  }
  _NativeInt32ToFloat32_X86(param_3,param_4,*(int *)(this + 0x2f0) * param_1);
  return;
}
