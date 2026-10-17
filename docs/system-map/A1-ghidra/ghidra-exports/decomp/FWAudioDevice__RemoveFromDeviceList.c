// m_audio_b_FWAudioDevice::RemoveFromDeviceList
// Entry: 00002046
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e6)

/* m_audio_b_FWAudioDevice::RemoveFromDeviceList() */

void __thiscall m_audio_b_FWAudioDevice::RemoveFromDeviceList(m_audio_b_FWAudioDevice *this)

{
  int iVar1;
  
  if (sAudioDevices != (long *)0x0) {
    (**(code **)(*sAudioDevices + 0x1d8))(sAudioDevices,this);
    iVar1 = (**(code **)(*sAudioDevices + 0x130))();
    if (iVar1 == 0) {
      (**(code **)(*sAudioDevices + 0x28))();
      sAudioDevices = (long *)0x0;
    }
  }
  return;
}
