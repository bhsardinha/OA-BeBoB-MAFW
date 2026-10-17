// com_m_audio_FWLightbridgeDevice::IndexForAC3OutputChannels
// Entry: 0001f48e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWLightbridgeDevice::IndexForAC3OutputChannels(unsigned int&, unsigned int&) */

void __thiscall
com_m_audio_FWLightbridgeDevice::IndexForAC3OutputChannels
          (com_m_audio_FWLightbridgeDevice *this,uint *param_1,uint *param_2)

{
  if ((*(long *)(this + 0xe8) != 0) && (0x15887 < *(uint *)(*(long *)(this + 0xe8) + 0x220))) {
    *param_1 = 0x10;
    *param_2 = 0x11;
    return;
  }
  *param_1 = 0x20;
  *param_2 = 0x21;
  return;
}
