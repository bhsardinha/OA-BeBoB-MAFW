// m_audio_b_FWBaseArrayProp::EstablishMuteControls
// Entry: 00008468
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::EstablishMuteControls(IOAudioEngine*, unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWBaseArrayProp::EstablishMuteControls
          (m_audio_b_FWBaseArrayProp *this,IOAudioEngine *param_1,uint param_2,uint param_3)

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
    uVar1 = m_audio_b_FWBaseProp::EstablishMuteControl
                      ((m_audio_b_FWBaseProp *)this,param_1,param_2,param_3 + uVar2,uVar2,&local_38)
    ;
    if ((int)uVar1 != 0) break;
    uVar2 = uVar2 + 1;
  }
  return uVar1;
}
