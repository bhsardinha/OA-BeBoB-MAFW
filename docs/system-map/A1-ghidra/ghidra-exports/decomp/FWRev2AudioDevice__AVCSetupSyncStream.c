// FWRev2AudioDevice::AVCSetupSyncStream
// Entry: 00012c60
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::AVCSetupSyncStream(FWRev2AudioDevice::SyncStreamType, unsigned int) */

ulong __thiscall
FWRev2AudioDevice::AVCSetupSyncStream(FWRev2AudioDevice *this,undefined4 param_2,undefined1 param_3)

{
  uint uVar1;
  int iVar2;
  undefined1 *puVar3;
  ulong uVar4;
  undefined1 *puVar5;
  undefined1 local_54 [12];
  undefined1 *local_48;
  uint local_40;
  undefined1 local_3c [16];
  undefined4 local_2c;
  
  local_40 = 8;
  local_48 = local_3c;
  _bzero(local_48,8);
  _bzero(local_54,8);
  *local_48 = 1;
  puVar3 = local_48;
  if (1 < local_40) {
    puVar3 = local_48 + 1;
  }
  *puVar3 = 0xff;
  puVar3 = local_48;
  if (2 < local_40) {
    puVar3 = local_48 + 2;
  }
  *puVar3 = 0x1a;
  puVar3 = local_48;
  if (3 < local_40) {
    puVar3 = local_48 + 3;
  }
  *puVar3 = 0xf;
  puVar3 = local_48;
  if (4 < local_40) {
    puVar3 = local_48 + 4;
  }
  *puVar3 = 0xff;
  puVar3 = local_48;
  if (5 < local_40) {
    puVar3 = local_48 + 5;
  }
  *puVar3 = 0xfe;
  puVar3 = local_48;
  if (6 < local_40) {
    puVar3 = local_48 + 6;
  }
  *puVar3 = 0x60;
  puVar3 = local_48;
  if (7 < local_40) {
    puVar3 = local_48 + 7;
  }
  *puVar3 = param_3;
  local_2c = 8;
  uVar1 = (**(code **)(**(long **)(this + 0x268) + 0x8b8))
                    (*(long **)(this + 0x268),local_48,8,local_54,&local_2c);
  uVar4 = (ulong)uVar1;
  if (uVar1 == 0) {
    puVar3 = local_48;
    if (5 < local_40) {
      puVar3 = local_48 + 5;
    }
    puVar5 = local_48;
    if (4 < local_40) {
      puVar5 = local_48 + 4;
    }
    iVar2 = (**(code **)(*(long *)this + 0xbc0))(this,param_2,param_3,puVar5,puVar3);
    if (iVar2 == 0) {
      *local_48 = 0;
      puVar3 = local_48;
      if (1 < local_40) {
        puVar3 = local_48 + 1;
      }
      *puVar3 = 0xff;
      puVar3 = local_48;
      if (2 < local_40) {
        puVar3 = local_48 + 2;
      }
      *puVar3 = 0x1a;
      puVar3 = local_48;
      if (3 < local_40) {
        puVar3 = local_48 + 3;
      }
      *puVar3 = 0xf;
      puVar3 = local_48;
      if (6 < local_40) {
        puVar3 = local_48 + 6;
      }
      *puVar3 = 0x60;
      puVar3 = local_48;
      if (7 < local_40) {
        puVar3 = local_48 + 7;
      }
      *puVar3 = param_3;
      uVar4 = (**(code **)(**(long **)(this + 0x268) + 0x8b8))
                        (*(long **)(this + 0x268),local_48,8,local_54,&local_2c);
    }
  }
  return uVar4;
}
