// com_m_audio_FWAudiophileDevice::GetRoundTripLatencyForFDF
// Entry: 0000afde  Size: 47 bytes
// Prototype: undefined GetRoundTripLatencyForFDF(com_m_audio_FWAudiophileDevice * this, FDFValue param_1)


/* com_m_audio_FWAudiophileDevice::GetRoundTripLatencyForFDF(FDFValue) */

undefined8 __thiscall
com_m_audio_FWAudiophileDevice::GetRoundTripLatencyForFDF(undefined8 param_1,int param_2)

{
  if (param_2 == 4) {
    return 0x162;
  }
  if (param_2 != 3) {
    if (param_2 == 2) {
      return 0xc4;
    }
    return 0xb6;
  }
  return 0x14a;
}


