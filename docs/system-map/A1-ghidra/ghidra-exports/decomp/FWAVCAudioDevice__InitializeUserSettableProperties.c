// m_audio_b_FWAVCAudioDevice::InitializeUserSettableProperties
// Entry: 00023b8a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAVCAudioDevice::InitializeUserSettableProperties() */

undefined8 __thiscall
m_audio_b_FWAVCAudioDevice::InitializeUserSettableProperties(m_audio_b_FWAVCAudioDevice *this)

{
  undefined8 uVar1;
  long *plVar2;
  undefined8 *puVar3;
  
  uVar1 = (*(code *)PTR_InitializeUserSettableProperties_00031c98)(this);
  if ((int)uVar1 == 0) {
    FirmwareCacheInfoRegisters(this,(IOService *)0x0);
    plVar2 = (long *)OSData::withBytes(this + 0x278,0x50);
    if (plVar2 != (long *)0x0) {
      (**(code **)(*(long *)this + 0x268))(this,"MAFirmwareInfo",plVar2);
      (**(code **)(*plVar2 + 0x28))(plVar2);
    }
    if (this != (m_audio_b_FWAVCAudioDevice *)0x0) {
      uVar1 = *(undefined8 *)(this + 0xf8);
      if (*(void **)(this + 0x150) != (void *)0x0) {
        operator_delete(*(void **)(this + 0x150));
      }
      puVar3 = operator_new(0x20);
      if (puVar3 != (undefined8 *)0x0) {
        *puVar3 = &PTR_Call_00048ef0;
        puVar3[1] = this;
        puVar3[2] = SetClockSource;
        puVar3[3] = 0;
        *(undefined8 **)(this + 0x150) = puVar3;
        uVar1 = (*(code *)PTR_init_00037a68)(this + 0x118,uVar1,"MAClockSource");
        return uVar1;
      }
      *(undefined8 *)(this + 0x150) = 0;
    }
    uVar1 = 0xe00002bc;
  }
  return uVar1;
}
