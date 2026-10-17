// m_audio_b_FWBaseProp::EstablishDirectMonitoringToggleControl
// Entry: 000090c4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::EstablishDirectMonitoringToggleControl(IOAudioEngine*, unsigned int,
   unsigned int, IOAudioControl*&) */

void __thiscall
m_audio_b_FWBaseProp::EstablishDirectMonitoringToggleControl
          (m_audio_b_FWBaseProp *this,IOAudioEngine *param_1,uint param_2,uint param_3,
          IOAudioControl **param_4)

{
  EstablishMuteControl(this,param_1,0x70617373,param_2,param_3,param_4);
  return;
}
