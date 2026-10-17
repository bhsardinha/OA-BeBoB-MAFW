// m_audio_b_FWAudioDevice::AddToDeviceList
// Entry: 000030a4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e6)

/* m_audio_b_FWAudioDevice::AddToDeviceList() */

void m_audio_b_FWAudioDevice::AddToDeviceList(void)

{
  if (sAudioDevices == (long *)0x0) {
    sAudioDevices = (long *)OSSet::withCapacity(4);
    if (sAudioDevices == (long *)0x0) {
      return;
    }
  }
                    /* WARNING: Could not recover jumptable at 0x000030ef. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*sAudioDevices + 0x1c0))(sAudioDevices);
  return;
}
