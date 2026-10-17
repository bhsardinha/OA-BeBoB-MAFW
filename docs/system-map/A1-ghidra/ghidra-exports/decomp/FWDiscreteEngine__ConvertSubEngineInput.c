// m_audio_b_FWDiscreteEngine::ConvertSubEngineInput
// Entry: 000188e4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* m_audio_b_FWDiscreteEngine::ConvertSubEngineInput(unsigned int, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8
m_audio_b_FWDiscreteEngine::ConvertSubEngineInput
          (uint param_1,uint param_2,uint param_3,_IOAudioStreamFormat *param_4,
          IOAudioStream *param_5)

{
  uint uVar1;
  undefined4 in_register_0000003c;
  long *plVar2;
  
  plVar2 = (long *)CONCAT44(in_register_0000003c,param_1);
  uVar1 = *(uint *)(plVar2 + 0x26);
  if (uVar1 == 0) {
    (**(code **)(*plVar2 + 0xc40))(plVar2,param_3,(ulong)param_4 & 0xffffffff);
    uVar1 = *(uint *)(plVar2 + 0x26);
  }
  *(uint *)(plVar2 + 0x26) = uVar1 | param_2;
  if (((uVar1 | param_2) & *(uint *)(plVar2 + 0x25)) == *(uint *)(plVar2 + 0x25)) {
    *(undefined4 *)(plVar2 + 0x26) = 0;
  }
  return 0;
}
