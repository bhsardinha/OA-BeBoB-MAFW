// com_m_audio_FW410Device::com_m_audio_FW410Device
// Entry: 00015a24
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::com_m_audio_FW410Device(OSMetaClass const*) */

void __thiscall
com_m_audio_FW410Device::com_m_audio_FW410Device(com_m_audio_FW410Device *this,OSMetaClass *param_1)

{
  FWRev2AudioDevice::FWRev2AudioDevice((FWRev2AudioDevice *)this,param_1);
  *(undefined ***)this = &PTR__com_m_audio_FW410Device_0003c610;
  *(undefined4 *)(this + 0x2d0) = 1;
  *(undefined ***)(this + 0x2d8) = &PTR_SendToDevice_00048dd0;
  m_audio_b_FW64BitNumberProp::m_audio_b_FW64BitNumberProp
            ((m_audio_b_FW64BitNumberProp *)(this + 0x310));
  m_audio_b_FWFunctionProp::m_audio_b_FWFunctionProp((m_audio_b_FWFunctionProp *)(this + 0x358));
  FWSettingsAuxSend::FWSettingsAuxSend((FWSettingsAuxSend *)(this + 0x3a0));
  FWSettings410Levels::FWSettings410Levels((FWSettings410Levels *)(this + 0x678));
  m_audio_b_FW32BitNumberProp::m_audio_b_FW32BitNumberProp
            ((m_audio_b_FW32BitNumberProp *)(this + 0x1148));
  $_663::__663((__663 *)(this + 0x1188));
  return;
}
