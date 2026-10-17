// m_audio_b_FWAudioDevice::InitializeUserSettableProperties
// Entry: 0000252a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAudioDevice::InitializeUserSettableProperties() */

int __thiscall
m_audio_b_FWAudioDevice::InitializeUserSettableProperties(m_audio_b_FWAudioDevice *this)

{
  code *pcVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  m_audio_b_FWPropertyDict *this_00;
  char *pcVar5;
  
  pcVar1 = *(code **)(*(long *)this + 0x268);
  uVar4 = (**(code **)(**(long **)(this + 0xe0) + 0x2e8))(*(long **)(this + 0xe0),"GUID");
  pcVar5 = "GUID";
  (*pcVar1)(this,"GUID",uVar4);
  this_00 = OSObject::operator_new((OSObject *)(section_00000068.segname + 8),(ulong)pcVar5);
  if (this_00 == (m_audio_b_FWPropertyDict *)0x0) {
    *(undefined8 *)(this + 0xf8) = 0;
  }
  else {
    m_audio_b_FWPropertyDict::m_audio_b_FWPropertyDict(this_00);
    *(m_audio_b_FWPropertyDict **)(this + 0xf8) = this_00;
    cVar2 = (**(code **)(*(long *)this_00 + 0x1b8))(this_00,this);
    if (cVar2 != '\0') {
      iVar3 = m_audio_b_FW32BitNumberProp::
              init<m_audio_b_FWAudioDevice*,int(m_audio_b_FWAudioDevice::*)(unsigned_int)>
                        ((m_audio_b_FW32BitNumberProp *)(this + 0x158),
                         *(m_audio_b_FWPropertyDict **)(this + 0xf8),"MAEnableStereoPairEngines",
                         this,SetExportStereoPairEngines);
      if (iVar3 != 0) {
        return iVar3;
      }
      iVar3 = m_audio_b_FW32BitNumberProp::
              init<m_audio_b_FWAudioDevice*,int(m_audio_b_FWAudioDevice::*)(unsigned_int)>
                        ((m_audio_b_FW32BitNumberProp *)(this + 0x198),
                         *(m_audio_b_FWPropertyDict **)(this + 0xf8),"MAEnableAggregateEngine",this,
                         SetExportAggregateEngine);
      if (iVar3 != 0) {
        return iVar3;
      }
      iVar3 = m_audio_b_FWFunctionProp::
              init<m_audio_b_FWAudioDevice*,int(m_audio_b_FWAudioDevice::*)(OSObject*)>
                        ((m_audio_b_FWFunctionProp *)(this + 0x218),
                         *(m_audio_b_FWPropertyDict **)(this + 0xf8),"MAMetaResetToFactorySettings",
                         this,ResetMixer);
      if (iVar3 != 0) {
        return iVar3;
      }
      iVar3 = m_audio_b_FWFunctionProp::
              init<m_audio_b_FWAudioDevice*,int(m_audio_b_FWAudioDevice::*)(OSObject*)>
                        ((m_audio_b_FWFunctionProp *)(this + 0x1d8),
                         *(m_audio_b_FWPropertyDict **)(this + 0xf8),"MADeviceUserLabel",this,
                         SetDeviceUserLabel);
      return iVar3;
    }
  }
  return -0x1ffffd43;
}
