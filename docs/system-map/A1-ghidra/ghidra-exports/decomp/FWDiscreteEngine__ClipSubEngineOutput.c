// m_audio_b_FWDiscreteEngine::ClipSubEngineOutput
// Entry: 00018884
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* m_audio_b_FWDiscreteEngine::ClipSubEngineOutput(unsigned int, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8 __thiscall
m_audio_b_FWDiscreteEngine::ClipSubEngineOutput
          (m_audio_b_FWDiscreteEngine *this,uint param_1,uint param_2,uint param_3,
          _IOAudioStreamFormat *param_4,IOAudioStream *param_5)

{
  uint uVar1;
  undefined8 uVar2;
  
  uVar1 = *(uint *)(this + 300);
  *(uint *)(this + 300) = param_1 | uVar1;
  if (((param_1 | uVar1) & *(uint *)(this + 0x128)) != *(uint *)(this + 0x128)) {
    return 0;
  }
  *(undefined4 *)(this + 300) = 0;
                    /* WARNING: Could not recover jumptable at 0x000188c3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar2 = (**(code **)(*(long *)this + 0xc38))(this,param_2,param_3,param_4,param_5);
  return uVar2;
}
