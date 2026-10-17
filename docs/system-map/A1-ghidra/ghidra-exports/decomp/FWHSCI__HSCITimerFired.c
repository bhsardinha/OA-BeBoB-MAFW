// m_audio_b_FWHSCI::HSCITimerFired
// Entry: 0001d600
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::HSCITimerFired() */

void __thiscall m_audio_b_FWHSCI::HSCITimerFired(m_audio_b_FWHSCI *this)

{
  uint uVar1;
  char cVar2;
  int iVar3;
  IOFWReadQuadCommand *this_00;
  size_t sVar4;
  ulong uVar5;
  long *plVar6;
  bool bVar7;
  ulong in_stack_ffffffffffffffa8;
  ushort local_3c [2];
  undefined2 local_38;
  undefined2 local_36;
  undefined4 local_34;
  undefined2 local_30;
  undefined2 local_2e;
  undefined4 local_2c;
  undefined2 local_28;
  undefined2 local_26;
  undefined4 local_24;
  
  local_38 = 0;
  local_36 = 0xdead;
  local_34 = 0xcafebabe;
  uVar1 = *(uint *)(this + 0xc0);
  IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0x10),local_3c);
  local_36 = 0xffc7;
  local_34 = 0x600000;
  uVar5 = (ulong)(uVar1 >> 2);
  sVar4 = uVar5 * 4;
  _bzero(*(void **)(this + 200),sVar4);
  plVar6 = *(long **)(this + 0xa0);
  if (plVar6 != (long *)0x0) {
    local_28 = local_38;
    local_26 = local_36;
    local_24 = local_34;
    iVar3 = (**(code **)(*plVar6 + 0x1e0))
                      (plVar6,&local_28,*(undefined8 *)(this + 200),uVar5,FWHSCIDeviceCallback,this,
                       in_stack_ffffffffffffffa8 & 0xffffffff00000000);
    bVar7 = iVar3 == 0;
    goto LAB_0001d750;
  }
  this_00 = OSObject::operator_new((OSObject *)&section_000000b8.addr,sVar4);
  if (this_00 == (IOFWReadQuadCommand *)0x0) {
    *(undefined8 *)(this + 0xa0) = 0;
    plVar6 = (long *)0x0;
LAB_0001d6f3:
    (**(code **)(*plVar6 + 0x28))();
    *(undefined8 *)(this + 0xa0) = 0;
  }
  else {
    IOFWReadQuadCommand::IOFWReadQuadCommand(this_00);
    *(IOFWReadQuadCommand **)(this + 0xa0) = this_00;
    local_30 = local_38;
    local_2e = local_36;
    local_2c = local_34;
    cVar2 = (**(code **)(*(long *)this_00 + 0x1d0))
                      (this_00,*(undefined8 *)(this + 0x10),&local_30,*(undefined8 *)(this + 200),
                       uVar5,FWHSCIDeviceCallback,this,0);
    if (cVar2 == '\0') {
      plVar6 = *(long **)(this + 0xa0);
      goto LAB_0001d6f3;
    }
  }
  bVar7 = true;
LAB_0001d750:
  plVar6 = *(long **)(this + 0xa0);
  if ((plVar6 != (long *)0x0) && (bVar7)) {
    iVar3 = (**(code **)(*plVar6 + 0x140))(plVar6,0);
    if (iVar3 != 0) {
      (**(code **)(**(long **)(this + 0x30) + 0x1d0))(*(long **)(this + 0x30),1000);
    }
  }
  return;
}
