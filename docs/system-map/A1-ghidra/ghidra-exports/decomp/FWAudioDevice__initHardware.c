// m_audio_b_FWAudioDevice::initHardware
// Entry: 0000342e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e5)

/* m_audio_b_FWAudioDevice::initHardware(IOService*) */

undefined8 m_audio_b_FWAudioDevice::initHardware(IOService *param_1)

{
  code *pcVar1;
  char cVar2;
  int iVar3;
  uint uVar4;
  undefined8 uVar5;
  long lVar6;
  long *plVar7;
  void *pvVar8;
  
  (**(code **)(*(long *)param_1 + 0x270))
            (param_1,"IOAudioDeviceConfigurationApplication","com.m-audio.appc.MAfw");
  cVar2 = (*(code *)PTR_initHardware_00047be0)(param_1);
  if (cVar2 != '\0') {
    pcVar1 = *(code **)(*(long *)param_1 + 0x958);
    uVar5 = (**(code **)(*(long *)param_1 + 0xa50))(param_1);
    (*pcVar1)(param_1,uVar5);
    pcVar1 = *(code **)(*(long *)param_1 + 0x960);
    uVar5 = (**(code **)(*(long *)param_1 + 0xa58))(param_1);
    (*pcVar1)(param_1,uVar5);
    (**(code **)(*(long *)param_1 + 0x968))(param_1,"M-Audio");
    (**(code **)(*(long *)param_1 + 0x850))(param_1,0x31333934);
    pcVar1 = *(code **)(*(long *)param_1 + 0x268);
    uVar5 = (**(code **)(**(long **)(param_1 + 0xe0) + 0x2b8))
                      (*(long **)(param_1 + 0xe0),"FireWire Product Name");
    (*pcVar1)(param_1,"MAIconName",uVar5);
    pcVar1 = *(code **)(*(long *)param_1 + 0x278);
    iVar3 = (**(code **)(*(long *)param_1 + 0xb30))(param_1);
    (*pcVar1)(param_1,"MAMaxBytesPerCycle",iVar3 != 0);
    lVar6 = (**(code **)(*(long *)param_1 + 0xa28))(param_1);
    *(long *)(param_1 + 0xf0) = lVar6;
    if ((lVar6 != 0) && (cVar2 = (**(code **)(*(long *)param_1 + 0xa30))(param_1), cVar2 != '\0')) {
      (**(code **)(*(long *)param_1 + 0xa78))
                (param_1,*(undefined4 *)(*(long *)(param_1 + 0xe8) + 0x220));
      iVar3 = m_audio_b_FWBaseEngine::CompletePendingStateChanges
                        (*(m_audio_b_FWBaseEngine **)(param_1 + 0xe8));
      if ((iVar3 == 0) && (iVar3 = (**(code **)(*(long *)param_1 + 0xb28))(param_1), iVar3 == 0)) {
        if (*(long *)(param_1 + 0x100) != 0) {
LAB_0000363c:
          param_1[0x25a] = (IOService)0x1;
          SetStaticPropertyAndNotifyUserClients((m_audio_b_FWAudioDevice *)param_1,"MARateLocked",1)
          ;
          if (*(long *)(param_1 + 0x108) != 0) {
            uVar4 = (**(code **)(*(long *)param_1 + 0xaf0))(param_1);
            _bzero(*(void **)(param_1 + 0x108),(ulong)uVar4 << 2);
            uVar4 = (**(code **)(*(long *)param_1 + 0xaf0))(param_1);
            _bzero(*(void **)(param_1 + 0x110),(ulong)uVar4 << 2);
          }
          (**(code **)(*(long *)param_1 + 0xb08))(param_1);
          AddToDeviceList();
          (**(code **)(**(long **)(param_1 + 0xe8) + 0xc70))();
          param_1[600] = (IOService)0x1;
          return 1;
        }
        plVar7 = (long *)IOBufferMemoryDescriptor::withOptions(0x10000,0x1000,0x1000);
        *(long **)(param_1 + 0x100) = plVar7;
        if (plVar7 != (long *)0x0) {
          iVar3 = (**(code **)(*plVar7 + 0x1f0))(plVar7,1);
          if (iVar3 == 0) {
            uVar5 = (**(code **)(**(long **)(param_1 + 0x100) + 0x2e0))();
            *(undefined8 *)(param_1 + 0x108) = uVar5;
            uVar4 = (**(code **)(*(long *)param_1 + 0xaf0))(param_1);
            pvVar8 = operator_new__((ulong)uVar4 << 2);
            *(void **)(param_1 + 0x110) = pvVar8;
            goto LAB_0000363c;
          }
          (**(code **)(**(long **)(param_1 + 0x100) + 0x28))();
          *(undefined8 *)(param_1 + 0x100) = 0;
        }
      }
    }
  }
  if (*(long **)(param_1 + 0xf0) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0xf0) + 0x28))();
    *(undefined8 *)(param_1 + 0xf0) = 0;
  }
  param_1[600] = (IOService)0x0;
  return 0;
}
