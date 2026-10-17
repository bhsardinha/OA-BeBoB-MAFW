// m_audio_b_FWAVCAudioDevice::MapRoutingParameters
// Entry: 00022a80
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e9)

/* m_audio_b_FWAVCAudioDevice::MapRoutingParameters(m_audio_b_FWAVCAudioDevice::Routing&) */

undefined8 __thiscall
m_audio_b_FWAVCAudioDevice::MapRoutingParameters(m_audio_b_FWAVCAudioDevice *this,Routing *param_1)

{
  param_1[0xc] = (Routing)0x1;
  param_1[0xd] = (Routing)0x0;
  return 0;
}
