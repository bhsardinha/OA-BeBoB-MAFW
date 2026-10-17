// com_m_audio_FW1814Device::ReceiveControlPacket
// Entry: 0000e9ea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*) */

void __thiscall
com_m_audio_FW1814Device::ReceiveControlPacket
          (com_m_audio_FW1814Device *this,FW1814ControlPacketWithoutNodeID *param_1)

{
  com_m_audio_FW1814Device *pcVar1;
  ushort uVar2;
  char cVar3;
  long lVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  OSArray *local_30;
  OSArray *local_28;
  int local_1c;
  
  local_1c = 0;
  if ((*(long *)(this + 0xe8) == 0) || (*(byte *)(*(long *)(this + 0xe8) + 0x228) < 5)) {
    lVar4 = 0;
    do {
      uVar2 = *(ushort *)(param_1 + lVar4 * 4 + 6);
      *(uint *)(*(long *)(this + 0x108) + lVar4 * 8) =
           (uint)(ushort)(*(ushort *)(param_1 + lVar4 * 4 + 4) << 8 |
                         *(ushort *)(param_1 + lVar4 * 4 + 4) >> 8);
      *(uint *)(*(long *)(this + 0x108) + 4 + lVar4 * 8) = (uint)(ushort)(uVar2 << 8 | uVar2 >> 8);
      lVar4 = lVar4 + 1;
    } while (lVar4 != 0x13);
    lVar4 = 0;
    do {
      uVar6 = *(uint *)(*(long *)(this + 0x110) + 0x9c + lVar4 * 8);
      *(uint *)(*(long *)(this + 0x108) + 0x98 + lVar4 * 8) =
           *(uint *)(*(long *)(this + 0x110) + 0x98 + lVar4 * 8) & 0xffff;
      *(uint *)(*(long *)(this + 0x108) + 0x9c + lVar4 * 8) = uVar6 & 0xffff;
      lVar4 = lVar4 + 1;
    } while (lVar4 != 7);
    lVar4 = *(long *)(this + 0x108);
    if (((byte)this[0x1317] & 4) == 0) {
      *(undefined4 *)(lVar4 + 0x58) = *(undefined4 *)(*(long *)(this + 0x110) + 0xa8);
      lVar4 = *(long *)(this + 0x108);
      uVar5 = *(undefined4 *)(*(long *)(this + 0x110) + 0xac);
    }
    else {
      *(undefined4 *)(lVar4 + 0x58) = *(undefined4 *)(lVar4 + 0x90);
      lVar4 = *(long *)(this + 0x108);
      uVar5 = *(undefined4 *)(lVar4 + 0x94);
    }
    *(undefined4 *)(lVar4 + 0x5c) = uVar5;
    pcVar1 = this + 0x110;
    *(undefined4 *)(*(long *)(this + 0x108) + 0x60) = *(undefined4 *)(*(long *)pcVar1 + 0xb0);
    *(undefined4 *)(*(long *)(this + 0x108) + 100) = *(undefined4 *)(*(long *)pcVar1 + 0xb4);
    *(undefined4 *)(*(long *)(this + 0x108) + 0x68) = *(undefined4 *)(*(long *)pcVar1 + 0xb8);
    *(undefined4 *)(*(long *)(this + 0x108) + 0x6c) = *(undefined4 *)(*(long *)pcVar1 + 0xbc);
    *(undefined4 *)(*(long *)(this + 0x108) + 0x70) = *(undefined4 *)(*(long *)pcVar1 + 0xc0);
    *(undefined4 *)(*(long *)(this + 0x108) + 0x74) = *(undefined4 *)(*(long *)pcVar1 + 0xc4);
    *(undefined4 *)(*(long *)(this + 0x108) + 0x78) = *(undefined4 *)(*(long *)pcVar1 + 200);
    *(undefined4 *)(*(long *)(this + 0x108) + 0x7c) = *(undefined4 *)(*(long *)pcVar1 + 0xcc);
  }
  else {
    lVar4 = *(long *)(this + 0x110);
    uVar7 = *(uint *)(lVar4 + 0xa4) & 0xffff;
    uVar8 = *(uint *)(lVar4 + 0xa0) & 0xffff;
    uVar9 = *(uint *)(lVar4 + 0x9c) & 0xffff;
    uVar6 = *(uint *)(lVar4 + 0x98) & 0xffff;
    *(uint *)(*(long *)(this + 0x108) + 0x48) = uVar6;
    *(uint *)(*(long *)(this + 0x108) + 0x4c) = uVar9;
    *(uint *)(*(long *)(this + 0x108) + 0x50) = uVar8;
    *(uint *)(*(long *)(this + 0x108) + 0x54) = uVar7;
    *(uint *)(*(long *)(this + 0x108) + 0x98) = uVar6;
    *(uint *)(*(long *)(this + 0x108) + 0x9c) = uVar9;
    *(uint *)(*(long *)(this + 0x108) + 0xa0) = uVar8;
    *(uint *)(*(long *)(this + 0x108) + 0xa4) = uVar7;
  }
  cVar3 = TRotaryControl::RotaryValueChanged
                    ((TRotaryControl *)(this + 0x2c8),*(uint *)param_1 >> 0x18 & 3,&local_1c);
  if (cVar3 != '\0') {
    FWSettingsRotaryControl::UserRotatedHardwareKnob
              ((FWSettingsRotaryControl *)(this + 0x470),local_1c);
  }
  cVar3 = TRotaryControl::RotaryValueChanged
                    ((TRotaryControl *)(this + 0x2cc),*(uint *)param_1 >> 0x10 & 3,&local_1c);
  if (cVar3 != '\0') {
    local_28 = (OSArray *)0x0;
    FWSettingsRotaryControl::SetPairLevelsFromRotary
              ((FWSettingsRotaryControl *)(this + 0x470),"MAHeadphoneLevels",1,&local_28,local_1c);
    if (local_28 != (OSArray *)0x0) {
      m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
                (*(m_audio_b_FWPropertyDict **)(this + 0xf8),"MAHeadphoneLevels",
                 (OSObject *)local_28);
      (**(code **)(*(long *)local_28 + 0x28))();
    }
  }
  cVar3 = TRotaryControl::RotaryValueChanged
                    ((TRotaryControl *)(this + 0x2d0),*(uint *)param_1 >> 8 & 3,&local_1c);
  if (cVar3 != '\0') {
    local_30 = (OSArray *)0x0;
    FWSettingsRotaryControl::SetPairLevelsFromRotary
              ((FWSettingsRotaryControl *)(this + 0x470),"MAHeadphoneLevels",0,&local_30,local_1c);
    if (local_30 != (OSArray *)0x0) {
      m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
                (*(m_audio_b_FWPropertyDict **)(this + 0xf8),"MAHeadphoneLevels",
                 (OSObject *)local_30);
      (**(code **)(*(long *)local_30 + 0x28))();
    }
  }
  if (((byte)*param_1 & 3) != 0) {
    FWSettingsABSwitch::MomentarySwitchPressed((FWSettingsABSwitch *)(this + 0x2d8));
  }
  return;
}
