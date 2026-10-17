// m_audio_b_FWAudioDevice::GetDeviceChannels
// Entry: 00001e52
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWAudioDevice::GetDeviceChannels() const */

void __thiscall m_audio_b_FWAudioDevice::GetDeviceChannels(m_audio_b_FWAudioDevice *this)

{
  undefined4 uVar1;
  
  if (*(long *)(this + 0xe8) == 0) {
    uVar1 = 0xac44;
  }
  else {
    uVar1 = *(undefined4 *)(*(long *)(this + 0xe8) + 0x220);
  }
                    /* WARNING: Could not recover jumptable at 0x00001e7a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xa68))(this,uVar1);
  return;
}
