// com_m_audio_FWMetaNub::FireWireReadCommand
// Entry: 0001c06e  Size: 387 bytes
// Prototype: undefined FireWireReadCommand(com_m_audio_FWMetaNub * this, FWAddressStruct * param_1, uint param_2, void * param_3)


/* com_m_audio_FWMetaNub::FireWireReadCommand(FWAddressStruct&, unsigned int, void*) */

int __thiscall
com_m_audio_FWMetaNub::FireWireReadCommand
          (com_m_audio_FWMetaNub *this,FWAddressStruct *param_1,uint param_2,void *param_3)

{
  char cVar1;
  int iVar2;
  long *plVar3;
  void *pvVar4;
  IOFWReadCommand *this_00;
  ulong uVar5;
  uint uVar6;
  undefined2 local_38;
  undefined2 local_36;
  undefined4 local_34;
  
  plVar3 = (long *)IOBufferMemoryDescriptor::withCapacity((ulong)param_2,3,false);
  if (plVar3 == (long *)0x0) {
    iVar2 = -0x1ffffd44;
  }
  else {
    uVar5 = 0;
    (**(code **)(*plVar3 + 0x1f0))(plVar3);
    iVar2 = -0x1ffffd44;
    uVar6 = 0xffffffff;
    while (uVar6 = uVar6 + 1, uVar6 < 10) {
      this_00 = OSObject::operator_new((OSObject *)(section_000000b8.segname + 8),uVar5);
      if (this_00 == (IOFWReadCommand *)0x0) break;
      IOFWReadCommand::IOFWReadCommand(this_00);
      local_38 = *(undefined2 *)param_1;
      local_36 = *(undefined2 *)(param_1 + 2);
      local_34 = *(undefined4 *)(param_1 + 4);
      cVar1 = (**(code **)(*(long *)this_00 + 0x178))
                        (this_00,*(undefined8 *)(this + 0xe0),&local_38,plVar3,0,0,0);
      if (cVar1 == '\0') break;
      *(undefined4 *)(this_00 + 0x48) = 10000000;
      uVar5 = 0;
      iVar2 = (**(code **)(*(long *)this_00 + 0x140))(this_00);
      (**(code **)(*(long *)this_00 + 0x28))(this_00);
      if (iVar2 != -0x1ffffd29) {
        if (iVar2 == 0) {
          pvVar4 = (void *)(**(code **)(*plVar3 + 0x2e0))(plVar3);
          _memcpy(param_3,pvVar4,(ulong)param_2);
        }
        break;
      }
      _IOSleep(0x9c4);
      if (this[0xf4] != (com_m_audio_FWMetaNub)0x0) break;
      cVar1 = IOService::isInactive();
      if (cVar1 != '\0') break;
    }
    (**(code **)(*plVar3 + 0x1f8))(plVar3,0);
    (**(code **)(*plVar3 + 0x28))(plVar3);
  }
  return iVar2;
}


