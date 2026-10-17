// m_audio_b_FWBaseProp::AttachAudioControl
// Entry: 00008f04
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::AttachAudioControl(IOAudioControl*) */

undefined8 __thiscall
m_audio_b_FWBaseProp::AttachAudioControl(m_audio_b_FWBaseProp *this,IOAudioControl *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x10);
  if (plVar1 == (long *)0x0) {
    plVar1 = (long *)OSSet::withCapacity(10);
    *(long **)(this + 0x10) = plVar1;
    if (plVar1 == (long *)0x0) {
      return 0xe00002bd;
    }
  }
  (**(code **)(*plVar1 + 0x1c0))(plVar1,param_1);
  return 0;
}
