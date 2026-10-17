// m_audio_b_FWPairEngine::convertInputSamples
// Entry: 0001ae64
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::convertInputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8
m_audio_b_FWPairEngine::convertInputSamples
          (void *param_1,void *param_2,uint param_3,uint param_4,_IOAudioStreamFormat *param_5,
          IOAudioStream *param_6)

{
  undefined4 uVar1;
  long lVar2;
  undefined8 uVar3;
  undefined4 in_register_00000014;
  
  lVar2 = (**(code **)(**(long **)((long)param_1 + 0x118) + 0xc00))();
  uVar1 = *(undefined4 *)(lVar2 + 0xb0);
  m_audio_b_FWDiscreteEngine::ConvertSubEngineInput
            ((uint)*(undefined8 *)((long)param_1 + 0x118),
             (uint)(1L << (*(byte *)((long)param_1 + 300) & 0x3f)),param_4,
             (_IOAudioStreamFormat *)((ulong)param_5 & 0xffffffff),param_6);
  uVar3 = (**(code **)(**(long **)((long)param_1 + 0x118) + 0xc10))();
  _CopyChannelPairFromMultichannelBuffer
            ((ulong)param_5 & 0xffffffff,uVar1,*(undefined4 *)((long)param_1 + 0x134),uVar3,
             CONCAT44(in_register_00000014,param_3));
  return 0;
}
