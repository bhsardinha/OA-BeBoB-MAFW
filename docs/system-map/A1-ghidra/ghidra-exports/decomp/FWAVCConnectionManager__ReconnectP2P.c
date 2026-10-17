// m_audio_b_FWAVCConnectionManager::ReconnectP2P
// Entry: 00024176
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::ReconnectP2P() */

void __thiscall
m_audio_b_FWAVCConnectionManager::ReconnectP2P(m_audio_b_FWAVCConnectionManager *this)

{
  int iVar1;
  uint uVar2;
  long lVar3;
  uint uVar4;
  
  if (*(long **)(this + 0x18) != (long *)0x0) {
    iVar1 = (**(code **)(**(long **)(this + 0x18) + 0x130))();
    if (iVar1 != 0) {
      _IOSleep(0x44c);
      uVar4 = 0;
      while( true ) {
        uVar2 = (**(code **)(**(long **)(this + 0x18) + 0x130))();
        if (uVar2 <= uVar4) break;
        lVar3 = (**(code **)(**(long **)(this + 0x18) + 0x1e8))(*(long **)(this + 0x18),uVar4);
        iVar1 = (**(code **)(*(long *)this + 0x170))
                          (this,*(undefined4 *)(lVar3 + 0xc),1,1,*(undefined4 *)(lVar3 + 0x10),
                           0x80000000);
        if (iVar1 == -0x1fff7fe0) break;
        uVar4 = uVar4 + 1;
      }
      if (*(long **)(*(long *)(this + 0x10) + 0xe8) != (long *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00024210. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (**(code **)(**(long **)(*(long *)(this + 0x10) + 0xe8) + 0xcb0))();
        return;
      }
    }
  }
  return;
}
