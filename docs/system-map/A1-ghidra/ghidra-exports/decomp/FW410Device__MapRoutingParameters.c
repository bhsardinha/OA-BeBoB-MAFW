// com_m_audio_FW410Device::MapRoutingParameters
// Entry: 0001428a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::MapRoutingParameters(m_audio_b_FWAVCAudioDevice::Routing&) */

void __thiscall
com_m_audio_FW410Device::MapRoutingParameters(com_m_audio_FW410Device *this,Routing *param_1)

{
  (*(code *)PTR_MapRoutingParameters_0003b4f0)(this,param_1);
  if (*(int *)param_1 == 1) {
    if (*(int *)(param_1 + 4) != 5) {
      return;
    }
    param_1[0xd] = (Routing)0x1;
  }
  else {
    if (*(int *)param_1 != 0) {
      return;
    }
    param_1[0xd] = (Routing)((*(int *)(param_1 + 4) != 1) + '\x01');
    if (*(int *)(param_1 + 4) == 1) {
      return;
    }
  }
  *(undefined4 *)(param_1 + 4) = 1;
  return;
}
