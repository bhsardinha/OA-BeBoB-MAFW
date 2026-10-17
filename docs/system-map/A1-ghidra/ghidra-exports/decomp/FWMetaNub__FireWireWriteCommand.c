// com_m_audio_FWMetaNub::FireWireWriteCommand
// Entry: 0001bd3e  Size: 253 bytes
// Prototype: undefined FireWireWriteCommand(com_m_audio_FWMetaNub * this, FWAddressStruct * param_1, uint param_2, void * param_3)


/* com_m_audio_FWMetaNub::FireWireWriteCommand(FWAddressStruct&, unsigned int, void*) */

undefined4 __thiscall
com_m_audio_FWMetaNub::FireWireWriteCommand
          (com_m_audio_FWMetaNub *this,FWAddressStruct *param_1,uint param_2,void *param_3)

{
  char cVar1;
  undefined4 uVar2;
  long *plVar3;
  IOFWWriteCommand *this_00;
  ulong uVar4;
  undefined2 local_30;
  undefined2 local_2e;
  undefined4 local_2c;
  
  plVar3 = (long *)IOBufferMemoryDescriptor::withBytes(param_3,(ulong)param_2,3,false);
  if (plVar3 == (long *)0x0) {
    uVar2 = 0xe00002bc;
  }
  else {
    uVar4 = 0;
    (**(code **)(*plVar3 + 0x1f0))(plVar3);
    this_00 = OSObject::operator_new((OSObject *)&section_000000b8.addr,uVar4);
    if (this_00 == (IOFWWriteCommand *)0x0) {
      uVar2 = 0xe00002bc;
    }
    else {
      IOFWWriteCommand::IOFWWriteCommand(this_00);
      local_30 = *(undefined2 *)param_1;
      local_2e = *(undefined2 *)(param_1 + 2);
      local_2c = *(undefined4 *)(param_1 + 4);
      cVar1 = (**(code **)(*(long *)this_00 + 0x178))
                        (this_00,*(undefined8 *)(this + 0xe0),&local_30,plVar3,0,0,0);
      if (cVar1 == '\0') {
        uVar2 = 0xe00002bc;
      }
      else {
        *(undefined4 *)(this_00 + 0x48) = 10000000;
        uVar2 = (**(code **)(*(long *)this_00 + 0x140))(this_00,0);
      }
      (**(code **)(*(long *)this_00 + 0x28))(this_00);
    }
    (**(code **)(*plVar3 + 0x1f8))(plVar3,0);
    (**(code **)(*plVar3 + 0x28))(plVar3);
  }
  return uVar2;
}


