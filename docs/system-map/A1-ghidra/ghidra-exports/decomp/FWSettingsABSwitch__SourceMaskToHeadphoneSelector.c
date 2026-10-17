// FWSettingsABSwitch::SourceMaskToHeadphoneSelector
// Entry: 0001dda0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SourceMaskToHeadphoneSelector(unsigned int) */

ulong __thiscall
FWSettingsABSwitch::SourceMaskToHeadphoneSelector(FWSettingsABSwitch *this,uint param_1)

{
  ulong uVar1;
  
  for (uVar1 = 0; (uint)uVar1 < 0x20; uVar1 = uVar1 + 1) {
    if ((param_1 >> (uVar1 & 0x3f) & 1) != 0) goto LAB_0001ddba;
  }
  uVar1 = 0;
LAB_0001ddba:
  return uVar1 & 0xffffffff;
}
