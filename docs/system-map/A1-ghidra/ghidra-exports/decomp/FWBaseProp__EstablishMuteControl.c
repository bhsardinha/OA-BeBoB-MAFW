// m_audio_b_FWBaseProp::EstablishMuteControl
// Entry: 00009060
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::EstablishMuteControl(IOAudioEngine*, unsigned int, unsigned int, unsigned
   int, IOAudioControl*&) */

undefined8 __thiscall
m_audio_b_FWBaseProp::EstablishMuteControl
          (m_audio_b_FWBaseProp *this,IOAudioEngine *param_1,uint param_2,uint param_3,uint param_4,
          IOAudioControl **param_5)

{
  IOAudioControl *pIVar1;
  undefined8 uVar2;
  
  uVar2 = 0;
  pIVar1 = (IOAudioControl *)
           IOAudioToggleControl::createMuteControl(false,param_3,(char *)0x0,param_4,param_2);
  if (pIVar1 == (IOAudioControl *)0x0) {
    uVar2 = 0xe00002bd;
  }
  else {
    HookUpAudioControl(this,pIVar1,param_1);
  }
  *param_5 = pIVar1;
  return uVar2;
}
