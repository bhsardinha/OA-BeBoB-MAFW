// com_m_audio_FWLightbridgeDevice::InitializeUserSettableProperties
// Entry: 00020556
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FWLightbridgeDevice::InitializeUserSettableProperties() */

void __thiscall
com_m_audio_FWLightbridgeDevice::InitializeUserSettableProperties
          (com_m_audio_FWLightbridgeDevice *this)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_InitializeUserSettableProperties_0003b4d8)(this);
  if (iVar1 != 0) {
    return;
  }
  (**(code **)(*(long *)this + 0xbc8))(this);
  UpdateChannelMaps(this,0,*(int *)(this + 0x464) != 0,*(int *)(this + 0x424) != 0);
  m_audio_b_FW32BitNumberProp::
  init<com_m_audio_FWLightbridgeDevice*,int(com_m_audio_FWLightbridgeDevice::*)(unsigned_int)>
            ((m_audio_b_FW32BitNumberProp *)(this + 0x3f8),
             *(m_audio_b_FWPropertyDict **)(this + 0xf8),"MASPDIFMirrorToAnalog12",this,
             SetMirrorSPDIFToAnalog);
  m_audio_b_FW32BitNumberProp::
  init<com_m_audio_FWLightbridgeDevice*,int(com_m_audio_FWLightbridgeDevice::*)(unsigned_int)>
            ((m_audio_b_FW32BitNumberProp *)(this + 0x438),
             *(m_audio_b_FWPropertyDict **)(this + 0xf8),"MAChannelOrderSPDIFFirst",this,
             SetOrderSPDIFChannelsFirst);
  return;
}
