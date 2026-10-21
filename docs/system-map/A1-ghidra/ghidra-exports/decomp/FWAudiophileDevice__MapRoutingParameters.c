// com_m_audio_FWAudiophileDevice::MapRoutingParameters
// Entry: 0000ba50
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::MapRoutingParameters(m_audio_b_FWAVCAudioDevice::Routing&) */

undefined8 __thiscall
com_m_audio_FWAudiophileDevice::MapRoutingParameters
          (com_m_audio_FWAudiophileDevice *this,Routing *param_1)

{
  char cVar1;
  undefined8 uVar2;
  
  cVar1 = m_audio_b_FWAVCAudioDevice::FetchRoutingEntry
                    ((m_audio_b_FWAVCAudioDevice *)this,param_1,0xf,(Routing *)&_kRouteTable_36633);
  if (cVar1 == '\0') {
    uVar2 = 0xe00002c2;
  }
  else {
    *(undefined4 *)(param_1 + 4) = 1;
    *(undefined4 *)(param_1 + 8) = 1;
    uVar2 = 0;
  }
  return uVar2;
}
