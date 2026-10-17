// m_audio_b_FWBaseProp::EstablishClockSourceControl
// Entry: 000090dc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::EstablishClockSourceControl(IOAudioEngine*, unsigned int, unsigned int,
   unsigned int, IOAudioControl*&) */

undefined8 __thiscall
m_audio_b_FWBaseProp::EstablishClockSourceControl
          (m_audio_b_FWBaseProp *this,IOAudioEngine *param_1,uint param_2,uint param_3,uint param_4,
          IOAudioControl **param_5)

{
  IOAudioControl *pIVar1;
  undefined8 uVar2;
  
  uVar2 = 0;
  pIVar1 = (IOAudioControl *)
           IOAudioSelectorControl::create(0,param_3,(char *)0x0,param_4,0x636c636b,param_2);
  if (pIVar1 == (IOAudioControl *)0x0) {
    uVar2 = 0xe00002bd;
  }
  else {
    HookUpAudioControl(this,pIVar1,param_1);
  }
  *param_5 = pIVar1;
  return uVar2;
}
