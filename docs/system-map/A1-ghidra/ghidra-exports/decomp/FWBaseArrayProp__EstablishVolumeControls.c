// m_audio_b_FWBaseArrayProp::EstablishVolumeControls
// Entry: 000083e4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::EstablishVolumeControls(IOAudioEngine*, unsigned int, unsigned int,
   int, int, int, int) */

undefined8 __thiscall
m_audio_b_FWBaseArrayProp::EstablishVolumeControls
          (m_audio_b_FWBaseArrayProp *this,IOAudioEngine *param_1,uint param_2,uint param_3,
          int param_4,int param_5,int param_6,int param_7)

{
  undefined8 uVar1;
  uint uVar2;
  IOAudioControl *local_38;
  
  *(uint *)(this + 0x58) = param_3;
  uVar1 = 0;
  uVar2 = 0;
  while( true ) {
    if (*(uint *)(this + 0x2c) <= uVar2) {
      return uVar1;
    }
    uVar1 = m_audio_b_FWBaseProp::EstablishVolumeControl
                      ((m_audio_b_FWBaseProp *)this,param_1,param_2,param_3 + uVar2,uVar2,&local_38,
                       param_4,param_5,param_6,param_7);
    if ((int)uVar1 != 0) break;
    uVar2 = uVar2 + 1;
  }
  return uVar1;
}
