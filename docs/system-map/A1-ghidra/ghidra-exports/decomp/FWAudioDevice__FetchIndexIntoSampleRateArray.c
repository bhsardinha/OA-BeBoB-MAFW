// m_audio_b_FWAudioDevice::FetchIndexIntoSampleRateArray
// Entry: 00001ec6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWAudioDevice::FetchIndexIntoSampleRateArray(_IOAudioSampleRate const*, unsigned int)
    */

ulong __thiscall
m_audio_b_FWAudioDevice::FetchIndexIntoSampleRateArray
          (m_audio_b_FWAudioDevice *this,_IOAudioSampleRate *param_1,uint param_2)

{
  ulong uVar1;
  
  uVar1 = 0;
  while( true ) {
    if (*(uint *)(param_1 + uVar1 * 8) == 0) {
      return 0;
    }
    if (*(uint *)(param_1 + uVar1 * 8) == param_2) break;
    uVar1 = (ulong)((int)uVar1 + 1);
  }
  return uVar1;
}
