// m_audio_b_FWAVCAudioDevice::ConvertClockSourceToDigitalSignalValue
// Entry: 000237e4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* m_audio_b_FWAVCAudioDevice::ConvertClockSourceToDigitalSignalValue(unsigned int&) */

void __thiscall
m_audio_b_FWAVCAudioDevice::ConvertClockSourceToDigitalSignalValue
          (m_audio_b_FWAVCAudioDevice *this,uint *param_1)

{
  if (*param_1 < 0x40000000) {
    _IOLog("M-FW: Converting old digital bitmap (%X)\n",*param_1);
    switch(*param_1) {
    case 0:
      *param_1 = 0x40000000;
      break;
    case 1:
      *param_1 = 0x40000004;
      break;
    case 2:
      *param_1 = 0x40000001;
      break;
    case 3:
      *param_1 = 0x40000005;
    }
  }
  return;
}
