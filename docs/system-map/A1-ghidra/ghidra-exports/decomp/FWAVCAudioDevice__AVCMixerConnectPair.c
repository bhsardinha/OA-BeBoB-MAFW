// m_audio_b_FWAVCAudioDevice::AVCMixerConnectPair
// Entry: 0002367c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::AVCMixerConnectPair(bool, unsigned char, unsigned char, unsigned
   char, unsigned char) */

void __thiscall
m_audio_b_FWAVCAudioDevice::AVCMixerConnectPair
          (m_audio_b_FWAVCAudioDevice *this,bool param_1,uchar param_2,uchar param_3,uchar param_4,
          uchar param_5)

{
  int iVar1;
  
  iVar1 = AVCMixerConnectSingle
                    (this,param_1,param_2,param_3,param_4 * '\x02' + 0xff,param_5 * '\x02' + 0xff);
  if (iVar1 != 0) {
    return;
  }
  AVCMixerConnectSingle(this,param_1,param_2,param_3,param_4 * '\x02',param_5 * '\x02');
  return;
}
