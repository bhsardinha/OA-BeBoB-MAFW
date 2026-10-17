// FWSettingsAuxSend::SendToDevice
// Entry: 00020af4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::SendToDevice() */

undefined8 __thiscall FWSettingsAuxSend::SendToDevice(FWSettingsAuxSend *this)

{
  undefined4 uVar1;
  uint uVar2;
  
  (**(code **)(**(long **)(this + 0x10) + 0x28))();
  if (*(int *)(this + 0x2c8) != 0) {
    uVar2 = 0;
    do {
      uVar1 = m_audio_b_FWBaseArrayProp::Get((m_audio_b_FWBaseArrayProp *)(this + 0x200),uVar2);
      uVar2 = uVar2 + 1;
      (**(code **)**(undefined8 **)(this + 0x238))(*(undefined8 **)(this + 0x238),uVar2,uVar1);
    } while (uVar2 < *(uint *)(this + 0x2c8));
  }
  uVar1 = m_audio_b_FWBaseArrayProp::Get((m_audio_b_FWBaseArrayProp *)(this + 0x260),0);
  (**(code **)**(undefined8 **)(this + 0x298))(*(undefined8 **)(this + 0x298),1,uVar1);
  uVar1 = m_audio_b_FWBaseArrayProp::Get((m_audio_b_FWBaseArrayProp *)(this + 0x260),1);
  (**(code **)**(undefined8 **)(this + 0x298))(*(undefined8 **)(this + 0x298),2,uVar1);
  return 0;
}
