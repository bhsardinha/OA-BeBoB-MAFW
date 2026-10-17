// com_m_audio_FW1814Device::ReadHSCIData
// Entry: 0000d952
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ReadHSCIData(unsigned int*) */

void __thiscall com_m_audio_FW1814Device::ReadHSCIData(com_m_audio_FW1814Device *this,uint *param_1)

{
  m_audio_b_FWAudioDevice::DetectExternalSyncChanges
            ((m_audio_b_FWAudioDevice *)this,param_1[0x14] >> 0x18 & 1,param_1[0x14] >> 0x10 & 7);
                    /* WARNING: Could not recover jumptable at 0x0000d98c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xbd8))(this,param_1);
  return;
}
