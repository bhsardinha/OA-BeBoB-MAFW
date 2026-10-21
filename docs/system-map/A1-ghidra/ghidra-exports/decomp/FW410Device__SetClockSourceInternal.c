// com_m_audio_FW410Device::SetClockSourceInternal
// Entry: 00014eda
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* WARNING: Removing unreachable block (ram,0x00014f2a) */
/* com_m_audio_FW410Device::SetClockSourceInternal(unsigned int, bool) */

undefined4 com_m_audio_FW410Device::SetClockSourceInternal(uint param_1,bool param_2)

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
LAB_00014f0c:
    uVar2 = 0xe00002c2;
  }
  else {
    if ((uVar3 & 3) == 1) {
      uVar4 = 3;
    }
    else {
      if (param_2) goto LAB_00014f0c;
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
