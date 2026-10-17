// m_audio_b_FWBaseEngine::createNewAudioStream
// Entry: 000055b6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::createNewAudioStream(IOAudioEngine*, _IOAudioStreamDirection, void*,
   unsigned int) */

m_audio_b_FWStream * __thiscall
m_audio_b_FWBaseEngine::createNewAudioStream
          (m_audio_b_FWBaseEngine *this,m_audio_b_FWBaseEngine *param_1,undefined4 param_3,
          undefined8 param_4,undefined4 param_5)

{
  char cVar1;
  m_audio_b_FWStream *this_00;
  m_audio_b_FWBaseEngine *pmVar2;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  
  local_38 = DAT_0002f710;
  local_40 = DAT_0002f708;
  local_48 = createNewAudioStream(IOAudioEngine*,_IOAudioStreamDirection,void*,unsigned_int)::C_105;
  this_00 = OSObject::operator_new((OSObject *)&segment_command_00000158,(ulong)param_1);
  if (this_00 == (m_audio_b_FWStream *)0x0) {
    this_00 = (m_audio_b_FWStream *)0x0;
  }
  else {
    m_audio_b_FWStream::m_audio_b_FWStream(this_00);
    pmVar2 = this;
    if (param_1 != (m_audio_b_FWBaseEngine *)0x0) {
      pmVar2 = param_1;
    }
    cVar1 = (**(code **)(*(long *)this_00 + 0x9d0))(this_00,pmVar2,param_3,1,0,0);
    if (cVar1 == '\0') {
      (**(code **)(*(long *)this_00 + 0x28))(this_00);
      this_00 = (m_audio_b_FWStream *)0x0;
    }
    else {
      (**(code **)(*(long *)this_00 + 0xa58))(this_00,&local_48,this + 0x8c,this + 0x8c,0,0);
      (**(code **)(*(long *)this_00 + 0x9e0))(this_00,param_4,param_5);
      if (param_1 != (m_audio_b_FWBaseEngine *)0x0) {
        (**(code **)(*(long *)param_1 + 0xa00))(param_1,this_00);
      }
    }
  }
  return this_00;
}
