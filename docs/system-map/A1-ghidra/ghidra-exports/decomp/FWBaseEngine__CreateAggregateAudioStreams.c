// m_audio_b_FWBaseEngine::CreateAggregateAudioStreams
// Entry: 00005a18
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::CreateAggregateAudioStreams(m_audio_b_FWAggregateEngine*) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::CreateAggregateAudioStreams
          (m_audio_b_FWBaseEngine *this,m_audio_b_FWAggregateEngine *param_1)

{
  IOAudioStream *pIVar1;
  IOAudioStream *pIVar2;
  
  pIVar1 = (IOAudioStream *)
           (**(code **)(*(long *)this + 0xc80))
                     (this,param_1,0,*(undefined8 *)(this + 0x208),*(undefined4 *)(this + 0x2d8));
  if (pIVar1 != (IOAudioStream *)0x0) {
    m_audio_b_FWAggregateEngine::AddStreamForAggregate(param_1,pIVar1,(IOAudioEngine *)this);
    pIVar2 = (IOAudioStream *)
             (**(code **)(*(long *)this + 0xc80))
                       (this,param_1,1,*(undefined8 *)(this + 0x290),*(undefined4 *)(this + 0x2dc));
    if (pIVar2 != (IOAudioStream *)0x0) {
      m_audio_b_FWAggregateEngine::AddStreamForAggregate(param_1,pIVar2,(IOAudioEngine *)this);
      ReplicateCurrentFormatToStream(this,*(IOAudioStream **)(this + 0x210),pIVar2);
      ReplicateCurrentFormatToStream(this,*(IOAudioStream **)(this + 0x218),pIVar1);
      (**(code **)(*(long *)pIVar1 + 0x28))(pIVar1);
      (**(code **)(*(long *)pIVar2 + 0x28))(pIVar2);
    }
  }
  return 0;
}
