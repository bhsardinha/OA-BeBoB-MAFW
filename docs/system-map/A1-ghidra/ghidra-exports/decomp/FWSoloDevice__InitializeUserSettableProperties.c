// com_m_audio_FWSoloDevice::InitializeUserSettableProperties
// Entry: 00017a54
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FWSoloDevice::InitializeUserSettableProperties() */

undefined8 __thiscall
com_m_audio_FWSoloDevice::InitializeUserSettableProperties(com_m_audio_FWSoloDevice *this)

{
  undefined8 uVar1;
  undefined8 *puVar2;
  
  uVar1 = (*(code *)PTR_InitializeUserSettableProperties_0003b4d8)(this);
  if ((int)uVar1 == 0) {
    (*(code *)PTR_Init_00048e88)(this + 0x2d0,this,this + 0xe38,0,this + 0xd70,0,0);
    if (this != (com_m_audio_FWSoloDevice *)0x0) {
      uVar1 = *(undefined8 *)(this + 0xf8);
      if (*(void **)(this + 0xd68) != (void *)0x0) {
        operator_delete(*(void **)(this + 0xd68));
      }
      puVar2 = operator_new(0x20);
      if (puVar2 != (undefined8 *)0x0) {
        *puVar2 = &PTR_Call_0003ed50;
        puVar2[1] = this;
        puVar2[2] = SetSPDIFMixerBypass;
        puVar2[3] = 0;
        *(undefined8 **)(this + 0xd68) = puVar2;
        uVar1 = (*(code *)PTR_init_00037a68)(this + 0xd30,uVar1,"MASPDIFMixerBypass");
        return uVar1;
      }
      *(undefined8 *)(this + 0xd68) = 0;
    }
    uVar1 = 0xe00002bc;
  }
  return uVar1;
}
