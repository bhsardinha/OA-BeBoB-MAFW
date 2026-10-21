// com_m_audio_FWMetaNub::SetOneProperty
// Entry: 0001ca24
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::SetOneProperty(OSString const*, OSObject*) */

undefined1 __thiscall
com_m_audio_FWMetaNub::SetOneProperty
          (com_m_audio_FWMetaNub *this,OSString *param_1,OSObject *param_2)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  long *plVar4;
  undefined8 uVar5;
  undefined1 uVar6;
  ushort local_2e;
  ushort local_2c [3];
  ushort local_26;
  ushort local_24 [2];
  
  if (*(uint **)(this + 0xe0) != (uint *)0x0) {
    IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0xe0),local_24,&local_26);
  }
  cVar1 = (**(code **)(*(long *)param_1 + 0x158))(param_1,"MAFirmwareUpgradeImage");
  if (cVar1 == '\0') {
    cVar1 = (**(code **)(*(long *)param_1 + 0x158))(param_1,"MAFirmwareStart");
    if (cVar1 == '\0') {
      cVar1 = (**(code **)(*(long *)param_1 + 0x158))(param_1,"MAFirmwareHalt");
      if (cVar1 == '\0') {
        uVar6 = (**(code **)(PTR_vtable_000300e0 + 0x270))(this,param_1,param_2);
        goto LAB_0001cb36;
      }
      iVar3 = FirmwareHalt(this);
    }
    else {
      iVar3 = FirmwareStart(this);
    }
  }
  else {
    plVar4 = (long *)OSMetaClassBase::safeMetaCast
                               ((OSMetaClassBase *)param_2,*(OSMetaClass **)PTR_metaClass_00030070);
    if (plVar4 == (long *)0x0) {
      return 0;
    }
    uVar5 = (**(code **)(*plVar4 + 0x178))(plVar4);
    *(undefined8 *)(this + 0x140) = uVar5;
    uVar2 = (**(code **)(*plVar4 + 0x140))(plVar4);
    *(undefined4 *)(this + 0x138) = uVar2;
    if ((*(long *)(this + 0x140) == 0) || (this[0xf0] == (com_m_audio_FWMetaNub)0x0)) {
      uVar6 = 0;
      goto LAB_0001cb36;
    }
    iVar3 = FirmwareUpdate(this);
  }
  uVar6 = iVar3 == 0;
LAB_0001cb36:
  if (*(uint **)(this + 0xe0) != (uint *)0x0) {
    IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0xe0),local_2c,&local_2e);
  }
  return uVar6;
}
