// m_audio_b_FWDCLOutputProgram::ProcessCompoundMIDIOutput
// Entry: 00026be6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h2)

/* m_audio_b_FWDCLOutputProgram::ProcessCompoundMIDIOutput(unsigned int) */

int __thiscall
m_audio_b_FWDCLOutputProgram::ProcessCompoundMIDIOutput
          (m_audio_b_FWDCLOutputProgram *this,uint param_1)

{
  char cVar1;
  int iVar2;
  long lVar3;
  long lVar4;
  ulong uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint local_54;
  DigiFWMIDIPacket *local_48;
  ulonglong local_40;
  undefined8 local_38;
  
  _clock_get_uptime(&local_38);
  _absolutetime_to_nanoseconds(local_38,&local_40);
  if (*(int *)(*(long *)(this + 0x80) + 0x60) == 0) {
    return 0;
  }
  local_54 = 0;
  iVar7 = 0;
  do {
    uVar5 = (ulong)local_54;
    uVar9 = local_54 + 1;
    uVar8 = param_1;
LAB_00026c37:
    if ((int)uVar8 < 1) goto LAB_00026cb6;
    cVar1 = m_audio_FWb::TMIDIPort::CheckForNewRealtimePacket
                      ((TMIDIPort *)(*(long *)(*(long *)(this + 0x80) + 0x68) + uVar5 * 0x28),
                       &local_48);
    if (cVar1 != '\0') {
      iVar2 = ScheduleMidiPacket(this,local_48,local_54,0,local_40);
      if (iVar2 == 0) {
        lVar3 = *(long *)(*(long *)(*(long *)(this + 0x80) + 0x68) + 0x18 + uVar5 * 0x28);
        goto LAB_00026c9c;
      }
      iVar7 = iVar7 + 1;
      uVar8 = uVar8 - 1;
      goto LAB_00026c37;
    }
    cVar1 = m_audio_FWb::TMIDIPort::CheckForNewTimestampedPacket
                      ((TMIDIPort *)(uVar5 * 0x28 + *(long *)(*(long *)(this + 0x80) + 0x68)),
                       &local_48,local_40,20000000);
    if (cVar1 == '\0') goto LAB_00026cb6;
    lVar3 = *(long *)local_48;
    if (lVar3 - *(long *)(this + uVar5 * 8 + 0x5d8) < 1000000) {
      *(long *)local_48 = *(long *)(this + uVar5 * 8 + 0x5d8) + 1000000;
      lVar3 = *(long *)local_48;
    }
    *(long *)(this + uVar5 * 8 + 0x5d8) = lVar3;
    uVar6 = *(uint *)(local_48 + 8);
    if ((int)uVar6 <= (int)uVar8) {
      uVar10 = ScheduleMidiPacket(this,local_48,local_54,1,local_40);
      uVar6 = *(uint *)(local_48 + 8);
      iVar7 = iVar7 + uVar6;
      if (uVar10 == uVar6) {
        uVar8 = uVar8 - uVar6;
        goto LAB_00026c37;
      }
      if (uVar10 == 0) goto LAB_00026df2;
      uVar6 = uVar6 - uVar10;
      _memcpy(local_48 + 0xc,local_48 + (ulong)uVar10 + 0xc,(ulong)uVar6);
LAB_00026de9:
      *(uint *)(local_48 + 8) = uVar6;
      goto LAB_00026df2;
    }
    *(uint *)(local_48 + 8) = uVar8;
    uVar8 = ScheduleMidiPacket(this,local_48,local_54,1,local_40);
    if (uVar8 == 0) goto LAB_00026de9;
    uVar10 = uVar6 - uVar8;
    iVar7 = iVar7 + uVar8;
    if (uVar6 != uVar8) {
      lVar3 = (ulong)(uVar10 - 1) + 1;
      if (uVar10 < 2) {
        lVar3 = 1;
      }
      lVar4 = 0;
      do {
        local_48[lVar4 + 0xc] = local_48[(ulong)(uVar8 + (int)lVar4) + 0xc];
        lVar4 = lVar4 + 1;
      } while (lVar3 != lVar4);
    }
    *(uint *)(local_48 + 8) = uVar10;
LAB_00026df2:
    lVar3 = *(long *)(*(long *)(*(long *)(this + 0x80) + 0x68) + 0x10 + uVar5 * 0x28);
LAB_00026c9c:
    iVar2 = *(int *)(lVar3 + 4) + -1;
    if (*(int *)(lVar3 + 4) == 0) {
      iVar2 = 0x7f;
    }
    *(int *)(lVar3 + 4) = iVar2;
LAB_00026cb6:
    local_54 = uVar9;
    if (*(uint *)(*(long *)(this + 0x80) + 0x60) <= uVar9) {
      return iVar7;
    }
  } while( true );
}
