// m_audio_b_FWAVCConnectionManager::BusReset
// Entry: 00024216
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::BusReset() */

void __thiscall m_audio_b_FWAVCConnectionManager::BusReset(m_audio_b_FWAVCConnectionManager *this)

{
  long *plVar1;
  undefined8 uVar2;
  CallCenter *pCVar3;
  undefined8 *puVar4;
  
  plVar1 = *(long **)(this + 0x18);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x20))(plVar1);
    (**(code **)(**(long **)(this + 0x10) + 0x20))();
    uVar2 = (**(code **)(**(long **)(this + 0x10) + 0x688))();
    pCVar3 = operator_new(0x20);
    if (pCVar3 != (CallCenter *)0x0) {
      *(undefined ***)pCVar3 = &PTR__TFunctor_00049be0;
      *(undefined8 *)(pCVar3 + 8) = 0;
      *(m_audio_b_FWAVCConnectionManager **)(pCVar3 + 0x18) = this;
      *(undefined8 *)(pCVar3 + 0x10) = uVar2;
      if (this == (m_audio_b_FWAVCConnectionManager *)0x0) {
        puVar4 = (undefined8 *)0x0;
      }
      else {
        puVar4 = operator_new(0x20);
        if (puVar4 != (undefined8 *)0x0) {
          *puVar4 = &PTR_Call_00049c10;
          puVar4[1] = this;
          puVar4[2] = ReconnectP2PThread;
          puVar4[3] = 0;
        }
      }
      *(undefined8 **)(pCVar3 + 8) = puVar4;
      m_audio_FWb::UKernelCallHelpers::InvokeOnNewThread(pCVar3);
      return;
    }
  }
  return;
}
