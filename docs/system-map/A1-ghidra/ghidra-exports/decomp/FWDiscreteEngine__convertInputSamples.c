// m_audio_b_FWDiscreteEngine::convertInputSamples
// Entry: 00018aa6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::convertInputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8
m_audio_b_FWDiscreteEngine::convertInputSamples
          (void *param_1,void *param_2,uint param_3,uint param_4,_IOAudioStreamFormat *param_5,
          IOAudioStream *param_6)

{
  int iVar1;
  void *pvVar2;
  undefined4 in_register_00000014;
  
  pvVar2 = (void *)(**(code **)(*(long *)param_1 + 0xc10))(param_1);
  iVar1 = *(int *)param_6;
  ConvertSubEngineInput
            ((uint)param_1,1,param_4,(_IOAudioStreamFormat *)((ulong)param_5 & 0xffffffff),param_6);
  _memcpy((void *)CONCAT44(in_register_00000014,param_3),pvVar2,
          (ulong)(uint)(iVar1 * (int)param_5 * 4));
  return 0;
}
