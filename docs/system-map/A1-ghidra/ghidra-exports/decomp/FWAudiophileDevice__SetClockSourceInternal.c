// com_m_audio_FWAudiophileDevice::SetClockSourceInternal
// Entry: 0000b9d4  Size: 123 bytes
// Prototype: undefined SetClockSourceInternal(uint param_1, bool param_2)


/* com_m_audio_FWAudiophileDevice::SetClockSourceInternal(unsigned int, bool) */

undefined4 com_m_audio_FWAudiophileDevice::SetClockSourceInternal(uint param_1,bool param_2)

{
  long *plVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined7 in_register_00000031;
  undefined8 uVar4;
  undefined4 in_register_0000003c;
  
  plVar1 = *(long **)((FWRev2AudioDevice *)CONCAT44(in_register_0000003c,param_1) + 0xe8);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0xa50))(plVar1);
  }
  uVar3 = (uint)CONCAT71(in_register_00000031,param_2);
  if (uVar3 < 0x40000000) {
LAB_0000ba06:
    uVar2 = 0xe00002c2;
  }
  else {
    if ((uVar3 & 3) == 1) {
      uVar4 = 4;
    }
    else {
      if (param_2) goto LAB_0000ba06;
      uVar4 = 0;
    }
    uVar2 = FWRev2AudioDevice::AVCSetupSyncStream
                      ((FWRev2AudioDevice *)CONCAT44(in_register_0000003c,param_1),uVar4,1);
  }
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0xa58))(plVar1);
  }
  return uVar2;
}


