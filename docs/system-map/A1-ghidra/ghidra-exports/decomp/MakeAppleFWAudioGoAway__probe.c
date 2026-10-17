// com_m_audio_MakeAppleFWAudioGoAway::probe
// Entry: 000129cc  Size: 114 bytes
// Prototype: undefined probe(com_m_audio_MakeAppleFWAudioGoAway * this, IOService * param_1, int * param_2)


/* com_m_audio_MakeAppleFWAudioGoAway::probe(IOService*, int*) */

com_m_audio_MakeAppleFWAudioGoAway * __thiscall
com_m_audio_MakeAppleFWAudioGoAway::probe
          (com_m_audio_MakeAppleFWAudioGoAway *this,IOService *param_1,int *param_2)

{
  code *pcVar1;
  char cVar2;
  long *plVar3;
  undefined8 uVar4;
  
  plVar3 = (long *)(**(code **)(*(long *)param_1 + 0x2b8))(param_1,"FireWire Product Name");
  *param_2 = 0;
  if (plVar3 != (long *)0x0) {
    pcVar1 = *(code **)(*plVar3 + 0x40);
    uVar4 = (**(code **)(*(long *)this + 0x2b8))(this,"FireWire Product Name");
    cVar2 = (*pcVar1)(plVar3,uVar4);
    if (cVar2 != '\0') {
      *param_2 = 0x7fffff;
      return this;
    }
  }
  return (com_m_audio_MakeAppleFWAudioGoAway *)0x0;
}


