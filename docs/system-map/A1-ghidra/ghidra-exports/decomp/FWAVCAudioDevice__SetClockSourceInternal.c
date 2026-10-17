// m_audio_b_FWAVCAudioDevice::SetClockSourceInternal
// Entry: 00023148
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::SetClockSourceInternal(unsigned int, bool) */

undefined8 m_audio_b_FWAVCAudioDevice::SetClockSourceInternal(uint param_1,bool param_2)

{
  undefined8 uVar1;
  undefined7 in_register_00000031;
  undefined4 in_register_0000003c;
  m_audio_b_FWAVCAudioDevice *this;
  undefined4 local_20;
  uchar local_1c;
  uchar local_1b;
  uint local_18;
  
  this = (m_audio_b_FWAVCAudioDevice *)CONCAT44(in_register_0000003c,param_1);
  local_18 = (uint)CONCAT71(in_register_00000031,param_2);
  local_20 = 4;
  local_1c = '\0';
  local_1b = '\0';
  if (local_18 < 0x40000000) {
    uVar1 = 0xe00002c2;
  }
  else {
    uVar1 = (**(code **)(*(long *)this + 0xb48))(this,&local_20);
    if ((int)uVar1 == 0) {
      uVar1 = AVCSelectorCommand(this,local_1c,local_1b);
    }
  }
  return uVar1;
}
