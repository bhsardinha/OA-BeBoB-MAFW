// com_m_audio_FW1814Device::SwitchDigitalSignals
// Entry: 0000e5dc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SwitchDigitalSignals(unsigned int) */

void com_m_audio_FW1814Device::SwitchDigitalSignals(uint param_1)

{
  int iVar1;
  uint uVar2;
  DeviceChannels *pDVar3;
  code *pcVar4;
  undefined4 in_register_0000003c;
  m_audio_b_FWAudioDevice *this;
  
  this = (m_audio_b_FWAudioDevice *)CONCAT44(in_register_0000003c,param_1);
  if (*(long *)(this + 0xe8) != 0) {
    pDVar3 = (DeviceChannels *)(**(code **)(*(long *)this + 0xa60))(this);
    (**(code **)(**(long **)(this + 0xe8) + 0xc48))();
    uVar2 = m_audio_b_FWAudioDevice::FetchIndexIntoSampleRateArray
                      (this,*(_IOAudioSampleRate **)(pDVar3 + 0x38),
                       *(uint *)(*(long *)(this + 0xe8) + 0x220));
    m_audio_b_FWBaseEngine::SetIOFormat(*(m_audio_b_FWBaseEngine **)(this + 0xe8),pDVar3,uVar2);
    iVar1 = *(int *)(*(long *)(this + 0xe8) + 0x2e0);
    if (iVar1 == 6) {
      pcVar4 = _Sample1814PeaksSPDIF6;
    }
    else if (iVar1 == 8) {
      pcVar4 = _Sample1814PeaksADAT8;
    }
    else if (iVar1 == 0xc) {
      pcVar4 = _Sample1814PeaksADAT12;
    }
    else {
      pcVar4 = _Sample1814Peaks4All;
    }
    *(code **)(*(long *)(this + 0xe8) + 0x308) = pcVar4;
    m_audio_b_FWAudioDevice::ClearPeakMeters(this);
    (**(code **)(*(long *)this + 0xaf8))(this,*(undefined8 *)(this + 0xe8));
    m_audio_b_FWAudioDevice::SetChannelNamesForMainEngine(this,*(IOAudioEngine **)(this + 0xe8));
    if (*(int *)(this + 0x184) != 0) {
                    /* WARNING: Could not recover jumptable at 0x0000e6c4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*(long *)this + 0xa40))(this);
      return;
    }
  }
  return;
}
