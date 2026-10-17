// m_audio_b_FWPairEngine::initHardware
// Entry: 0001aff8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::initHardware(IOService*) */

undefined8 m_audio_b_FWPairEngine::initHardware(IOService *param_1)

{
  undefined4 uVar1;
  code *pcVar2;
  char cVar3;
  m_audio_b_FWPairStream *pmVar4;
  undefined8 uVar5;
  uint uVar6;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined4 local_28;
  undefined4 local_24;
  ulong uVar7;
  
  local_30 = DAT_0002fc90;
  local_38 = DAT_0002fc88;
  local_40 = ResetIOFormat(_IOAudioSampleRate*)::C_58;
  cVar3 = (*(code *)PTR_initHardware_00042d60)(param_1);
  if (cVar3 != '\0') {
    local_28 = 0xac44;
    local_24 = 0;
    (**(code **)(*(long *)param_1 + 0xb58))(param_1,&local_28);
    (**(code **)(*(long *)param_1 + 0xb40))
              (param_1,*(undefined4 *)(*(long *)(param_1 + 0x118) + 0x88));
    uVar6 = *(int *)(param_1 + 0x88) << 3;
    uVar7 = (ulong)uVar6;
    *(uint *)(param_1 + 0x178) = uVar6;
    cVar3 = m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes
                      ((TRingBuffer *)(param_1 + 0x148),uVar6);
    if (cVar3 != '\0') {
      *(undefined8 *)(param_1 + 0x170) = *(undefined8 *)(param_1 + 0x150);
      if (param_1[0x17d] == (IOService)0x0) {
LAB_0001b190:
        if (param_1[0x17c] == (IOService)0x0) {
LAB_0001b242:
          (**(code **)(*(long *)param_1 + 0xbf8))(param_1,*(long *)(param_1 + 0x118) + 0x8c);
          (**(code **)(*(long *)param_1 + 0xb80))(param_1,0);
          uVar5 = 1;
          goto LAB_0001b26b;
        }
        pmVar4 = OSObject::operator_new((OSObject *)&section_00000108.reserved2,uVar7);
        if (pmVar4 == (m_audio_b_FWPairStream *)0x0) {
          *(undefined8 *)(param_1 + 0x140) = 0;
        }
        else {
          m_audio_b_FWPairStream::m_audio_b_FWPairStream(pmVar4);
          *(m_audio_b_FWPairStream **)(param_1 + 0x140) = pmVar4;
          cVar3 = (**(code **)(*(long *)pmVar4 + 0x9d0))(pmVar4,param_1,1,1,0,0);
          if (cVar3 != '\0') {
            (**(code **)(**(long **)(param_1 + 0x140) + 0xa58))
                      (*(long **)(param_1 + 0x140),&local_40,&local_28,&local_28,0,0);
            (**(code **)(**(long **)(param_1 + 0x140) + 0x9e0))
                      (*(long **)(param_1 + 0x140),*(undefined8 *)(param_1 + 0x170),
                       *(undefined4 *)(param_1 + 0x178));
            (**(code **)(*(long *)param_1 + 0xa00))(param_1,*(undefined8 *)(param_1 + 0x140));
            goto LAB_0001b242;
          }
        }
      }
      else {
        pmVar4 = OSObject::operator_new((OSObject *)&section_00000108.reserved2,uVar7);
        if (pmVar4 == (m_audio_b_FWPairStream *)0x0) {
          *(undefined8 *)(param_1 + 0x138) = 0;
        }
        else {
          m_audio_b_FWPairStream::m_audio_b_FWPairStream(pmVar4);
          *(m_audio_b_FWPairStream **)(param_1 + 0x138) = pmVar4;
          cVar3 = (**(code **)(*(long *)pmVar4 + 0x9d0))(pmVar4,param_1,0,1,0,0);
          if (cVar3 != '\0') {
            (**(code **)(**(long **)(param_1 + 0x138) + 0xa58))
                      (*(long **)(param_1 + 0x138),&local_40,&local_28,&local_28,0,0);
            (**(code **)(**(long **)(param_1 + 0x138) + 0x9e0))
                      (*(long **)(param_1 + 0x138),*(undefined8 *)(param_1 + 0x170),
                       *(undefined4 *)(param_1 + 0x178));
            uVar1 = *(undefined4 *)(param_1 + 0x178);
            pcVar2 = *(code **)(**(long **)(param_1 + 0x138) + 0x9f8);
            uVar5 = (**(code **)(**(long **)(param_1 + 0x118) + 0xc08))();
            (*pcVar2)(*(undefined8 *)(param_1 + 0x138),uVar5,uVar1);
            uVar7 = *(ulong *)(param_1 + 0x138);
            (**(code **)(*(long *)param_1 + 0xa00))(param_1);
            goto LAB_0001b190;
          }
        }
      }
    }
  }
  uVar5 = 0;
LAB_0001b26b:
  if (*(long **)(param_1 + 0x138) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0x138) + 0x28))();
  }
  if (*(long **)(param_1 + 0x140) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0x140) + 0x28))();
  }
  param_1[0x128] = (IOService)0x0;
  return uVar5;
}
