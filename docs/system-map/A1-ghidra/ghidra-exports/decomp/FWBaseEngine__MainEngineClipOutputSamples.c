// m_audio_b_FWBaseEngine::MainEngineClipOutputSamples
// Entry: 00003ed4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* m_audio_b_FWBaseEngine::MainEngineClipOutputSamples(unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::MainEngineClipOutputSamples
          (m_audio_b_FWBaseEngine *this,uint param_1,uint param_2,_IOAudioStreamFormat *param_3,
          IOAudioStream *param_4)

{
  int iVar1;
  long lVar2;
  undefined8 uVar3;
  
  lVar2 = (**(code **)(**(long **)(this + 0x218) + 0xa00))();
  iVar1 = *(int *)(*(long *)(this + 0x218) + 0xb0);
  if ((*(int *)(this + 0x2ec) == iVar1) &&
     (*(int *)(this + 0x2f0) == *(int *)(*(long *)(this + 0x210) + 0xb0))) {
    if (*(char *)(*(long *)(this + 0x218) + 0xc0) != '\0') {
      lVar2 = lVar2 + (ulong)(iVar1 * param_1) * 4;
    }
    uVar3 = (**(code **)(*(long *)this + 0xc90))
                      (this,lVar2,*(undefined8 *)(this + 0x208),param_1,param_2,param_3,param_4);
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}
