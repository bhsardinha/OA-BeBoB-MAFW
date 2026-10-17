// m_audio_b_FWAVCConnectionManager::free
// Entry: 00023e96
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::free() */

void __thiscall m_audio_b_FWAVCConnectionManager::free(m_audio_b_FWAVCConnectionManager *this)

{
  int iVar1;
  uint uVar2;
  long lVar3;
  long *plVar4;
  uint uVar5;
  
  if (*(long **)(this + 0x18) != (long *)0x0) {
    iVar1 = (**(code **)(**(long **)(this + 0x18) + 0x130))();
    plVar4 = *(long **)(this + 0x18);
    lVar3 = *plVar4;
    if (iVar1 != 0) {
      uVar5 = 0;
      do {
        lVar3 = (**(code **)(lVar3 + 0x1e8))(plVar4,uVar5);
        (**(code **)(*(long *)this + 0x170))
                  (this,*(undefined4 *)(lVar3 + 0xc),0xffffffff,0,0xffffffff,0x80000000);
        uVar2 = (**(code **)(**(long **)(this + 0x18) + 0x130))();
        uVar5 = uVar5 + 1;
        plVar4 = *(long **)(this + 0x18);
        lVar3 = *plVar4;
      } while (uVar5 < uVar2);
    }
    (**(code **)(lVar3 + 0x28))();
    *(undefined8 *)(this + 0x18) = 0;
  }
  (*(code *)PTR_free_0004a280)(this);
  return;
}
