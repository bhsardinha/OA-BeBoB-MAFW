// com_m_audio_FWEngine2::ConnectAllPlugs
// Entry: 00012476
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::ConnectAllPlugs() */

undefined8 __thiscall com_m_audio_FWEngine2::ConnectAllPlugs(com_m_audio_FWEngine2 *this)

{
  char cVar1;
  undefined8 uVar2;
  
  if (this[0x2d3] == (com_m_audio_FWEngine2)0x0) {
    cVar1 = m_audio_b_FWAudioDevice::SufficientBandwidthAvailable
                      (*(m_audio_b_FWAudioDevice **)(this + 0x138));
    if (cVar1 == '\0') {
      uVar2 = 0xe00002be;
    }
    else {
      uVar2 = ConnectAudioMIDIChannels(this);
      if ((int)uVar2 == 0) {
        this[0x2d3] = (com_m_audio_FWEngine2)0x1;
      }
      else {
        this[0x2d3] = (com_m_audio_FWEngine2)0x0;
      }
    }
  }
  else {
    uVar2 = 0xe00002bc;
  }
  return uVar2;
}
