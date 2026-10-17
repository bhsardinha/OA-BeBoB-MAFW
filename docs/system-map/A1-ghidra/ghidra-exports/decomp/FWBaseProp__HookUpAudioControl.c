// m_audio_b_FWBaseProp::HookUpAudioControl
// Entry: 00008f4a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::HookUpAudioControl(IOAudioControl*, IOAudioEngine*) */

void __thiscall
m_audio_b_FWBaseProp::HookUpAudioControl
          (m_audio_b_FWBaseProp *this,IOAudioControl *param_1,IOAudioEngine *param_2)

{
  (**(code **)(*(long *)param_1 + 0x948))(param_1,StaticValueChanged,this);
  (**(code **)(*(long *)param_2 + 0xb18))(param_2,param_1);
  AttachAudioControl(this,param_1);
                    /* WARNING: Could not recover jumptable at 0x00008fa0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_1 + 0x28))(param_1);
  return;
}
