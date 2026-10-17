// com_m_audio_FWMetaNub::FirmwareCacheInfoRegisters
// Entry: 0001cc9e  Size: 188 bytes
// Prototype: undefined FirmwareCacheInfoRegisters(IOService * param_1)


/* com_m_audio_FWMetaNub::FirmwareCacheInfoRegisters(IOService*) */

undefined4 com_m_audio_FWMetaNub::FirmwareCacheInfoRegisters(IOService *param_1)

{
  IOService *pIVar1;
  undefined4 uVar2;
  long *plVar3;
  undefined2 local_28;
  undefined2 local_26;
  undefined4 local_24;
  
  local_28 = 0;
  local_26 = 0xdead;
  local_24 = 0xcafebabe;
  if (*(long *)(param_1 + 0xe0) == 0) {
    uVar2 = 0xe00002bc;
  }
  else {
    pIVar1 = param_1 + 0x90;
    _bzero(pIVar1,0x50);
    local_26 = 0xffff;
    local_24 = 0xc8020000;
    uVar2 = FireWireReadCommand((com_m_audio_FWMetaNub *)param_1,(FWAddressStruct *)&local_28,0x50,
                                pIVar1);
    param_1[0xf0] = (IOService)(*(int *)(param_1 + 0x9c) != 0);
    plVar3 = (long *)OSData::withBytes(pIVar1,0x50);
    if (plVar3 != (long *)0x0) {
      (**(code **)(*(long *)param_1 + 0x268))(param_1,"MAFirmwareInfo",plVar3);
      (**(code **)(*plVar3 + 0x28))(plVar3);
    }
  }
  return uVar2;
}


