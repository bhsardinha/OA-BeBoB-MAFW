// CFW1814DeviceSettings::MapMixerChannelPairToChannelIDs
// Entry: 0005c47c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::MapMixerChannelPairToChannelIDs(unsigned short, unsigned short&, unsigned
   short&) */

void __thiscall
CFW1814DeviceSettings::MapMixerChannelPairToChannelIDs
          (CFW1814DeviceSettings *this,ushort param_1,ushort *param_2,ushort *param_3)

{
  switch(param_1) {
  case 1:
    *param_2 = 1;
    *param_3 = 2;
    break;
  case 2:
    *param_2 = 3;
    *param_3 = 4;
    break;
  case 3:
    *param_2 = 5;
    *param_3 = 6;
    break;
  case 4:
    *param_2 = 7;
    *param_3 = 8;
    break;
  case 5:
    *param_2 = 9;
    *param_3 = 10;
    break;
  case 6:
    *param_2 = 0xb;
    *param_3 = 0xc;
    break;
  case 7:
    *param_2 = 0xd;
    *param_3 = 0xe;
    break;
  case 8:
    *param_2 = 0xf;
    *param_3 = 0x10;
    break;
  case 9:
    *param_2 = 0x11;
    *param_3 = 0x12;
    break;
  case 10:
    *param_2 = 0x13;
    *param_3 = 0x14;
    break;
  case 0xb:
    *param_2 = 0x15;
    *param_3 = 0x16;
    break;
  case 0xc:
    *param_2 = 0x17;
    *param_3 = 0x18;
    break;
  case 0xd:
    *param_2 = 0x19;
    *param_3 = 0x1a;
    break;
  case 0xe:
    *param_2 = 0x1b;
    *param_3 = 0x1c;
    break;
  case 0xf:
    *param_2 = 0x1d;
    *param_3 = 0x1e;
    break;
  case 0x10:
    *param_2 = 0x1f;
    *param_3 = 0x20;
  }
  return;
}
