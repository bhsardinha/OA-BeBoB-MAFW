// m_audio_b_FWBaseEngine::initHardware
// Entry: 000056b0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::initHardware(IOService*) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::initHardware(m_audio_b_FWBaseEngine *this,IOService *param_1)

{
  char cVar1;
  long lVar2;
  undefined8 uVar3;
  char local_120 [256];
  undefined4 local_20;
  undefined4 local_1c;
  
  lVar2 = OSMetaClassBase::safeMetaCast
                    ((OSMetaClassBase *)param_1,(OSMetaClass *)m_audio_b_FWAudioDevice::metaClass);
  *(long *)(this + 0x138) = lVar2;
  if ((lVar2 != 0) &&
     (cVar1 = (**(code **)(PTR_vtable_000300b0 + 0x9f0))(this,param_1), cVar1 != '\0')) {
    *(long *)(this + 0x140) = (*(long **)(this + 0x138))[0x1c];
    uVar3 = (**(code **)(**(long **)(this + 0x138) + 0xa50))();
    _snprintf(local_120,0x100,"%s Multichannel",uVar3);
    (**(code **)(*(long *)this + 0x9f8))(this,local_120);
    local_20 = *(undefined4 *)(this + 0x220);
    local_1c = 0;
    (**(code **)(**(long **)(this + 0x138) + 0x280))
              (*(long **)(this + 0x138),"MASampleRate",local_20,0x20);
    (**(code **)(*(long *)this + 0xb58))(this,&local_20);
    (**(code **)(*(long *)this + 0xb40))(this,0x2000);
    *(undefined4 *)(this + 0x228) = 1;
    *(undefined4 *)(this + 0x224) = 8;
    ResetLatency(this);
    this[0x346] = (m_audio_b_FWBaseEngine)((byte)this[0x346] | 7);
    cVar1 = RecreateAudioStreams(this,true,true);
    if (cVar1 != '\0') {
      *(uint *)(this + 0x344) = *(uint *)(this + 0x344) | 0x208000;
      return 1;
    }
  }
  return 0;
}
