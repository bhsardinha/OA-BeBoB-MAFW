// com_m_audio_FW1814Device::CustomMixerWriteWithRotatedAuxChannels
// Entry: 0000e982
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::CustomMixerWriteWithRotatedAuxChannels(unsigned int, unsigned int,
   void*) */

void __thiscall
com_m_audio_FW1814Device::CustomMixerWriteWithRotatedAuxChannels
          (com_m_audio_FW1814Device *this,uint param_1,uint param_2,void *param_3)

{
  undefined1 local_5c [8];
  undefined1 local_54 [36];
  
  _memcpy(local_5c,(void *)((long)param_3 + 0x24),8);
  _memcpy(local_54,param_3,0x24);
  (**(code **)(*(long *)this + 0xba8))(this,param_1,param_2,local_5c);
  return;
}
