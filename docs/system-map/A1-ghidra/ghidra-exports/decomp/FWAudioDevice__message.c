// m_audio_b_FWAudioDevice::message
// Entry: 00003344
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e8)

/* m_audio_b_FWAudioDevice::message(unsigned int, IOService*, void*) */

undefined8 m_audio_b_FWAudioDevice::message(uint param_1,IOService *param_2,void *param_3)

{
  undefined8 uVar1;
  CallCenter *pCVar2;
  undefined8 *puVar3;
  undefined4 in_register_0000003c;
  long *plVar4;
  
  plVar4 = (long *)CONCAT44(in_register_0000003c,param_1);
  if (((char)plVar4[0x4b] != '\0') && ((int)param_2 == -0x1fffffd0)) {
    if (*(char *)((long)plVar4 + 0x259) == '\0') {
      (**(code **)(*plVar4 + 0xb20))(plVar4);
    }
    else {
      uVar1 = (**(code **)(*plVar4 + 0x688))(plVar4);
      pCVar2 = operator_new(0x20);
      if (pCVar2 != (CallCenter *)0x0) {
        *(undefined ***)pCVar2 = &PTR__TFunctor_00030fe0;
        *(undefined8 *)(pCVar2 + 8) = 0;
        *(long **)(pCVar2 + 0x18) = plVar4;
        *(undefined8 *)(pCVar2 + 0x10) = uVar1;
        if (plVar4 == (long *)0x0) {
          puVar3 = (undefined8 *)0x0;
        }
        else {
          puVar3 = operator_new(0x20);
          if (puVar3 != (undefined8 *)0x0) {
            *puVar3 = &PTR_Call_00031010;
            puVar3[1] = plVar4;
            puVar3[2] = WakeAfterSleep;
            puVar3[3] = 0;
          }
        }
        *(undefined8 **)(pCVar2 + 8) = puVar3;
        m_audio_FWb::UKernelCallHelpers::InvokeOnNewThread(pCVar2);
      }
      if ((long *)plVar4[0x1e] != (long *)0x0) {
        (**(code **)(*(long *)plVar4[0x1e] + 0x140))();
      }
      *(undefined1 *)((long)plVar4 + 0x259) = 0;
    }
  }
  return 0;
}
