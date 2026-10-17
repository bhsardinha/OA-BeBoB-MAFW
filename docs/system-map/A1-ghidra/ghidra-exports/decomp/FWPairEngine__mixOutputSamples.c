// m_audio_b_FWPairEngine::mixOutputSamples
// Entry: 0001af32
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::mixOutputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8
m_audio_b_FWPairEngine::mixOutputSamples
          (void *param_1,void *param_2,uint param_3,uint param_4,_IOAudioStreamFormat *param_5,
          IOAudioStream *param_6)

{
  long lVar1;
  long lVar2;
  undefined8 uVar3;
  undefined4 in_register_00000014;
  
  lVar1 = (**(code **)(**(long **)((long)param_1 + 0x118) + 0xc08))
                    (*(long **)((long)param_1 + 0x118),param_2,
                     CONCAT44(in_register_00000014,param_3));
  if ((param_2 == (void *)0x0) || (lVar1 == 0)) {
    uVar3 = 0xe00002c2;
  }
  else {
    lVar2 = (**(code **)(**(long **)((long)param_1 + 0x118) + 0xbf8))();
    _MixChannelPairToMultichannelBuffer
              ((ulong)param_5 & 0xffffffff,*(int *)(lVar2 + 0xb0),
               *(undefined4 *)((long)param_1 + 0x130),param_2,
               lVar1 + (ulong)(param_4 * *(int *)(lVar2 + 0xb0)) * 4);
    uVar3 = 0;
  }
  return uVar3;
}
