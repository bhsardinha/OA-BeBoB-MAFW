// m_audio_b_FWPairEngine::ResetIOFormat
// Entry: 0001ab42
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::ResetIOFormat(_IOAudioSampleRate*) */

void __thiscall
m_audio_b_FWPairEngine::ResetIOFormat(m_audio_b_FWPairEngine *this,_IOAudioSampleRate *param_1)

{
  undefined8 uVar1;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  
  local_28 = DAT_0002fc90;
  local_30 = DAT_0002fc88;
  local_38 = ResetIOFormat(_IOAudioSampleRate*)::C_58;
  (**(code **)(*(long *)this + 0xb00))(this);
  if (this[0x17c] != (m_audio_b_FWPairEngine)0x0) {
    (**(code **)(**(long **)(this + 0x140) + 0xa68))();
    (**(code **)(**(long **)(this + 0x140) + 0xa58))
              (*(long **)(this + 0x140),&local_38,param_1,param_1,0,0);
    (**(code **)(**(long **)(this + 0x140) + 0xa38))(*(long **)(this + 0x140),&local_38,1);
  }
  if (this[0x17d] != (m_audio_b_FWPairEngine)0x0) {
    (**(code **)(**(long **)(this + 0x138) + 0xa68))();
    (**(code **)(**(long **)(this + 0x138) + 0xa58))
              (*(long **)(this + 0x138),&local_38,param_1,param_1,0,0);
    (**(code **)(**(long **)(this + 0x138) + 0xa38))(*(long **)(this + 0x138),&local_38,1);
  }
  (**(code **)(*(long *)this + 0xb58))(this,param_1);
  (**(code **)(*(long *)this + 0xa80))(this,param_1);
  uVar1 = (**(code **)(**(long **)(this + 0x118) + 0x2b8))
                    (*(long **)(this + 0x118),"IOAudioEngineInputSampleLatency");
  (**(code **)(*(long *)this + 0x268))(this,"IOAudioEngineInputSampleLatency",uVar1);
  uVar1 = (**(code **)(**(long **)(this + 0x118) + 0x2b8))
                    (*(long **)(this + 0x118),"IOAudioEngineOutputSampleLatency");
  (**(code **)(*(long *)this + 0x268))(this,"IOAudioEngineOutputSampleLatency",uVar1);
  uVar1 = (**(code **)(**(long **)(this + 0x118) + 0x2b8))
                    (*(long **)(this + 0x118),"IOAudioEngineSampleOffset");
  (**(code **)(*(long *)this + 0x268))(this,"IOAudioEngineSampleOffset",uVar1);
  (**(code **)(*(long *)this + 0xb08))(this);
  return;
}
