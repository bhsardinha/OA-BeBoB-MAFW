// CFW1814DeviceSettings::MapMixerChannelIDToChannelPair
// Entry: 0005c5b4
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::MapMixerChannelIDToChannelPair(unsigned short, unsigned short&) */

void __thiscall
CFW1814DeviceSettings::MapMixerChannelIDToChannelPair
          (CFW1814DeviceSettings *this,ushort param_1,ushort *param_2)

{
  switch(param_1) {
  case 1:
  case 2:
    *param_2 = 1;
    break;
  case 3:
  case 4:
    *param_2 = 2;
    break;
  case 5:
  case 6:
    *param_2 = 3;
    break;
  case 7:
  case 8:
    *param_2 = 4;
    break;
  case 9:
  case 10:
    *param_2 = 5;
    break;
  case 0xb:
  case 0xc:
    *param_2 = 6;
    break;
  case 0xd:
  case 0xe:
    *param_2 = 7;
    break;
  case 0xf:
  case 0x10:
    *param_2 = 8;
    break;
  case 0x11:
  case 0x12:
    *param_2 = 9;
    break;
  case 0x13:
  case 0x14:
    *param_2 = 10;
    break;
  case 0x15:
  case 0x16:
    *param_2 = 0xb;
    break;
  case 0x17:
  case 0x18:
    *param_2 = 0xc;
    break;
  case 0x19:
  case 0x1a:
    *param_2 = 0xd;
    break;
  case 0x1b:
  case 0x1c:
    *param_2 = 0xe;
    break;
  case 0x1d:
  case 0x1e:
    *param_2 = 0xf;
    break;
  case 0x1f:
  case 0x20:
    *param_2 = 0x10;
    break;
  default:
    *param_2 = 0;
  }
  return;
}
