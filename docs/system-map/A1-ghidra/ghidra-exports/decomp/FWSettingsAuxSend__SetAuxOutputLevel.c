// FWSettingsAuxSend::SetAuxOutputLevel
// Entry: 00020d66
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::SetAuxOutputLevel(unsigned int, unsigned int) */

void __thiscall
FWSettingsAuxSend::SetAuxOutputLevel(FWSettingsAuxSend *this,uint param_1,uint param_2)

{
  SetAuxOutChannelFeature(this,param_1,'\x02',param_2);
  return;
}
