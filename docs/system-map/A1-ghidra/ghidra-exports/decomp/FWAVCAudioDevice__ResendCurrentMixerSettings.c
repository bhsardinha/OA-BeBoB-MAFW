// m_audio_b_FWAVCAudioDevice::ResendCurrentMixerSettings
// Entry: 00023af8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e12)

/* m_audio_b_FWAVCAudioDevice::ResendCurrentMixerSettings() */

void __thiscall
m_audio_b_FWAVCAudioDevice::ResendCurrentMixerSettings(m_audio_b_FWAVCAudioDevice *this)

{
  char cVar1;
  uint uVar2;
  ulong local_20;
  int local_18 [2];
  
  cVar1 = (**(code **)(*(long *)this + 0xac8))(this);
  if (cVar1 != '\0') {
    _clock_get_uptime(local_18);
    uVar2 = local_18[0] - *(int *)(this + 0x270) >> 0x1f;
    _absolutetime_to_nanoseconds
              ((long)(int)((local_18[0] - *(int *)(this + 0x270)) + uVar2 ^ uVar2),&local_20);
    if (local_20 < 1000000000) {
      _IOLog("M-FW: Trying to resend mixer settings too often (within %d second of previous resend)\n"
             ,1);
      _IOSleep(1000);
    }
    (**(code **)**(undefined8 **)(this + 0x150))
              (*(undefined8 **)(this + 0x150),*(undefined4 *)(this + 0x144));
  }
  _clock_get_uptime(this + 0x270);
  return;
}
