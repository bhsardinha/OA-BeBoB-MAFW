// FWSettingsAuxSend::SetAuxPan
// Entry: 00020d78
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::SetAuxPan(unsigned int, unsigned int) */

void __thiscall FWSettingsAuxSend::SetAuxPan(FWSettingsAuxSend *this,uint param_1,uint param_2)

{
  SetAuxOutChannelFeature(this,param_1,'\x03',-param_2);
  return;
}
