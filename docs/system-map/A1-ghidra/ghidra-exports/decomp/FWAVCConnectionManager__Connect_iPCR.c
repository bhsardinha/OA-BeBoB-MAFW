// m_audio_b_FWAVCConnectionManager::Connect_iPCR
// Entry: 000247ba
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::Connect_iPCR(unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWAVCConnectionManager::Connect_iPCR
          (m_audio_b_FWAVCConnectionManager *this,uint param_1,uint param_2)

{
  undefined4 uVar1;
  IOFireWireBus *pIVar2;
  long *plVar3;
  undefined8 uVar4;
  undefined4 *puVar5;
  long lVar6;
  
  pIVar2 = (IOFireWireBus *)IOFireWireNub::getBus();
  plVar3 = (long *)IOFireWirePCRSpace::getPCRAddressSpace(pIVar2);
  if (plVar3 == (long *)0x0) {
    uVar4 = 0xe00002bc;
  }
  else {
    lVar6 = *(long *)(this + 0x20);
    if (param_1 < *(uint *)(this + 0x28) && -1 < (int)param_1) {
      lVar6 = lVar6 + (long)(int)param_1 * 4;
    }
    uVar4 = (**(code **)(*plVar3 + 0x1b8))(plVar3,this,HandlePCRLock,lVar6);
    if ((int)uVar4 == 0) {
      puVar5 = *(undefined4 **)(this + 0x20);
      if (param_1 < *(uint *)(this + 0x28) && -1 < (int)param_1) {
        puVar5 = puVar5 + (int)param_1;
      }
      uVar1 = (**(code **)(*plVar3 + 0x1c8))(plVar3,*puVar5);
      puVar5 = *(undefined4 **)(this + 0x20);
      if (param_1 < *(uint *)(this + 0x28) && -1 < (int)param_1) {
        puVar5 = puVar5 + (int)param_1;
      }
      uVar4 = (**(code **)(*plVar3 + 0x1d0))(plVar3,*puVar5,uVar1,param_2 << 0x10 | 0xc1003c7a);
      if ((int)uVar4 == 0) {
                    /* WARNING: Could not recover jumptable at 0x000248cc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        uVar4 = (**(code **)(*(long *)this + 0x148))(this,param_1,param_2);
        return uVar4;
      }
    }
  }
  return uVar4;
}
