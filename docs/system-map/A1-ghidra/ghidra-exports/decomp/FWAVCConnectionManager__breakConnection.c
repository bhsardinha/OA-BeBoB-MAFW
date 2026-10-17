// m_audio_b_FWAVCConnectionManager::breakConnection
// Entry: 00024454
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::breakConnection(unsigned int) */

undefined4 __thiscall
m_audio_b_FWAVCConnectionManager::breakConnection
          (m_audio_b_FWAVCConnectionManager *this,uint param_1)

{
  undefined4 uVar1;
  uint uVar2;
  long lVar3;
  long *plVar4;
  uint uVar5;
  
  plVar4 = *(long **)(this + 0x18);
  if (plVar4 == (long *)0x0) {
    uVar1 = 0xe00002be;
  }
  else {
    for (uVar5 = 0; uVar2 = (**(code **)(*plVar4 + 0x130))(), uVar5 < uVar2; uVar5 = uVar5 + 1) {
      lVar3 = (**(code **)(**(long **)(this + 0x18) + 0x1e8))(*(long **)(this + 0x18),uVar5);
      if (*(uint *)(lVar3 + 0xc) == param_1) {
        uVar1 = (**(code **)(*(long *)this + 0x170))
                          (this,param_1,0xffffffff,0,0xffffffff,0x80000000);
        (**(code **)(**(long **)(this + 0x18) + 0x1d8))(*(long **)(this + 0x18),uVar5);
        return uVar1;
      }
      plVar4 = *(long **)(this + 0x18);
    }
    uVar1 = 0xe00002f0;
  }
  return uVar1;
}
