// m_audio_b_FWAVCConnectionManager::Disconnect_iPCR
// Entry: 000246ba
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::Disconnect_iPCR(unsigned int) */

undefined4 __thiscall
m_audio_b_FWAVCConnectionManager::Disconnect_iPCR
          (m_audio_b_FWAVCConnectionManager *this,uint param_1)

{
  undefined4 uVar1;
  IOFireWireBus *pIVar2;
  long *plVar3;
  undefined4 *puVar4;
  
  (**(code **)(*(long *)this + 0x150))(this,param_1);
  pIVar2 = (IOFireWireBus *)IOFireWireNub::getBus();
  plVar3 = (long *)IOFireWirePCRSpace::getPCRAddressSpace(pIVar2);
  if (plVar3 == (long *)0x0) {
    uVar1 = 0xe00002bc;
  }
  else {
    puVar4 = *(undefined4 **)(this + 0x20);
    if (param_1 < *(uint *)(this + 0x28) && -1 < (int)param_1) {
      puVar4 = puVar4 + (int)param_1;
    }
    uVar1 = (**(code **)(*plVar3 + 0x1c8))(plVar3,*puVar4);
    puVar4 = *(undefined4 **)(this + 0x20);
    if (param_1 < *(uint *)(this + 0x28) && -1 < (int)param_1) {
      puVar4 = puVar4 + (int)param_1;
    }
    uVar1 = (**(code **)(*plVar3 + 0x1d0))(plVar3,*puVar4,uVar1,0x7a);
    puVar4 = *(undefined4 **)(this + 0x20);
    if (param_1 < *(uint *)(this + 0x28) && -1 < (int)param_1) {
      puVar4 = puVar4 + (int)param_1;
    }
    (**(code **)(*plVar3 + 0x1c0))(plVar3,*puVar4);
  }
  return uVar1;
}
