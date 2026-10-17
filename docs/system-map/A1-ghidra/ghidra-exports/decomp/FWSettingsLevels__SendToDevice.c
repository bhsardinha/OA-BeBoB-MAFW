// FWSettingsLevels::SendToDevice
// Entry: 00021a36
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SendToDevice() */

undefined8 __thiscall FWSettingsLevels::SendToDevice(FWSettingsLevels *this)

{
  m_audio_b_FWBaseArrayProp *this_00;
  uint uVar1;
  undefined4 uVar2;
  ulong uVar3;
  uint uVar4;
  uint uVar5;
  
  if (this[0x8e2] != (FWSettingsLevels)0x0) {
    (*(code *)PTR_SendLevelsToDevice_00047fe8)(this + 0x8d0);
  }
  if (this[0x942] != (FWSettingsLevels)0x0) {
    (*(code *)PTR_SendLevelsToDevice_00047fe8)(this + 0x930);
  }
  if (*(long **)(this + 0x8c8) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x8c8) + 0x28))();
  }
  if (*(long **)(this + 0x8c0) != (long *)0x0) {
    uVar1 = (**(code **)(**(long **)(this + 0x8c0) + 0x10))();
    this_00 = (m_audio_b_FWBaseArrayProp *)(this + 0x10);
    uVar4 = 0;
    uVar2 = m_audio_b_FWBaseArrayProp::Get(this_00,0);
    (**(code **)**(undefined8 **)(this + 0x48))(*(undefined8 **)(this + 0x48),1,uVar2);
    uVar2 = m_audio_b_FWBaseArrayProp::Get(this_00,1);
    (**(code **)**(undefined8 **)(this + 0x48))(*(undefined8 **)(this + 0x48),2,uVar2);
    uVar2 = m_audio_b_FWBaseArrayProp::Get(this_00,2);
    (**(code **)**(undefined8 **)(this + 0x48))(*(undefined8 **)(this + 0x48),3,uVar2);
    uVar2 = m_audio_b_FWBaseArrayProp::Get(this_00,3);
    (**(code **)**(undefined8 **)(this + 0x48))(*(undefined8 **)(this + 0x48),4,uVar2);
    if (uVar1 >> 1 != 0) {
      do {
        uVar2 = m_audio_b_FWBaseArrayProp::Get((m_audio_b_FWBaseArrayProp *)(this + 0x990),uVar4);
        uVar5 = uVar4 + 2;
        uVar4 = uVar4 + 1;
        (**(code **)**(undefined8 **)(this + 0x9c8))(*(undefined8 **)(this + 0x9c8),uVar4,uVar2);
      } while (uVar5 <= uVar1 >> 1);
    }
  }
  if (*(long **)(this + 0x8b0) != (long *)0x0) {
    uVar3 = (**(code **)(**(long **)(this + 0x8b0) + 0x10))();
    if ((uVar3 >> 1 & 0x7fffffff) != 0) {
      uVar1 = 0;
      do {
        uVar2 = m_audio_b_FWBaseArrayProp::Get((m_audio_b_FWBaseArrayProp *)(this + 0x9f0),uVar1);
        uVar4 = uVar1 + 2;
        uVar1 = uVar1 + 1;
        (**(code **)**(undefined8 **)(this + 0xa28))(*(undefined8 **)(this + 0xa28),uVar1,uVar2);
      } while (uVar4 <= ((uint)(uVar3 >> 1) & 0x7fffffff));
    }
  }
  return 0;
}
