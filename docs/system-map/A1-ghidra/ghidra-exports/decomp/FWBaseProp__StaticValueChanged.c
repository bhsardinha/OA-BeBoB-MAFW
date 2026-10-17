// m_audio_b_FWBaseProp::StaticValueChanged
// Entry: 00008bce
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::StaticValueChanged(IOService*, IOAudioControl*, int, int) */

undefined8
m_audio_b_FWBaseProp::StaticValueChanged
          (IOService *param_1,IOAudioControl *param_2,int param_3,int param_4)

{
  undefined8 uVar1;
  
  if (param_1 == (IOService *)0x0) {
    return 0xe00002c2;
  }
                    /* WARNING: Could not recover jumptable at 0x00008be9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (**(code **)(*(long *)param_1 + 0x140))();
  return uVar1;
}
