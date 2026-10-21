// com_m_audio_FWAudiophileDevice::com_m_audio_FWAudiophileDevice
// Entry: 0000c0aa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::com_m_audio_FWAudiophileDevice(OSMetaClass const*) */

void __thiscall
com_m_audio_FWAudiophileDevice::com_m_audio_FWAudiophileDevice
          (com_m_audio_FWAudiophileDevice *this,OSMetaClass *param_1)

{
  FWRev2AudioDevice::FWRev2AudioDevice((FWRev2AudioDevice *)this,param_1);
  *(undefined ***)this = &PTR__com_m_audio_FWAudiophileDevice_00035690;
  FWSettingsABSwitch::FWSettingsABSwitch((FWSettingsABSwitch *)(this + 0x2d0));
  *(undefined ***)(this + 0x468) = &PTR_SendToDevice_00048dd0;
  m_audio_b_FW64BitNumberProp::m_audio_b_FW64BitNumberProp
            ((m_audio_b_FW64BitNumberProp *)(this + 0x4a0));
  m_audio_b_FWFunctionProp::m_audio_b_FWFunctionProp((m_audio_b_FWFunctionProp *)(this + 0x4e8));
  FWSettingsAuxSend::FWSettingsAuxSend((FWSettingsAuxSend *)(this + 0x530));
  FWSettingsLevels::FWSettingsLevels((FWSettingsLevels *)(this + 0x808));
  *(undefined4 *)(this + 0x1268) = 1;
  *(undefined4 *)(this + 0x126c) = 1;
  $_667::__667((__667 *)(this + 0x1270));
  return;
}
