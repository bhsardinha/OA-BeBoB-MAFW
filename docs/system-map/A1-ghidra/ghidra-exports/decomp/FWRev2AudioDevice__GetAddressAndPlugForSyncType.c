// FWRev2AudioDevice::GetAddressAndPlugForSyncType
// Entry: 00012ae8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::GetAddressAndPlugForSyncType(FWRev2AudioDevice::SyncStreamType, unsigned char,
   unsigned char&, unsigned char&) */

undefined8 __thiscall
FWRev2AudioDevice::GetAddressAndPlugForSyncType
          (undefined8 param_1,undefined4 param_2,char param_3,undefined1 *param_4,char *param_5)

{
  char cVar1;
  
  switch(param_2) {
  case 0:
    *param_4 = 0x60;
    cVar1 = '\x01';
    if (param_3 != '\x01') {
      cVar1 = param_3 + '\x01';
    }
    *param_5 = cVar1;
    break;
  case 1:
    *param_4 = 0xff;
    *param_5 = -0x7f;
    break;
  case 2:
    *param_4 = 0xff;
    *param_5 = '\0';
    break;
  case 3:
    *param_4 = 0xff;
    *param_5 = -0x7d;
    break;
  case 4:
    *param_4 = 0xff;
    *param_5 = -0x7e;
    break;
  default:
    return 0xe00002c7;
  }
  return 0;
}
