// FWSettingsABSwitch::SubmitHeadphoneSourceChange
// Entry: 0001e2e2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SubmitHeadphoneSourceChange() */

void __thiscall FWSettingsABSwitch::SubmitHeadphoneSourceChange(FWSettingsABSwitch *this)

{
  undefined4 uVar1;
  OSObject *pOVar2;
  
  pOVar2 = (OSObject *)OSNumber::withNumber((ulong)*(uint *)(this + 0x144),0x20);
  if (pOVar2 == (OSObject *)0x0) {
    return;
  }
  uVar1 = (**(code **)(*(long *)this + 0x28))(this,*(undefined4 *)(this + 0x144));
  (**(code **)**(undefined8 **)(this + 0x28))(*(undefined8 **)(this + 0x28),0,uVar1);
  m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients
            (*(m_audio_b_FWPropertyDict **)(*(long *)(this + 8) + 0xf8),"MAHeadphoneSourceV2",pOVar2
            );
                    /* WARNING: Could not recover jumptable at 0x0001e349. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)pOVar2 + 0x28))(pOVar2);
  return;
}
