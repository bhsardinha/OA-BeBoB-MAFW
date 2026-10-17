// m_audio_b_FWAVCConnectionManager::makeP2PInputConnection
// Entry: 000243f2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::makeP2PInputConnection(unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWAVCConnectionManager::makeP2PInputConnection
          (m_audio_b_FWAVCConnectionManager *this,uint param_1,uint param_2)

{
  long *plVar1;
  undefined8 uVar2;
  
  plVar1 = (long *)(**(code **)(**(long **)(this + 0x10) + 0x688))();
  if (plVar1 == (long *)0x0) {
    uVar2 = 0xe00002bc;
  }
  else {
    uVar2 = (**(code **)(*plVar1 + 0x1a0))
                      (plVar1,CoreMakeP2PInputConnection,this,param_1,param_2,0,0);
  }
  return uVar2;
}
