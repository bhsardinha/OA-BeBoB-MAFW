// m_audio_b_FWAudioDevice::RequestFormatUpdateForSampleRate
// Entry: 00002a84
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWAudioDevice::RequestFormatUpdateForSampleRate(unsigned int) */

void __thiscall
m_audio_b_FWAudioDevice::RequestFormatUpdateForSampleRate
          (m_audio_b_FWAudioDevice *this,uint param_1)

{
  uint uVar1;
  DeviceChannels *pDVar2;
  uint uVar3;
  
  pDVar2 = (DeviceChannels *)(**(code **)(*(long *)this + 0xa68))(this,param_1);
  uVar3 = 0;
  while (uVar1 = *(uint *)(*(long *)(pDVar2 + 0x38) + (ulong)uVar3 * 8), uVar1 != 0) {
    if (uVar1 == param_1) goto LAB_00002aba;
    uVar3 = uVar3 + 1;
  }
  uVar3 = 0;
LAB_00002aba:
  m_audio_b_FWBaseEngine::SetIOFormat(*(m_audio_b_FWBaseEngine **)(this + 0xe8),pDVar2,uVar3);
  return;
}
