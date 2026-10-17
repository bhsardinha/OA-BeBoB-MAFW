// m_audio_b_FWDCLOutputProgram::ConvertTimestampNanosToAbsPacketIndex
// Entry: 00026a06
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h3)

/* m_audio_b_FWDCLOutputProgram::ConvertTimestampNanosToAbsPacketIndex(unsigned long long, unsigned
   long long) */

long __thiscall
m_audio_b_FWDCLOutputProgram::ConvertTimestampNanosToAbsPacketIndex
          (m_audio_b_FWDCLOutputProgram *this,ulonglong param_1,ulonglong param_2)

{
  long lVar1;
  uint uVar2;
  ulong uVar3;
  
  if ((param_1 == 0) || ((long)(param_1 - param_2) < 4000000)) {
    param_1 = param_2 + 4000000;
  }
  uVar3 = (ulong)*(uint *)(this + 0x550);
  lVar1 = *(long *)(this + uVar3 * 0x18 + 0x528);
  uVar2 = m_audio_b_FWTPacketIndex::GetIndexOfActivePacket
                    (*(m_audio_b_FWTPacketIndex **)(this + 0x20),
                     *(int *)(this + uVar3 * 0x18 + 0x520) + 1U & 3,1);
  return (long)*(int *)(this + 0x30) * *(long *)(this + uVar3 * 0x18 + 0x530) +
         (ulong)uVar2 + ((param_1 - lVar1) * 8000 + 12800000000) / 1000000000;
}
