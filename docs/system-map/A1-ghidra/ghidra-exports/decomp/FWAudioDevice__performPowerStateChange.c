// m_audio_b_FWAudioDevice::performPowerStateChange
// Entry: 00001e96
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2b)

/* m_audio_b_FWAudioDevice::performPowerStateChange(_IOAudioDevicePowerState,
   _IOAudioDevicePowerState, unsigned int*) */

undefined8 __thiscall
m_audio_b_FWAudioDevice::performPowerStateChange
          (m_audio_b_FWAudioDevice *this,int param_2,int param_3,undefined4 *param_4)

{
  *param_4 = 0;
  if ((param_2 == 0) && (0 < param_3)) {
    this[0x259] = (m_audio_b_FWAudioDevice)0x1;
  }
  else if ((0 < param_2) && (param_3 == 0)) {
    (**(code **)(*(long *)this + 0xb18))();
  }
  return 0;
}
