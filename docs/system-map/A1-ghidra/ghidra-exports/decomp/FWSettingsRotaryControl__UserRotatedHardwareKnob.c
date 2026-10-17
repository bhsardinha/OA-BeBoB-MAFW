// FWSettingsRotaryControl::UserRotatedHardwareKnob
// Entry: 00021334
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsRotaryControl::UserRotatedHardwareKnob(int) */

void __thiscall
FWSettingsRotaryControl::UserRotatedHardwareKnob(FWSettingsRotaryControl *this,int param_1)

{
  ulonglong uVar1;
  
  uVar1 = *(ulonglong *)(this + 0x68);
  if (uVar1 == 0) {
    return;
  }
  SetLevelsFromKnob(this,uVar1,"MASWReturnLevels",0,*(BaseChannelGroup **)(this + 0x10),param_1);
  SetLevelsFromKnob(this,uVar1,"MAOutputLevels",0x20,*(BaseChannelGroup **)(this + 0x18),param_1);
  SetLevelsFromKnob(this,uVar1,"MAInputLevels",0x10,*(BaseChannelGroup **)(this + 0x20),param_1);
  SetLevelsFromKnob(this,uVar1,"MAHeadphoneLevels",0x38,*(BaseChannelGroup **)(this + 0x28),param_1)
  ;
  SetLevelsFromKnob(this,uVar1,"MAAuxOutputLevels",0x32,*(BaseChannelGroup **)(this + 0x30),param_1)
  ;
  return;
}
