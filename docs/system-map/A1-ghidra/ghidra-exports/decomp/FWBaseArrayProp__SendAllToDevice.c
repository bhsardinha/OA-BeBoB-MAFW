// m_audio_b_FWBaseArrayProp::SendAllToDevice
// Entry: 0000803c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::SendAllToDevice() */

undefined8 __thiscall m_audio_b_FWBaseArrayProp::SendAllToDevice(m_audio_b_FWBaseArrayProp *this)

{
  uint uVar1;
  undefined4 uVar2;
  uint uVar3;
  
  if ((*(long *)(this + 0x38) != 0) && (*(int *)(this + 0x2c) != 0)) {
    uVar3 = 0;
    do {
      uVar2 = Get(this,uVar3);
      uVar1 = uVar3 + 2;
      uVar3 = uVar3 + 1;
      (**(code **)**(undefined8 **)(this + 0x38))(*(undefined8 **)(this + 0x38),uVar3,uVar2);
    } while (uVar1 <= *(uint *)(this + 0x2c));
  }
  return 0;
}
