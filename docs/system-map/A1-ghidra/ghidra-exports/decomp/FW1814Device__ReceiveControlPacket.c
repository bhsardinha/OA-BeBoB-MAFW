// com_m_audio_FW1814Device::ReceiveControlPacket
// Entry: 0000d98e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ReceiveControlPacket(FWControlPacket*) */

void __thiscall
com_m_audio_FW1814Device::ReceiveControlPacket
          (com_m_audio_FW1814Device *this,FWControlPacket *param_1)

{
  m_audio_b_FWAudioDevice::DetectExternalSyncChanges
            ((m_audio_b_FWAudioDevice *)this,*(uint *)(param_1 + 0x54) >> 3 & 1,
             *(uint *)(param_1 + 0x54) >> 8 & 7);
                    /* WARNING: Could not recover jumptable at 0x0000d9cc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xbd8))(this,param_1 + 4);
  return;
}
