// m_audio_b_FWAVCAudioDevice::ReleaseCustomMixerWriteBuffer
// Entry: 00022b8c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::ReleaseCustomMixerWriteBuffer(IOBufferMemoryDescriptor*) */

void __thiscall
m_audio_b_FWAVCAudioDevice::ReleaseCustomMixerWriteBuffer
          (m_audio_b_FWAVCAudioDevice *this,IOBufferMemoryDescriptor *param_1)

{
  if (param_1 == (IOBufferMemoryDescriptor *)0x0) {
    return;
  }
  (**(code **)(*(long *)param_1 + 0x1f8))(param_1,0);
                    /* WARNING: Could not recover jumptable at 0x00022bbe. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_1 + 0x28))(param_1);
  return;
}
