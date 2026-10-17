// m_audio_b_FWAudioDevice::GetBandwidthBytesAtSampleRate
// Entry: 00001ee2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWAudioDevice::GetBandwidthBytesAtSampleRate(unsigned int) */

int m_audio_b_FWAudioDevice::GetBandwidthBytesAtSampleRate(uint param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 in_register_0000003c;
  long *plVar3;
  
  plVar3 = (long *)CONCAT44(in_register_0000003c,param_1);
  piVar1 = (int *)(**(code **)(*plVar3 + 0xa68))(plVar3);
  if (plVar3[0x1d] == 0) {
    iVar2 = 0x20;
  }
  else {
    iVar2 = (*(uint *)(plVar3[0x1d] + 0x224) & 0xff) << 2;
  }
  return (piVar1[2] + *piVar1 + piVar1[4] * 2) * iVar2 + 0x10;
}
