// com_m_audio_FW1814Device::BusReset
// Entry: 0000e53a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::BusReset() */

void __thiscall com_m_audio_FW1814Device::BusReset(com_m_audio_FW1814Device *this)

{
  undefined8 uVar1;
  CallCenter *pCVar2;
  undefined8 *puVar3;
  
  uVar1 = (**(code **)(*(long *)this + 0x688))(this);
  pCVar2 = operator_new(0x20);
  if (pCVar2 != (CallCenter *)0x0) {
    *(undefined ***)pCVar2 = &PTR__TFunctor_000367e0;
    *(undefined8 *)(pCVar2 + 8) = 0;
    *(com_m_audio_FW1814Device **)(pCVar2 + 0x18) = this;
    *(undefined8 *)(pCVar2 + 0x10) = uVar1;
    if (this == (com_m_audio_FW1814Device *)0x0) {
      puVar3 = (undefined8 *)0x0;
    }
    else {
      puVar3 = operator_new(0x20);
      if (puVar3 != (undefined8 *)0x0) {
        *puVar3 = &PTR_Call_00036890;
        puVar3[1] = this;
        puVar3[2] = 0xbf1;
        puVar3[3] = 0;
      }
    }
    *(undefined8 **)(pCVar2 + 8) = puVar3;
    m_audio_FWb::UKernelCallHelpers::InvokeOnNewThread(pCVar2);
    return;
  }
  return;
}
