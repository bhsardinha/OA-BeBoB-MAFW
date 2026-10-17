// m_audio_b_FWBaseProp::EstablishDirectMonitoringVolumeControl
// Entry: 00009024
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseProp::EstablishDirectMonitoringVolumeControl(IOAudioEngine*, unsigned int,
   unsigned int, IOAudioControl*&, int, int, int, int) */

void __thiscall
m_audio_b_FWBaseProp::EstablishDirectMonitoringVolumeControl
          (m_audio_b_FWBaseProp *this,IOAudioEngine *param_1,uint param_2,uint param_3,
          IOAudioControl **param_4,int param_5,int param_6,int param_7,int param_8)

{
  EstablishVolumeControl
            (this,param_1,0x70617373,param_2,param_3,param_4,param_5,param_6,param_7,param_8);
  return;
}
