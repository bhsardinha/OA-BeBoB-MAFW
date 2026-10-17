// m_audio_b_FWDCLOutputProgram::ScheduleMidiPacket
// Entry: 00026ab2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h2)

/* m_audio_b_FWDCLOutputProgram::ScheduleMidiPacket(DigiFWMIDIPacket&, unsigned int, unsigned int,
   unsigned long long) */

uint __thiscall
m_audio_b_FWDCLOutputProgram::ScheduleMidiPacket
          (m_audio_b_FWDCLOutputProgram *this,DigiFWMIDIPacket *param_1,uint param_2,uint param_3,
          ulonglong param_4)

{
  int iVar1;
  ulong uVar2;
  long lVar3;
  ulong uVar4;
  ulong uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  
  iVar1 = *(int *)(this + 100);
  uVar2 = ConvertTimestampNanosToAbsPacketIndex(this,*(ulonglong *)param_1,param_4);
  uVar5 = *(ulong *)(this + (ulong)param_3 * 8 + (ulong)param_2 * 0x10 + 0x558);
  if (*(ulong *)(this + (ulong)param_3 * 8 + (ulong)param_2 * 0x10 + 0x558) <= uVar2) {
    uVar5 = uVar2;
  }
  uVar2 = (ulong)(iVar1 + 2 + param_2 * (iVar1 + 1));
  uVar8 = 0;
  uVar6 = uVar8;
  uVar7 = uVar8;
  do {
    uVar7 = uVar7 + 1;
    uVar4 = uVar5;
    if (uVar5 % (ulong)(long)*(int *)(this + 0x34) < 4) {
      uVar4 = (uVar5 + 4) - uVar5 % (ulong)(long)*(int *)(this + 0x34);
    }
    lVar3 = m_audio_FWb::TPacketPool::GetIndexedPacket
                      ((TPacketPool *)(this + 0x18),
                       (uint)(uVar4 % (ulong)(long)*(int *)(this + 0x30)));
    if ((*(short *)(lVar3 + 6) == -1) || (*(int *)(lVar3 + uVar2 * 4) != 0x80)) {
LAB_00026bc4:
      uVar5 = uVar4 + 1;
    }
    else {
      uVar5 = uVar4 + 8;
      if (*(ulong *)(this + (ulong)param_3 * 8 + (ulong)param_2 * 0x10 + 0x558) <= uVar4) {
        *(ulong *)(this + (ulong)param_3 * 8 + (ulong)param_2 * 0x10 + 0x558) = uVar4;
        if (3 < *(uint *)(param_1 + 8)) {
          return uVar8;
        }
        *(uint *)(lVar3 + uVar2 * 4) = (uint)(byte)param_1[(ulong)uVar6 + 0xc] * 0x100 + 0x81;
        uVar8 = uVar8 + 1;
        uVar6 = uVar6 + 1;
        if (uVar6 == *(uint *)(param_1 + 8)) {
          return uVar8;
        }
        goto LAB_00026bc4;
      }
    }
    if (200 < (int)uVar7) {
      return uVar8;
    }
  } while( true );
}
