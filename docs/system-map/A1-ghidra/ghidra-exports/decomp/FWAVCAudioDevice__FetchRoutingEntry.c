// m_audio_b_FWAVCAudioDevice::FetchRoutingEntry
// Entry: 00022a24
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_b_FWAVCAudioDevice::FetchRoutingEntry(m_audio_b_FWAVCAudioDevice::Routing&, unsigned int,
   m_audio_b_FWAVCAudioDevice::Routing const*) */

undefined8 __thiscall
m_audio_b_FWAVCAudioDevice::FetchRoutingEntry
          (m_audio_b_FWAVCAudioDevice *this,Routing *param_1,uint param_2,Routing *param_3)

{
  uint uVar1;
  Routing *pRVar2;
  
  uVar1 = 0xffffffff;
  do {
    pRVar2 = param_3;
    uVar1 = uVar1 + 1;
    if (param_2 <= uVar1) {
      return 0;
    }
    param_3 = pRVar2 + 0x10;
  } while (((*(int *)pRVar2 != *(int *)param_1) || (*(int *)(pRVar2 + 4) != *(int *)(param_1 + 4)))
          || (*(int *)(pRVar2 + 8) != *(int *)(param_1 + 8)));
  *(int *)param_1 = *(int *)pRVar2;
  *(int *)(param_1 + 4) = *(int *)(pRVar2 + 4);
  *(int *)(param_1 + 8) = *(int *)(pRVar2 + 8);
  param_1[0xc] = pRVar2[0xc];
  param_1[0xd] = pRVar2[0xd];
  return 1;
}
