// com_m_audio_FWAudiophileDevice::ReadHSCIData
// Entry: 0000ba92
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::ReadHSCIData(unsigned int*) */

void __thiscall
com_m_audio_FWAudiophileDevice::ReadHSCIData(com_m_audio_FWAudiophileDevice *this,uint *param_1)

{
  uint uVar1;
  char cVar2;
  long lVar3;
  int local_24;
  
  local_24 = 0;
  uVar1 = param_1[0xe];
  FWRev2AudioDevice::UpdatePeakMeters((FWRev2AudioDevice *)this,param_1,0xe);
  lVar3 = 0;
  do {
    *(undefined4 *)(*(long *)(this + 0x108) + 0x38 + lVar3 * 4) =
         *(undefined4 *)(*(long *)(this + 0x110) + 0x38 + lVar3 * 4);
    lVar3 = lVar3 + 1;
  } while (lVar3 != 6);
  m_audio_b_FWAudioDevice::DetectExternalSyncChanges
            ((m_audio_b_FWAudioDevice *)this,0xffffff < uVar1,uVar1 >> 0x10 & 0xff);
  cVar2 = TRotaryControl::RotaryValueChanged
                    ((TRotaryControl *)(this + 0x1268),uVar1 >> 8 & 0xf,&local_24);
  if (cVar2 != '\0') {
    FWSettingsRotaryControl::UserRotatedHardwareKnob
              ((FWSettingsRotaryControl *)(this + 0x468),local_24);
  }
  cVar2 = TRotaryControl::RotaryValueChanged
                    ((TRotaryControl *)(this + 0x126c),uVar1 >> 0xc & 0xf,&local_24);
  if (cVar2 != '\0') {
    FWSettingsRotaryControl::SetLevelsFromKnob
              ((FWSettingsRotaryControl *)(this + 0x468),0x100000000000000,"MAHeadphoneLevels",0x38,
               (BaseChannelGroup *)(this + 0x1548),local_24);
  }
  if (0xfffffff < uVar1 << 0x18) {
    FWSettingsABSwitch::MomentarySwitchPressed((FWSettingsABSwitch *)(this + 0x2d0));
  }
  return;
}
