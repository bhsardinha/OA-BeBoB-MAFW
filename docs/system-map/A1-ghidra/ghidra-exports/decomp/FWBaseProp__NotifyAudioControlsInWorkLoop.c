// m_audio_b_FWBaseProp::NotifyAudioControlsInWorkLoop
// Entry: 00008b98
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::NotifyAudioControlsInWorkLoop() */

void __thiscall m_audio_b_FWBaseProp::NotifyAudioControlsInWorkLoop(m_audio_b_FWBaseProp *this)

{
  undefined8 local_18;
  
  (**(code **)(*(long *)this + 0x120))(this,0,&local_18);
  (**(code **)(*(long *)this + 0x138))(this,local_18);
  return;
}
