// com_m_audio_FW1814Device::com_m_audio_FW1814Device
// Entry: 0000f2f4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2c)

/* com_m_audio_FW1814Device::com_m_audio_FW1814Device(OSMetaClass const*) */

void __thiscall
com_m_audio_FW1814Device::com_m_audio_FW1814Device
          (com_m_audio_FW1814Device *this,OSMetaClass *param_1)

{
  long lVar1;
  com_m_audio_FW1814Device *pcVar2;
  
  FWRev1AudioDevice::FWRev1AudioDevice((FWRev1AudioDevice *)this,param_1);
  *(undefined ***)this = &PTR__com_m_audio_FW1814Device_000369c0;
  *(undefined4 *)(this + 0x2c8) = 1;
  *(undefined4 *)(this + 0x2cc) = 1;
  *(undefined4 *)(this + 0x2d0) = 1;
  FWSettingsABSwitch::FWSettingsABSwitch((FWSettingsABSwitch *)(this + 0x2d8));
  *(undefined ***)(this + 0x470) = &PTR_SendToDevice_00048dd0;
  m_audio_b_FW64BitNumberProp::m_audio_b_FW64BitNumberProp
            ((m_audio_b_FW64BitNumberProp *)(this + 0x4a8));
  m_audio_b_FWFunctionProp::m_audio_b_FWFunctionProp((m_audio_b_FWFunctionProp *)(this + 0x4f0));
  FWSettings1814Levels::FWSettings1814Levels((FWSettings1814Levels *)(this + 0x538));
  FWSettings1814AuxSend::FWSettings1814AuxSend((FWSettings1814AuxSend *)(this + 0xf98));
  $_667::__667((__667 *)(this + 0x1318));
  pcVar2 = this + 0x1b8c;
  lVar1 = 0;
  do {
    *(int *)pcVar2 = (int)lVar1;
    pcVar2 = pcVar2 + 4;
    lVar1 = lVar1 + 6;
  } while (lVar1 != 0x60);
  return;
}
