// m_audio_b_FWAVCAudioDevice::AVCVendorCommand
// Entry: 00023970
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i2)

/* m_audio_b_FWAVCAudioDevice::AVCVendorCommand(unsigned char, unsigned char, unsigned int, unsigned
   char*) */

undefined8
m_audio_b_FWAVCAudioDevice::AVCVendorCommand
          (uchar param_1,uchar param_2,uint param_3,uchar *param_4)

{
  long lVar1;
  long *plVar2;
  uchar *puVar3;
  undefined8 uVar4;
  ulong uVar5;
  undefined7 in_register_00000039;
  long in_R8;
  undefined8 in_R9;
  uint uVar6;
  undefined1 *puVar7;
  undefined4 uVar8;
  undefined1 local_6c [20];
  uchar *local_58;
  uint local_50;
  uchar local_4c [24];
  undefined4 local_34;
  
  local_50 = 0x10;
  local_58 = local_4c;
  _bzero(local_58,0x10);
  uVar8 = 0x10;
  puVar7 = local_6c;
  _bzero(puVar7,0x10);
  uVar6 = (uint)param_4;
  if (uVar6 < 8) {
    *local_58 = '\0';
    puVar3 = local_58;
    if (1 < local_50) {
      puVar3 = local_58 + 1;
    }
    *puVar3 = 0xff;
    puVar3 = local_58;
    if (2 < local_50) {
      puVar3 = local_58 + 2;
    }
    *puVar3 = '\0';
    puVar3 = local_58;
    if (3 < local_50) {
      puVar3 = local_58 + 3;
    }
    *puVar3 = '\0';
    puVar3 = local_58;
    if (4 < local_50) {
      puVar3 = local_58 + 4;
    }
    *puVar3 = '\r';
    puVar3 = local_58;
    if (5 < local_50) {
      puVar3 = local_58 + 5;
    }
    *puVar3 = 'l';
    puVar3 = local_58;
    if (6 < local_50) {
      puVar3 = local_58 + 6;
    }
    *puVar3 = param_2;
    puVar3 = local_58;
    if (7 < local_50) {
      puVar3 = local_58 + 7;
    }
    *puVar3 = (uchar)param_3;
    puVar3 = local_58;
    if (8 < local_50) {
      puVar3 = local_58 + 8;
    }
    *puVar3 = (uchar)param_4;
    if (local_50 < uVar6 + 9) {
                    /* WARNING: Subroutine does not return */
      _panic(
            "\"array overrun in SetElements\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/../Shared/KernelDataStructures/TStaticArray.h:67"
            );
    }
    if (uVar6 != 0) {
      uVar5 = 9;
      do {
        local_58[uVar5 & 0xffffffff] = *(uchar *)(in_R8 + -9 + uVar5);
        lVar1 = (1 - ((ulong)param_4 & 0xffffffff)) + uVar5;
        uVar5 = uVar5 + 1;
      } while (lVar1 != 9);
    }
    plVar2 = *(long **)(CONCAT71(in_register_00000039,param_1) + 0x268);
    local_34 = uVar8;
    uVar4 = (**(code **)(*plVar2 + 0x8b8))
                      (plVar2,local_58,local_50,puVar7,&local_34,in_R9,puVar7,uVar8);
  }
  else {
    uVar4 = 0xe00002c2;
  }
  return uVar4;
}
