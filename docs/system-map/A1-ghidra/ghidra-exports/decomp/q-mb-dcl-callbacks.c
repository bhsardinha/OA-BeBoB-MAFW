=== m_audio_FWb::TPacketPool::MarkGroupReadForInput @ 0000a738 ===  

/* m_audio_FWb::TPacketPool::MarkGroupReadForInput(unsigned int) */

void __thiscall m_audio_FWb::TPacketPool::MarkGroupReadForInput(TPacketPool *this,uint param_1)

{
  long lVar1;
  TPacketForwardIterator local_40 [48];
  
  TPacketForwardIterator::TPacketForwardIterator
            (local_40,this,param_1 * *(uint *)(this + 0x1c),*(uint *)(this + 0x1c));
  lVar1 = TPacketForwardIterator::NextPacket(local_40);
  if (lVar1 != 0) {
    do {
      *(byte *)(lVar1 + 0xb) = *(byte *)(lVar1 + 0xb) & 0x7f;
      lVar1 = TPacketForwardIterator::NextPacket(local_40);
    } while (lVar1 != 0);
  }
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketPool::GetIndexedActivePacket @ 0000a784 ===  

/* m_audio_FWb::TPacketPool::GetIndexedActivePacket(unsigned int, unsigned int) const */

void __thiscall
m_audio_FWb::TPacketPool::GetIndexedActivePacket(TPacketPool *this,uint param_1,uint param_2)

{
  uint uVar1;
  
  uVar1 = m_audio_b_FWTPacketIndex::GetIndexOfActivePacket
                    (*(m_audio_b_FWTPacketIndex **)(this + 8),param_1,param_2);
  GetIndexedPacket(this,uVar1);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketPool::GetPacketFrameContainingSampleIndex @ 0000a7aa ===  

/* m_audio_FWb::TPacketPool::GetPacketFrameContainingSampleIndex(unsigned int, unsigned int,
   unsigned int&, int**, unsigned int*) const */

int * __thiscall
m_audio_FWb::TPacketPool::GetPacketFrameContainingSampleIndex
          (TPacketPool *this,uint param_1,uint param_2,uint *param_3,int **param_4,uint *param_5)

{
  uint uVar1;
  int *piVar2;
  uint uVar3;
  
  uVar3 = param_1 % param_2;
  uVar1 = m_audio_b_FWTPacketIndex::GetIndexOfActivePacket
                    (*(m_audio_b_FWTPacketIndex **)(this + 8),param_1 >> 0xb,
                     param_1 / param_2 - (int)(0x800 / (ulong)param_2) * (param_1 >> 0xb));
  piVar2 = (int *)GetIndexedPacket(this,uVar1);
  if (param_4 != (int **)0x0) {
    *param_4 = piVar2;
  }
  if (param_5 != (uint *)0x0) {
    *param_5 = uVar3;
  }
  *param_3 = param_2 - uVar3;
  return piVar2 + ((*(int *)(this + 0x30) + *(int *)(this + 0x2c)) * uVar3 + 2);
}

 (GhidraScript)  
=== m_audio_FWb::TPacketPool::RawPacketIndexFromSampleFrame @ 0000a83e ===  

/* m_audio_FWb::TPacketPool::RawPacketIndexFromSampleFrame(unsigned int, unsigned int) */

void __thiscall
m_audio_FWb::TPacketPool::RawPacketIndexFromSampleFrame(TPacketPool *this,uint param_1,uint param_2)

{
  m_audio_b_FWTPacketIndex::GetIndexOfActivePacket
            (*(m_audio_b_FWTPacketIndex **)(this + 8),param_1 >> 0xb,
             param_1 / param_2 - (0x800 / param_2) * (param_1 >> 0xb));
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketPool::SyncCurrentInputPacketIndex @ 0000a86c ===  

/* m_audio_FWb::TPacketPool::SyncCurrentInputPacketIndex(unsigned int) */

undefined4 __thiscall
m_audio_FWb::TPacketPool::SyncCurrentInputPacketIndex(TPacketPool *this,uint param_1)

{
  long lVar1;
  TPacketReverseIterator local_50 [16];
  undefined4 local_40;
  
  TPacketReverseIterator::TPacketReverseIterator(local_50,this,param_1,*(uint *)(this + 0x18));
  while( true ) {
    lVar1 = TPacketReverseIterator::PreviousPacket(local_50);
    if (lVar1 == 0) {
      return 0;
    }
    if (*(int *)(lVar1 + 4) == 0) break;
    if (*(int *)(lVar1 + 8) < 0) {
      return local_40;
    }
  }
  return local_40;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketPool::InitGroupCIPsForOutput @ 0000a8b0 ===  

/* m_audio_FWb::TPacketPool::InitGroupCIPsForOutput(unsigned int, unsigned int, unsigned long long*)
    */

void __thiscall
m_audio_FWb::TPacketPool::InitGroupCIPsForOutput
          (TPacketPool *this,uint param_1,uint param_2,ulonglong *param_3)

{
  int iVar1;
  ulonglong *puVar2;
  uint uVar3;
  ulong uVar4;
  
  uVar4 = 0;
  if (param_1 < 4) {
    uVar4 = (ulong)param_1;
  }
  iVar1 = *(int *)(*(long *)(this + 8) + 0x18 + uVar4 * 4);
  if (iVar1 + param_2 <= *(uint *)(this + 0x1c)) {
    uVar3 = *(uint *)(this + 0x1c) * param_1;
    puVar2 = (ulonglong *)GetIndexedPacket(this,uVar3);
    *puVar2 = *param_3;
    if (param_2 != 0) {
      uVar3 = iVar1 + uVar3;
      uVar4 = (ulong)param_2;
      do {
        puVar2 = (ulonglong *)GetIndexedPacket(this,uVar3);
        *puVar2 = *param_3;
        uVar3 = uVar3 + 1;
        param_3 = param_3 + 1;
        uVar4 = uVar4 - 1;
      } while (uVar4 != 0);
    }
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWTPacketIndex::SetValidPackets @ 00010504 ===  

/* m_audio_b_FWTPacketIndex::SetValidPackets(unsigned int, unsigned int, unsigned int, bool*) */

void __thiscall
m_audio_b_FWTPacketIndex::SetValidPackets
          (m_audio_b_FWTPacketIndex *this,uint param_1,uint param_2,uint param_3,bool *param_4)

{
  long lVar1;
  uint uVar2;
  ulong uVar3;
  
  if ((param_3 <= *(uint *)(this + 0x2c)) && (param_1 < 4)) {
    *(uint *)(this + (ulong)param_1 * 4 + 0x18) = param_2;
    if (param_3 != 0) {
      lVar1 = *(long *)(this + (ulong)param_1 * 8 + 0x30);
      uVar2 = 0;
      uVar3 = 0;
      do {
        if (param_4[uVar3] == false) {
          *(undefined4 *)(lVar1 + uVar3 * 4) = 0;
        }
        else {
          if ((int)uVar3 == 0) {
            *(undefined4 *)(lVar1 + (ulong)uVar2 * 4) = 0;
          }
          else {
            *(uint *)(lVar1 + (ulong)uVar2 * 4) = param_2 + (int)uVar3;
          }
          uVar2 = uVar2 + 1;
        }
        uVar3 = uVar3 + 1;
      } while (param_3 != uVar3);
    }
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWTPacketIndex::GetIndexOfActivePacket @ 0001055e ===  

/* m_audio_b_FWTPacketIndex::GetIndexOfActivePacket(unsigned int, unsigned int) const */

int __thiscall
m_audio_b_FWTPacketIndex::GetIndexOfActivePacket
          (m_audio_b_FWTPacketIndex *this,uint param_1,uint param_2)

{
  if ((param_2 <= *(uint *)(this + 0x2c)) && (param_1 < 4)) {
    return *(uint *)(this + 0x2c) * param_1 +
           *(int *)(*(long *)(this + (ulong)param_1 * 8 + 0x30) + (ulong)param_2 * 4);
  }
  return 0;
}

 (GhidraScript)  
=== m_audio_b_FWTPacketIndex::init @ 000105f2 ===  

/* m_audio_b_FWTPacketIndex::init(unsigned int) */

undefined8 __thiscall m_audio_b_FWTPacketIndex::init(m_audio_b_FWTPacketIndex *this,uint param_1)

{
  uint uVar1;
  char cVar2;
  int iVar3;
  long *plVar4;
  void *pvVar5;
  
  cVar2 = (**(code **)(PTR_vtable_000300d8 + 0x98))();
  if (cVar2 != '\0') {
    *(uint *)(this + 0x28) = param_1;
    *(uint *)(this + 0x2c) = param_1 >> 2;
    *(undefined8 *)(this + 0x20) = 0;
    *(undefined8 *)(this + 0x18) = 0;
    *(undefined8 *)(this + 0x30) = 0;
    *(undefined8 *)(this + 0x38) = 0;
    *(undefined8 *)(this + 0x40) = 0;
    *(undefined8 *)(this + 0x48) = 0;
    plVar4 = (long *)IOBufferMemoryDescriptor::withOptions(0,(ulong)param_1 << 2,0x10);
    *(long **)(this + 0x10) = plVar4;
    if (plVar4 != (long *)0x0) {
      iVar3 = (**(code **)(*plVar4 + 0x1f0))(plVar4,0);
      if (iVar3 == 0) {
        pvVar5 = (void *)(**(code **)(**(long **)(this + 0x10) + 0x2e0))();
        if (pvVar5 != (void *)0x0) {
          *(void **)(this + 0x30) = pvVar5;
          uVar1 = *(uint *)(this + 0x2c);
          *(void **)(this + 0x38) = (void *)((long)pvVar5 + (ulong)uVar1 * 4);
          *(void **)(this + 0x40) = (void *)((long)pvVar5 + (ulong)(uVar1 * 2) * 4);
          *(void **)(this + 0x48) = (void *)((long)pvVar5 + (ulong)(uVar1 * 3) * 4);
          _bzero(pvVar5,(ulong)*(uint *)(this + 0x28) << 2);
          return 1;
        }
        (**(code **)(**(long **)(this + 0x10) + 0x1f8))(*(long **)(this + 0x10),0);
      }
      (**(code **)(**(long **)(this + 0x10) + 0x28))();
    }
    (**(code **)(PTR_vtable_000300d8 + 0xa0))(this);
  }
  return 0;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::SetPacketSize @ 00025e56 ===  

/* m_audio_b_FWDCLProgram::SetPacketSize(unsigned int, unsigned int, unsigned int, unsigned int) */

undefined1 __thiscall
m_audio_b_FWDCLProgram::SetPacketSize
          (m_audio_b_FWDCLProgram *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  TPacketPool *this_00;
  undefined8 *******pppppppuVar1;
  undefined8 *****pppppuVar2;
  char cVar3;
  int iVar4;
  undefined8 ****ppppuVar5;
  undefined8 *******pppppppuVar6;
  uint uVar7;
  uint uVar8;
  ulong uVar9;
  char local_170 [128];
  char local_f0 [128];
  TPacketForwardIterator local_70 [48];
  undefined8 ******local_40;
  undefined8 ******local_38;
  
  *(uint *)(this + 0x60) = param_1;
  *(uint *)(this + 100) = param_3;
  *(uint *)(this + 0x68) = param_4;
  if (this[0x8d] != (m_audio_b_FWDCLProgram)0x0) {
                    /* WARNING: Subroutine does not return */
    _panic(
          "\"IsochStreamRunning during SetPacketSize\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWDCLProgram.cpp:1727"
          );
  }
  if (*(m_audio_b_FWIsochChannel **)(this + 0x58) != (m_audio_b_FWIsochChannel *)0x0) {
    m_audio_b_FWIsochChannel::ReleaseTalker(*(m_audio_b_FWIsochChannel **)(this + 0x58));
  }
  iVar4 = RebuildDCLProgram(this);
  if (iVar4 == 0) {
    this_00 = (TPacketPool *)(this + 0x18);
    m_audio_FWb::TPacketPool::Invalidate(this_00);
    cVar3 = m_audio_FWb::TPacketPool::Initialize
                      (this_00,*(IOFireWireDevice **)(this + 0x78),param_2,param_1,
                       *(uint *)(this + 100),*(uint *)(this + 0x68),(m_audio_b_FWTPacketIndex *)0x0)
    ;
    if (cVar3 != '\0') {
      this[0x51] = (m_audio_b_FWDCLProgram)0x1;
      if (*(long *)(this + 0x10) == 0) {
                    /* WARNING: Subroutine does not return */
        _panic(
              "\"DCL program is NULL\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWDCLBuilder.h:85"
              );
      }
      pppppppuVar1 = *(undefined8 ********)(*(long *)(this + 0x10) + 0x10);
      local_40 = pppppppuVar1;
      local_38 = pppppppuVar1;
      m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator(local_70,this_00,1,0xffffffff);
      do {
        pppppppuVar6 = pppppppuVar1;
        if (pppppppuVar6 == (undefined8 *******)0x0) {
          return 1;
        }
        pppppppuVar1 = (undefined8 *******)*pppppppuVar6;
      } while (*(int *)(pppppppuVar6 + 2) != 0xc);
      local_40 = *pppppppuVar6;
      if (pppppppuVar6 != (undefined8 *******)0x0) {
        uVar8 = 0;
        do {
          if (*(int *)(pppppppuVar6 + 4) != 0) {
            uVar9 = 0;
            do {
              pppppuVar2 = pppppppuVar6[3][uVar9];
              iVar4 = *(int *)(pppppuVar2 + 2);
              uVar7 = (int)uVar9 + 1;
              if ((iVar4 == 1) || (iVar4 == 5)) {
                if (pppppuVar2 == (undefined8 *****)0x0) {
                  _snprintf(local_f0,0x80,"dclPacket == NULL (%u)",uVar9);
                    /* WARNING: Subroutine does not return */
                  _panic(
                        "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWDCLProgram.cpp:1790"
                        );
                }
                ppppuVar5 = (undefined8 ****)
                            m_audio_FWb::TPacketForwardIterator::NextPacket(local_70);
                pppppuVar2[3] = ppppuVar5;
                *(uint *)(pppppuVar2 + 4) = param_1;
                if (ppppuVar5 == (undefined8 ****)0x0) {
                  _snprintf(local_170,0x80,"(dclPacket->buffer == NULL) at index %u",uVar9);
                    /* WARNING: Subroutine does not return */
                  _panic(
                        "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWDCLProgram.cpp:1801"
                        );
                }
                CalcIsochPacketHeaders(this,(uint *)ppppuVar5,0,0,false);
              }
              else if (iVar4 == 0xe) {
                pppppuVar2[3] =
                     (undefined8 ****)
                     ((ulong)(*(int *)(this + 0x3c) * 0x1000 - 0x1000) + *(long *)(this + 0x28) +
                     (ulong)uVar8 * 4);
                uVar8 = uVar8 + 1;
              }
              uVar9 = (ulong)uVar7;
            } while (uVar7 < *(uint *)(pppppppuVar6 + 4));
          }
          pppppppuVar6 = &local_40;
          do {
            pppppppuVar6 = (undefined8 *******)*pppppppuVar6;
            if (pppppppuVar6 == (undefined8 *******)0x0) {
              return 1;
            }
          } while (*(int *)(pppppppuVar6 + 2) != 0xc);
          local_40 = *pppppppuVar6;
        } while (pppppppuVar6 != (undefined8 *******)0x0);
      }
      return 1;
    }
  }
  return 0;
}

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::SetPacketSize @ 000260a8 ===  

/* m_audio_b_FWDCLInputProgram::SetPacketSize(unsigned int, unsigned int) */

undefined1 __thiscall
m_audio_b_FWDCLInputProgram::SetPacketSize
          (m_audio_b_FWDCLInputProgram *this,uint param_1,uint param_2)

{
  undefined1 uVar1;
  
  uVar1 = m_audio_b_FWDCLProgram::SetPacketSize
                    ((m_audio_b_FWDCLProgram *)this,
                     (param_2 + param_1) * *(int *)(this + 0x4ac) * 4 + 0xc,0x100,param_1,param_2);
  return uVar1;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::SetPacketSize @ 000260d2 ===  

/* m_audio_b_FWDCLOutputProgram::SetPacketSize(unsigned int, unsigned int) */

undefined1 __thiscall
m_audio_b_FWDCLOutputProgram::SetPacketSize
          (m_audio_b_FWDCLOutputProgram *this,uint param_1,uint param_2)

{
  undefined1 uVar1;
  
  uVar1 = m_audio_b_FWDCLProgram::SetPacketSize
                    ((m_audio_b_FWDCLProgram *)this,
                     (param_2 + param_1) * *(int *)(this + 0x4ac) * 4 + 8,
                     *(int *)(this + 0x4e0) << 2,param_1,param_2);
  return uVar1;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::GetPacketFrameContainingSampleIndex @ 00026100 ===  

/* m_audio_b_FWDCLOutputProgram::GetPacketFrameContainingSampleIndex(unsigned int, unsigned int,
   unsigned int&, int**, unsigned int*) const */

void __thiscall
m_audio_b_FWDCLOutputProgram::GetPacketFrameContainingSampleIndex
          (m_audio_b_FWDCLOutputProgram *this,uint param_1,uint param_2,uint *param_3,int **param_4,
          uint *param_5)

{
  m_audio_FWb::TPacketPool::GetPacketFrameContainingSampleIndex
            ((TPacketPool *)(this + 0x18),param_1,param_2,param_3,param_4,param_5);
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::ResyncInputBuffer @ 0002610e ===  

/* m_audio_b_FWDCLInputProgram::ResyncInputBuffer(unsigned int) */

undefined1 __thiscall
m_audio_b_FWDCLInputProgram::ResyncInputBuffer(m_audio_b_FWDCLInputProgram *this,uint param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  
  if (this[0x8e] != (m_audio_b_FWDCLInputProgram)0x0) {
    uVar1 = param_1 >> 0xc & 0x1fff;
    uVar3 = *(uint *)(*(long *)(this + 0x80) + 0xc) >> 0xc & 0x1fff;
    iVar5 = uVar1 - uVar3;
    if (uVar3 < uVar1) {
      iVar2 = iVar5 + -8000;
      iVar4 = iVar5;
    }
    else {
      iVar2 = iVar5 + 8000;
      iVar4 = -iVar5;
    }
    if (iVar4 < 0x7d1) {
      iVar2 = iVar5;
    }
    if (0 < iVar2) {
      uVar1 = _SubtractFWCycleTimeFromFWCycleTime
                        (param_1 & 0x1ffffff,*(uint *)(*(long *)(this + 0x80) + 0xc) & 0x1ffffff);
      uVar1 = (int)(*(int *)(this + 0x4a8) / 2 +
                   ((uVar1 >> 0xc & 0x1fff) * 0xc00 + (uVar1 >> 0x19) * 0x1770000 + (uVar1 & 0xfff))
                   * *(int *)(this + 0x4ac)) / *(int *)(this + 0x4a8) & 0x1fff;
      uVar3 = (*(int *)(this + 0x68) + *(int *)(this + 100)) * 4;
      if (*(uint *)(*(long *)(this + 0x80) + 0x40) / uVar3 == uVar1) {
        return 1;
      }
      *(uint *)(*(long *)(this + 0x80) + 0x40) = uVar3 * uVar1;
      return 1;
    }
  }
  return 0;
}

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::CopySamplesFromPacket @ 00026208 ===  

/* m_audio_b_FWDCLInputProgram::CopySamplesFromPacket(unsigned int, unsigned int*,
   m_audio_FWb::TRingBuffer&) */

void __thiscall
m_audio_b_FWDCLInputProgram::CopySamplesFromPacket
          (m_audio_b_FWDCLInputProgram *this,uint param_1,uint *param_2,TRingBuffer *param_3)

{
  uint uVar1;
  TRingBuffer TVar2;
  uint uVar3;
  
  uVar1 = *(uint *)(param_3 + 0x18);
  TVar2 = (TRingBuffer)
          m_audio_FWb::TRingBuffer::AdvanceIndex
                    (param_3,param_1 * 4,(uint *)(param_3 + 0x18),(uint *)(param_3 + 0x20));
  param_3[0x25] = TVar2;
  if ((*(uint *)(param_3 + 0x18) != 0) && (TVar2 != (TRingBuffer)0x0)) {
    uVar3 = *(uint *)(param_3 + 0x18) >> 2;
    param_1 = param_1 - uVar3;
    _memcpy(*(void **)(param_3 + 8),param_2 + (param_1 + 3),(ulong)uVar3 << 2);
  }
  _memcpy((void *)((ulong)uVar1 + *(long *)(param_3 + 8)),param_2 + 3,(ulong)param_1 << 2);
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::CycleTimeToHostNanos @ 00026284 ===  

/* m_audio_b_FWDCLProgram::CycleTimeToHostNanos(unsigned int) */

long __thiscall
m_audio_b_FWDCLProgram::CycleTimeToHostNanos(m_audio_b_FWDCLProgram *this,uint param_1)

{
  uint uVar1;
  int iVar2;
  long *plVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint local_54;
  long local_50;
  long local_48;
  undefined8 local_40;
  undefined8 local_38;
  
  local_54 = 0;
  do {
    _clock_get_uptime(&local_38);
    plVar3 = (long *)IOFireWireNub::getBus();
    (**(code **)(*plVar3 + 0x860))(plVar3,&local_54);
    _clock_get_uptime(&local_40);
    _absolutetime_to_nanoseconds(local_38,&local_48);
    _absolutetime_to_nanoseconds(local_40,&local_50);
  } while (149999 < (ulong)(local_50 - local_48));
  uVar5 = param_1 >> 0xc & 0x1fff;
  uVar1 = local_54 >> 0xc & 0x1fff;
  iVar4 = uVar5 - uVar1;
  if (uVar1 < uVar5) {
    iVar2 = iVar4 + -8000;
    iVar6 = iVar4;
  }
  else {
    iVar2 = iVar4 + 8000;
    iVar6 = -iVar4;
  }
  if (iVar6 < 0x7d1) {
    iVar2 = iVar4;
  }
  return (long)(iVar2 * 0x1e848) + local_48 + ((ulong)(local_50 - local_48) >> 1);
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::AdjustOutputDCL @ 0002636e ===  

/* m_audio_b_FWDCLOutputProgram::AdjustOutputDCL(unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWDCLOutputProgram::AdjustOutputDCL
          (m_audio_b_FWDCLOutputProgram *this,uint param_1,uint param_2)

{
  uint uVar1;
  
  if (param_2 < 5) {
    if (param_1 == 1) {
      param_2 = param_2 + 5;
      uVar1 = 0;
    }
    else if (param_1 == 2) {
      param_2 = param_2 + 10;
      uVar1 = 1;
    }
    else if (param_1 == 3) {
      param_2 = param_2 + 0xf;
      uVar1 = 2;
    }
    else {
      uVar1 = 3;
    }
    m_audio_b_FWDCLBuilder::ModifyJumpLabel
              (*(m_audio_b_FWDCLBuilder **)(this + 0x10),
               *(IOFWLocalIsochPort **)(*(long *)(this + 0x58) + 0x28),uVar1,param_2);
    return;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::OutputDCLCallback @ 000263c2 ===  

/* m_audio_b_FWDCLOutputProgram::OutputDCLCallback(long, bool) */

void __thiscall
m_audio_b_FWDCLOutputProgram::OutputDCLCallback
          (m_audio_b_FWDCLOutputProgram *this,long param_1,bool param_2)

{
  ulonglong *puVar1;
  m_audio_b_FWDCLOutputProgram mVar2;
  undefined4 uVar3;
  long *plVar4;
  code *pcVar5;
  m_audio_b_FWDCLProgram *this_00;
  m_audio_b_FWDCLOutputProgram *this_01;
  m_audio_b_FWTPacketIndex *pmVar6;
  ulonglong uVar7;
  bool *pbVar8;
  ushort uVar9;
  int iVar10;
  undefined4 uVar11;
  undefined8 uVar12;
  uint uVar13;
  ulong uVar14;
  bool *pbVar15;
  bool bVar16;
  uint uVar17;
  undefined7 in_register_00000011;
  int iVar18;
  long lVar19;
  int iVar20;
  int iVar21;
  uint uVar22;
  bool abStack_80 [8];
  ulonglong local_78;
  ulonglong *local_70;
  ulonglong *local_68;
  bool *local_60;
  int local_54;
  long local_50;
  uint local_44;
  char local_3d;
  uint local_3c;
  undefined1 local_38 [8];
  
  local_3c = (uint)CONCAT71(in_register_00000011,param_2);
  if (this[0x8e] == (m_audio_b_FWDCLOutputProgram)0x0) {
    return;
  }
  if (3 < (ulong)param_1) {
    return;
  }
  local_54 = *(int *)(this + 0x4ac);
  uVar14 = (ulong)(*(uint *)(this + 0x4e0) * 8) + 0xf & 0xfffffffffffffff0;
  local_70 = (ulonglong *)((long)&local_78 - uVar14);
  local_68 = (ulonglong *)((long)local_70 - uVar14);
  pbVar15 = (bool *)((long)local_68 - ((ulong)*(uint *)(this + 0x4e0) + 0xf & 0xfffffffffffffff0));
  uVar22 = *(uint *)((ulong)(*(int *)(this + 0x3c) * 0x1000 - 0x1000) + *(long *)(this + 0x28) +
                    param_1 * 4);
  local_78 = param_1;
  local_60 = pbVar15;
  if (!param_2) {
    if (param_1 == 3) {
      *(long *)(this + 0x518) = *(long *)(this + 0x518) + 1;
    }
    pbVar15[-8] = true;
    pbVar15[-7] = true;
    pbVar15[-6] = true;
    pbVar15[-5] = false;
    pbVar15[-4] = false;
    pbVar15[-3] = false;
    pbVar15[-2] = false;
    pbVar15[-1] = false;
    uVar12 = m_audio_b_FWDCLProgram::CycleTimeToHostNanos((m_audio_b_FWDCLProgram *)this,uVar22);
    uVar13 = *(uint *)(this + 0x550) & 1 ^ 1;
    uVar14 = (ulong)uVar13;
    *(int *)(this + uVar14 * 0x18 + 0x520) = (int)local_78;
    *(undefined8 *)(this + uVar14 * 0x18 + 0x528) = uVar12;
    *(undefined8 *)(this + uVar14 * 0x18 + 0x530) = *(undefined8 *)(this + 0x518);
    *(uint *)(this + 0x550) = uVar13;
    *(int *)(*(long *)(this + 0x80) + 0x14) = (int)local_78;
    if (local_78 == 3) {
      if (*(ulong *)(this + 0x518) < 3) {
        if (*(ulong *)(this + 0x518) == 2) {
          *(uint *)(*(long *)(this + 0x80) + 0xc) = uVar22;
          *(undefined1 *)(*(long *)(this + 0x80) + 0x11) = 1;
        }
      }
      else {
        pbVar15[-8] = true;
        pbVar15[-7] = true;
        pbVar15[-6] = true;
        pbVar15[-5] = false;
        pbVar15[-4] = false;
        pbVar15[-3] = false;
        pbVar15[-2] = false;
        pbVar15[-1] = false;
        _nanoseconds_to_absolutetime(uVar12,local_38);
        plVar4 = *(long **)(*(long *)(this + 0x80) + 0x78);
        pcVar5 = *(code **)(*plVar4 + 0xae0);
        pbVar15[-8] = true;
        pbVar15[-7] = true;
        pbVar15[-6] = true;
        pbVar15[-5] = false;
        pbVar15[-4] = false;
        pbVar15[-3] = false;
        pbVar15[-2] = false;
        pbVar15[-1] = false;
        (*pcVar5)(plVar4,1,local_38);
      }
    }
  }
  if (this[0x8d] == (m_audio_b_FWDCLOutputProgram)0x0 && (char)local_3c == '\0') {
    return;
  }
  lVar19 = *(long *)(this + 0x80);
  local_50 = *(long *)(lVar19 + 0x18);
  iVar10 = *(int *)(lVar19 + 0x20);
  uVar13 = *(uint *)(lVar19 + 0x24);
  if ((char)local_3c == '\0') {
    iVar20 = (uVar22 >> 0xc & 0x1fff) + *(int *)(this + 0x4cc);
    iVar18 = (*(uint *)(this + 0x4d4) >> 0xc & 0x1fff) - iVar20;
    iVar21 = iVar18 + 8000;
    if (-0xfa1 < iVar18) {
      iVar21 = iVar18;
    }
    if (2 < iVar21 - 3U) {
      *(uint *)(this + 0x4d4) =
           (*(uint *)(this + 0x4d4) & 0xfff) +
           (iVar20 * 0x1000 + 0x4000U & 0x1fff000) + (uVar22 & 0xfe000000);
    }
  }
  if (this[0x618] == (m_audio_b_FWDCLOutputProgram)0x0) {
    iVar21 = *(int *)(this + 0x4e0);
  }
  else {
    uVar17 = iVar10 - uVar13;
    iVar21 = *(int *)(this + 0x4e0);
    if (iVar21 + 4U <= uVar17) {
      if ((uint)(iVar21 * 3) < uVar17) {
        uVar13 = (iVar10 + -4) - iVar21;
      }
      iVar10 = *(int *)(this + 0x4cc) + (uVar22 >> 0xc);
      uVar22 = (uint)*(ushort *)(local_50 + (ulong)(uVar13 & 0x7ff) * 2);
      if ((uVar22 != 0xffff) && (0x2000 < (uVar22 + iVar10 * -0x1000 & 0xffff) - 0x1000)) {
        do {
          do {
            uVar13 = uVar13 + 1;
            uVar22 = (uint)*(ushort *)(local_50 + (ulong)(uVar13 & 0x7ff) * 2);
          } while (uVar22 == 0xffff);
        } while (0x1000 < (uVar22 + iVar10 * -0x1000 & 0xf000) - 0x1000);
      }
      local_3d = '\x01';
      goto LAB_000265e5;
    }
  }
  local_3d = '\0';
LAB_000265e5:
  if (iVar21 == 0) {
    local_44 = 0;
  }
  else {
    local_3c = 0;
    uVar22 = 0;
    do {
      uVar9 = *(ushort *)(local_50 + (ulong)(uVar13 & 0x7ff) * 2);
      local_44 = uVar22 + 1;
      if (local_3d == '\0') {
        uVar17 = *(int *)(this + 0x4c0) + *(int *)(this + 0x4bc);
        *(uint *)(this + 0x4bc) = uVar17;
        if (uVar17 <= *(uint *)(this + 0x4c4)) goto LAB_00026725;
        *(uint *)(this + 0x4bc) = uVar17 - *(uint *)(this + 0x4c4);
        bVar16 = false;
      }
      else if (uVar9 == 0xffff) {
        uVar13 = uVar13 + 1;
        bVar16 = false;
      }
      else {
        uVar17 = (uint)uVar9;
        if ((uint)uVar9 < *(uint *)(this + 0x4d8)) {
          pbVar15[-8] = true;
          pbVar15[-7] = true;
          pbVar15[-6] = true;
          pbVar15[-5] = false;
          pbVar15[-4] = false;
          pbVar15[-3] = false;
          pbVar15[-2] = false;
          pbVar15[-1] = false;
          uVar11 = _SubtractFWCycleTimeFromFWCycleTime(0xfc00);
          pbVar15[-8] = true;
          pbVar15[-7] = true;
          pbVar15[-6] = true;
          pbVar15[-5] = false;
          pbVar15[-4] = false;
          pbVar15[-3] = false;
          pbVar15[-2] = false;
          pbVar15[-1] = false;
          uVar11 = _AddFWCycleTimeToFWCycleTime(uVar11,uVar17);
        }
        else {
          pbVar15[-8] = true;
          pbVar15[-7] = true;
          pbVar15[-6] = true;
          pbVar15[-5] = false;
          pbVar15[-4] = false;
          pbVar15[-3] = false;
          pbVar15[-2] = false;
          pbVar15[-1] = false;
          uVar11 = _SubtractFWCycleTimeFromFWCycleTime(uVar17);
        }
        uVar3 = *(undefined4 *)(this + 0x4d4);
        pbVar15[-8] = true;
        pbVar15[-7] = true;
        pbVar15[-6] = true;
        pbVar15[-5] = false;
        pbVar15[-4] = false;
        pbVar15[-3] = false;
        pbVar15[-2] = false;
        pbVar15[-1] = false;
        uVar11 = _AddFWCycleTimeToFWCycleTime(uVar3,uVar11);
        *(undefined4 *)(this + 0x4d4) = uVar11;
        *(uint *)(this + 0x4d8) = uVar17;
        uVar13 = uVar13 + 1;
LAB_00026725:
        if (local_3d == '\0') {
          uVar17 = *(int *)(this + 0x4b0) + *(int *)(this + 0x4b8);
          *(uint *)(this + 0x4b8) = uVar17;
          if (*(uint *)(this + 0x4b4) < uVar17) {
            *(uint *)(this + 0x4b8) = uVar17 - *(uint *)(this + 0x4b4);
            uVar17 = *(uint *)(this + 0x4a8);
          }
          else {
            uVar17 = *(int *)(this + 0x4a8) + 1;
          }
          uVar11 = *(undefined4 *)(this + 0x4d4);
          pbVar15[-8] = true;
          pbVar15[-7] = true;
          pbVar15[-6] = true;
          pbVar15[-5] = false;
          pbVar15[-4] = false;
          pbVar15[-3] = false;
          pbVar15[-2] = false;
          pbVar15[-1] = false;
          uVar11 = _AddFWCycleTimeToFWCycleTime(uVar11,uVar17 >> 1);
          *(undefined4 *)(this + 0x4d4) = uVar11;
          pbVar15[-8] = true;
          pbVar15[-7] = true;
          pbVar15[-6] = true;
          pbVar15[-5] = false;
          pbVar15[-4] = false;
          pbVar15[-3] = false;
          pbVar15[-2] = false;
          pbVar15[-1] = false;
          uVar11 = _AddFWCycleTimeToFWCycleTime(uVar11,uVar17 >> 1);
          *(undefined4 *)(this + 0x4d4) = uVar11;
          pbVar15[-8] = true;
          pbVar15[-7] = true;
          pbVar15[-6] = true;
          pbVar15[-5] = false;
          pbVar15[-4] = false;
          pbVar15[-3] = false;
          pbVar15[-2] = false;
          pbVar15[-1] = false;
          uVar11 = _AddFWCycleTimeToFWCycleTime(uVar11,uVar17 & 1);
          *(undefined4 *)(this + 0x4d4) = uVar11;
        }
        else {
          uVar11 = *(undefined4 *)(this + 0x4d4);
        }
        uVar9 = (ushort)uVar11;
        local_3c = local_3c + local_54;
        bVar16 = true;
      }
      local_60[uVar22] = bVar16;
      puVar1 = local_70 + uVar22;
      mVar2 = this[0x4d0];
      pbVar15[-8] = true;
      pbVar15[-7] = true;
      pbVar15[-6] = true;
      pbVar15[-5] = false;
      pbVar15[-4] = false;
      pbVar15[-3] = false;
      pbVar15[-2] = false;
      pbVar15[-1] = false;
      m_audio_b_FWDCLProgram::CalcIsochPacketHeaders
                ((m_audio_b_FWDCLProgram *)this,(uint *)puVar1,(uint)uVar9,(uint)(byte)mVar2,bVar16)
      ;
      this_00 = *(m_audio_b_FWDCLProgram **)(this + 0x90);
      if (this_00 != (m_audio_b_FWDCLProgram *)0x0) {
        puVar1 = local_68 + uVar22;
        mVar2 = this[0x4d0];
        pbVar15[-8] = true;
        pbVar15[-7] = true;
        pbVar15[-6] = true;
        pbVar15[-5] = false;
        pbVar15[-4] = false;
        pbVar15[-3] = false;
        pbVar15[-2] = false;
        pbVar15[-1] = false;
        m_audio_b_FWDCLProgram::CalcIsochPacketHeaders
                  (this_00,(uint *)puVar1,(uint)uVar9,(uint)(byte)mVar2,bVar16);
      }
      this[0x4d0] = (m_audio_b_FWDCLOutputProgram)((char)this[0x4d0] + (char)local_54);
    } while ((local_44 < *(uint *)(this + 0x4e0)) && (uVar22 = local_44, local_3c < 0x800));
    lVar19 = *(long *)(this + 0x80);
  }
  uVar7 = local_78;
  *(uint *)(lVar19 + 0x24) = uVar13;
  uVar22 = *(int *)(this + 0x4e0) - local_44;
  pbVar15[-8] = true;
  pbVar15[-7] = true;
  pbVar15[-6] = true;
  pbVar15[-5] = false;
  pbVar15[-4] = false;
  pbVar15[-3] = false;
  pbVar15[-2] = false;
  pbVar15[-1] = false;
  AdjustOutputDCL(this,(uint)uVar7,uVar22);
  uVar7 = local_78;
  this_01 = *(m_audio_b_FWDCLOutputProgram **)(this + 0x90);
  if (this_01 != (m_audio_b_FWDCLOutputProgram *)0x0) {
    pbVar15[-8] = true;
    pbVar15[-7] = true;
    pbVar15[-6] = true;
    pbVar15[-5] = false;
    pbVar15[-4] = false;
    pbVar15[-3] = false;
    pbVar15[-2] = false;
    pbVar15[-1] = false;
    AdjustOutputDCL(this_01,(uint)uVar7,uVar22);
  }
  uVar17 = (uint)local_78;
  pbVar15[-8] = true;
  pbVar15[-7] = true;
  pbVar15[-6] = true;
  pbVar15[-5] = false;
  pbVar15[-4] = false;
  pbVar15[-3] = false;
  pbVar15[-2] = false;
  pbVar15[-1] = false;
  m_audio_FWb::TPacketPool::ZeroElements((TPacketPool *)(this + 0x18),uVar17);
  uVar13 = local_44;
  pbVar8 = local_60;
  pmVar6 = *(m_audio_b_FWTPacketIndex **)(this + 0x20);
  pbVar15[-8] = true;
  pbVar15[-7] = true;
  pbVar15[-6] = true;
  pbVar15[-5] = false;
  pbVar15[-4] = false;
  pbVar15[-3] = false;
  pbVar15[-2] = false;
  pbVar15[-1] = false;
  m_audio_b_FWTPacketIndex::SetValidPackets(pmVar6,uVar17,uVar22,uVar13,pbVar8);
  puVar1 = local_70;
  pbVar15[-8] = true;
  pbVar15[-7] = true;
  pbVar15[-6] = true;
  pbVar15[-5] = false;
  pbVar15[-4] = false;
  pbVar15[-3] = false;
  pbVar15[-2] = false;
  pbVar15[-1] = false;
  m_audio_FWb::TPacketPool::InitGroupCIPsForOutput
            ((TPacketPool *)(this + 0x18),uVar17,uVar13,puVar1);
  lVar19 = *(long *)(this + 0x90);
  if (lVar19 != 0) {
    pbVar15[-8] = true;
    pbVar15[-7] = true;
    pbVar15[-6] = true;
    pbVar15[-5] = false;
    pbVar15[-4] = false;
    pbVar15[-3] = false;
    pbVar15[-2] = false;
    pbVar15[-1] = false;
    m_audio_FWb::TPacketPool::ZeroElements((TPacketPool *)(lVar19 + 0x18),uVar17);
    pbVar8 = local_60;
    pmVar6 = *(m_audio_b_FWTPacketIndex **)(lVar19 + 0x20);
    pbVar15[-8] = true;
    pbVar15[-7] = true;
    pbVar15[-6] = true;
    pbVar15[-5] = false;
    pbVar15[-4] = false;
    pbVar15[-3] = false;
    pbVar15[-2] = false;
    pbVar15[-1] = false;
    m_audio_b_FWTPacketIndex::SetValidPackets(pmVar6,uVar17,uVar22,uVar13,pbVar8);
    puVar1 = local_68;
    lVar19 = *(long *)(this + 0x90);
    pbVar15[-8] = true;
    pbVar15[-7] = true;
    pbVar15[-6] = true;
    pbVar15[-5] = false;
    pbVar15[-4] = false;
    pbVar15[-3] = false;
    pbVar15[-2] = false;
    pbVar15[-1] = false;
    m_audio_FWb::TPacketPool::InitGroupCIPsForOutput
              ((TPacketPool *)(lVar19 + 0x18),uVar17,uVar13,puVar1);
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::Start @ 00026900 ===  

/* m_audio_b_FWDCLOutputProgram::Start() */

void __thiscall m_audio_b_FWDCLOutputProgram::Start(m_audio_b_FWDCLOutputProgram *this)

{
  uint uVar1;
  int iVar2;
  long lVar3;
  
  *(undefined4 *)(this + 0x520) = 3;
  *(undefined8 *)(this + 0x530) = 0;
  *(undefined8 *)(this + 0x528) = *(undefined8 *)(*(long *)(this + 0x80) + 0x80);
  *(undefined4 *)(this + 0x538) = 0;
  *(undefined8 *)(this + 0x548) = 0;
  *(undefined8 *)(this + 0x540) = 0;
  *(undefined4 *)(this + 0x550) = 0;
  *(undefined4 *)(*(long *)(this + 0x80) + 0x14) = 0;
  *(undefined8 *)(this + 0x518) = 0;
  *(undefined8 *)(this + 0x508) = 0;
  *(undefined4 *)(this + 0x510) = 0;
  if (*(int *)(*(long *)(this + 0x80) + 0x60) != 0) {
    uVar1 = 0;
    do {
      lVar3 = (ulong)uVar1 * 0x10;
      *(undefined8 *)(this + lVar3 + 0x558) = 0;
      *(undefined8 *)(this + lVar3 + 0x560) = 0;
      *(undefined8 *)(this + (ulong)uVar1 * 8 + 0x5d8) = 0;
      uVar1 = uVar1 + 1;
    } while (uVar1 < *(uint *)(*(long *)(this + 0x80) + 0x60));
  }
  lVar3 = 0;
  do {
    OutputDCLCallback(this,lVar3,true);
    lVar3 = lVar3 + 1;
  } while (lVar3 != 4);
  iVar2 = m_audio_b_FWDCLProgram::Start((m_audio_b_FWDCLProgram *)this);
  if (iVar2 == 0) {
    **(int **)(this + 0x80) = **(int **)(this + 0x80) + 1;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::ConvertTimestampNanosToAbsPacketIndex @ 00026a06 ===  

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

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::ScheduleMidiPacket @ 00026ab2 ===  

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

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::ProcessCompoundMIDIOutput @ 00026be6 ===  

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

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::InputDCLCallbackCopyPacketGroupToSamples @ 00026e3e ===  

/* m_audio_b_FWDCLInputProgram::InputDCLCallbackCopyPacketGroupToSamples(unsigned int) */

void __thiscall
m_audio_b_FWDCLInputProgram::InputDCLCallbackCopyPacketGroupToSamples
          (m_audio_b_FWDCLInputProgram *this,uint param_1)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  long lVar5;
  long lVar6;
  m_audio_b_FWDCLInputProgram mVar7;
  char cVar8;
  uint *puVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  m_audio_b_FWDCLInputProgram mVar13;
  int iVar14;
  bool bVar15;
  uint local_64;
  long local_40;
  uint local_38;
  uint local_34;
  
  local_40 = *(long *)(this + 0x80);
  uVar10 = *(int *)(local_40 + 0x88) + 1U & 0x3f;
  local_64 = param_1;
  if (uVar10 != param_1) {
    uVar11 = (param_1 - *(int *)(local_40 + 0x88)) + 0x3f;
    uVar12 = uVar11 & 0x3f;
    if (0x20 < uVar12) {
      uVar12 = uVar11 | 0xffffffc0;
    }
    if ((int)uVar12 < 0) {
      return;
    }
    param_1 = uVar12 + uVar10;
    local_64 = uVar10;
    if (param_1 < uVar10) {
      return;
    }
  }
  local_38 = local_64 * 4;
  while( true ) {
    iVar2 = *(int *)(this + 0x68);
    iVar3 = *(int *)(this + 100);
    iVar4 = *(int *)(this + 0x4ac);
    lVar5 = *(long *)(local_40 + 0x18);
    local_34 = *(uint *)(local_40 + 0x20);
    bVar15 = *(char *)(local_40 + 0x58) != '\0';
    uVar10 = _SubtractFWCycleTimeFromFWCycleTime
                       (*(undefined4 *)
                         ((ulong)(*(int *)(this + 0x3c) * 0x1000 - 0x1000) + *(long *)(this + 0x28)
                         + (ulong)(local_64 & 0x3f) * 4),0x3000);
    if (this[0x8e] != (m_audio_b_FWDCLInputProgram)0x0) {
      *(uint *)(*(long *)(this + 0x80) + 0x88) = local_64 & 0x3f;
    }
    iVar14 = 0;
    do {
      puVar9 = (uint *)m_audio_FWb::TPacketPool::GetIndexedPacket
                                 ((TPacketPool *)(this + 0x18),(local_38 & 0xff) + iVar14);
      uVar12 = puVar9[1];
      if ((uVar12 & 0x80) == 0) {
        uVar1 = *(ushort *)((long)puVar9 + 10);
        if (this[0x8e] != (m_audio_b_FWDCLInputProgram)0x0) {
          *(ushort *)(lVar5 + (ulong)(local_34 & 0x7ff) * 2) = uVar1 << 8 | uVar1 >> 8;
          local_34 = local_34 + 1;
        }
        if (uVar1 == 0xffff) {
          *(int *)(this + 0x510) = *(int *)(this + 0x510) + 1;
        }
        else {
          *(undefined4 *)(this + 0x510) = 0;
          if ((iVar2 != 0 && bVar15) && (*(char *)(*(long *)(this + 0x80) + 0x58) != '\0')) {
            ProcessCompoundMIDIInputForPacket((int *)this,(int)puVar9 + 0xc);
          }
          mVar13 = SUB41(uVar12 >> 0x18,0);
          if (this[0x504] == (m_audio_b_FWDCLInputProgram)0x0) {
            this[0x504] = (m_audio_b_FWDCLInputProgram)0x1;
            this[0x505] = mVar13;
            mVar7 = mVar13;
          }
          else {
            mVar7 = this[0x505];
          }
          if ((uint)(byte)mVar7 != uVar12 >> 0x18) {
            this[0x505] = mVar13;
          }
          CopySamplesFromPacket
                    (this,(iVar3 + iVar2) * iVar4,puVar9,(TRingBuffer *)(local_40 + 0x28));
          this[0x505] = (m_audio_b_FWDCLInputProgram)((char)this[0x505] + (char)this[0x4ac]);
          puVar9[1] = 0x80;
          if ((this[0x8e] != (m_audio_b_FWDCLInputProgram)0x0) &&
             (lVar6 = *(long *)(this + 0x80), *(char *)(lVar6 + 0x11) != '\0')) {
            cVar8 = ResyncInputBuffer(this,(uint)(ushort)(*(ushort *)((long)puVar9 + 10) << 8 |
                                                         *(ushort *)((long)puVar9 + 10) >> 8) +
                                           (uVar10 & 0xffff0000));
            *(bool *)(lVar6 + 0x11) = cVar8 == '\0';
          }
        }
        uVar10 = _AddFWCycleTimeToFWCycleTime(uVar10,0x1000);
      }
      iVar14 = iVar14 + 1;
    } while (iVar14 != 4);
    if (this[0x8e] != (m_audio_b_FWDCLInputProgram)0x0) {
      *(uint *)(*(long *)(this + 0x80) + 0x20) = local_34;
    }
    if (((iVar2 != 0 && bVar15) &&
        (*(m_audio_b_FWDCLOutputProgram **)(this + 0x508) != (m_audio_b_FWDCLOutputProgram *)0x0))
       && ((local_64 & 1) == 0)) {
      m_audio_b_FWDCLOutputProgram::ProcessCompoundMIDIOutput
                (*(m_audio_b_FWDCLOutputProgram **)(this + 0x508),3);
    }
    if (param_1 < local_64 + 1) break;
    local_40 = *(long *)(this + 0x80);
    local_38 = (uint)(byte)((char)local_38 + 4);
    local_64 = local_64 + 1;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::SetPacketParameters @ 000272f4 ===  

/* m_audio_b_FWDCLProgram::SetPacketParameters(unsigned int, unsigned int, unsigned int,
   SYTInterval*, FDFValue*) */

undefined8 __thiscall
m_audio_b_FWDCLProgram::SetPacketParameters
          (m_audio_b_FWDCLProgram *this,uint param_1,uint param_2,uint param_3,SYTInterval *param_4,
          FDFValue *param_5)

{
  char cVar1;
  undefined *puVar2;
  undefined8 uVar3;
  char *pcVar4;
  undefined8 uVar5;
  
  *(uint *)(this + 100) = param_2;
  *(uint *)(this + 0x68) = param_3;
  if ((int)param_1 < 48000) {
    if (param_1 == 32000) {
      pcVar4 = (char *)0x7d00;
      uVar3 = 8;
      puVar2 = (undefined *)0x1800;
      uVar5 = 0;
      goto LAB_00027404;
    }
  }
  else if ((int)param_1 < 0x15888) {
    if (param_1 == 48000) {
      pcVar4 = (char *)0xbb80;
      uVar3 = 8;
      puVar2 = (undefined *)0x1000;
      uVar5 = 2;
      goto LAB_00027404;
    }
  }
  else if ((int)param_1 < 0x2ee00) {
    if (param_1 == 0x15888) {
      pcVar4 = (char *)0x15888;
      uVar3 = 0x10;
      puVar2 = &DAT_0000116a;
      uVar5 = 3;
      goto LAB_00027404;
    }
    if (param_1 == 96000) {
      pcVar4 = "";
      uVar3 = 0x10;
      puVar2 = (undefined *)0x1000;
      uVar5 = 4;
      goto LAB_00027404;
    }
    if (param_1 == 0x2b110) {
      pcVar4 = (char *)0x2b110;
      uVar3 = 0x20;
      puVar2 = &DAT_0000116a;
      uVar5 = 5;
      goto LAB_00027404;
    }
  }
  else if (param_1 == 0x2ee00) {
    pcVar4 = "AFirmwareHalt";
    uVar3 = 0x20;
    puVar2 = (undefined *)0x1000;
    uVar5 = 6;
    goto LAB_00027404;
  }
  pcVar4 = "";
  uVar3 = 8;
  puVar2 = &DAT_0000116a;
  uVar5 = 1;
LAB_00027404:
  SetupCIP(this,pcVar4,uVar3,puVar2,uVar5);
  cVar1 = (**(code **)(*(long *)this + 0x118))(this,param_2,param_3);
  if (cVar1 == '\0') {
    uVar3 = 0xe00002bc;
  }
  else {
    if (param_4 != (SYTInterval *)0x0) {
      *(undefined4 *)param_4 = *(undefined4 *)(this + 0x4ac);
    }
    if (param_5 != (FDFValue *)0x0) {
      *(undefined4 *)param_5 = *(undefined4 *)(this + 0x4a4);
    }
    uVar3 = 0;
  }
  return uVar3;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::Initialize @ 00027458 ===  

/* m_audio_b_FWDCLProgram::Initialize(m_audio_b_FWAudioDevice*, FWDCLSharedSyncData*) */

void __thiscall
m_audio_b_FWDCLProgram::Initialize
          (m_audio_b_FWDCLProgram *this,m_audio_b_FWAudioDevice *param_1,
          FWDCLSharedSyncData *param_2)

{
  *(m_audio_b_FWAudioDevice **)(this + 0x70) = param_1;
  *(undefined8 *)(this + 0x78) = *(undefined8 *)(param_1 + 0xe0);
  *(FWDCLSharedSyncData **)(this + 0x80) = param_2;
  *(undefined4 *)(this + 100) = 2;
  SetupCIP(this,0xac44,8,&DAT_0000116a,1);
  RebuildDCLProgram(this);
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::Start @ 0002500e ===  

/* m_audio_b_FWDCLInputProgram::Start() */

void __thiscall m_audio_b_FWDCLInputProgram::Start(m_audio_b_FWDCLInputProgram *this)

{
  *(undefined4 *)(this + 0x510) = 0;
  this[0x504] = (m_audio_b_FWDCLInputProgram)0x0;
  *(undefined4 *)(*(long *)(this + 0x80) + 0x88) = 0x3f;
  m_audio_b_FWDCLProgram::Start((m_audio_b_FWDCLProgram *)this);
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::Start @ 000250e4 ===  

/* m_audio_b_FWDCLProgram::Start() */

undefined8 __thiscall m_audio_b_FWDCLProgram::Start(m_audio_b_FWDCLProgram *this)

{
  undefined8 uVar1;
  
  if (this[0x8d] == (m_audio_b_FWDCLProgram)0x0) {
    uVar1 = (**(code **)(**(long **)(this + 0x58) + 0x158))();
    if ((int)uVar1 == 0) {
      this[0x8d] = (m_audio_b_FWDCLProgram)0x1;
    }
  }
  else {
    uVar1 = 0xe00002c2;
  }
  return uVar1;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::Stop @ 00025120 ===  

/* m_audio_b_FWDCLProgram::Stop() */

void __thiscall m_audio_b_FWDCLProgram::Stop(m_audio_b_FWDCLProgram *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x58);
  if ((plVar1 != (long *)0x0) && (this[0x8d] != (m_audio_b_FWDCLProgram)0x0)) {
    (**(code **)(*plVar1 + 0x160))(plVar1);
    this[0x8d] = (m_audio_b_FWDCLProgram)0x0;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::Stop @ 00025160 ===  

/* m_audio_b_FWDCLOutputProgram::Stop() */

void __thiscall m_audio_b_FWDCLOutputProgram::Stop(m_audio_b_FWDCLOutputProgram *this)

{
  if (this[0x8d] != (m_audio_b_FWDCLOutputProgram)0x0) {
    m_audio_b_FWDCLProgram::Stop((m_audio_b_FWDCLProgram *)this);
    **(int **)(this + 0x80) = **(int **)(this + 0x80) + -1;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket @ 0002525c ===  

/* m_audio_b_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket(int*, unsigned int) */

void m_audio_b_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket(int *param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  long lVar4;
  int *piVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  undefined4 in_register_00000034;
  
  iVar1 = param_1[0x19];
  iVar7 = param_1[0x1a] + iVar1;
  iVar2 = param_1[299];
  if (iVar2 * iVar7 != 0) {
    uVar6 = 0;
    uVar8 = uVar6;
    uVar9 = uVar6;
    do {
      uVar3 = *(undefined4 *)(CONCAT44(in_register_00000034,param_2) + (ulong)(iVar1 + uVar6) * 4);
      if (((char)uVar3 == -0x7f) && (uVar9 < *(uint *)(*(long *)(param_1 + 0x20) + 0x5c))) {
        lVar4 = *(long *)(*(long *)(param_1 + 0x20) + 0x70);
        piVar5 = *(int **)(lVar4 + (ulong)uVar9 * 0x28);
        *(char *)((long)piVar5 + (long)(*piVar5 % 0x200) + 4) = (char)((uint)uVar3 >> 8);
        piVar5 = *(int **)(lVar4 + (ulong)uVar9 * 0x28);
        *piVar5 = *piVar5 + 1;
      }
      uVar10 = uVar9 + 1;
      uVar9 = 0;
      if (uVar10 < 8) {
        uVar9 = uVar10;
      }
      uVar6 = uVar6 + iVar7;
      uVar8 = uVar8 + iVar7;
    } while (uVar8 < (uint)(iVar2 * iVar7));
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLInputProgram::InputDCLStaticCallback @ 00025abe ===  

/* m_audio_b_FWDCLInputProgram::InputDCLStaticCallback(DCLCommandStruct*) */

void m_audio_b_FWDCLInputProgram::InputDCLStaticCallback(DCLCommandStruct *param_1)

{
  undefined8 *puVar1;
  
  if (((param_1 != (DCLCommandStruct *)0x0) &&
      (puVar1 = *(undefined8 **)(param_1 + 0x20), puVar1 != (undefined8 *)0x0)) &&
     ((m_audio_b_FWDCLInputProgram *)*puVar1 != (m_audio_b_FWDCLInputProgram *)0x0)) {
    InputDCLCallbackCopyPacketGroupToSamples
              ((m_audio_b_FWDCLInputProgram *)*puVar1,*(uint *)(puVar1 + 1));
    return;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLOutputProgram::OutputDCLStaticCallback @ 00025e2e ===  

/* m_audio_b_FWDCLOutputProgram::OutputDCLStaticCallback(DCLCommandStruct*) */

void m_audio_b_FWDCLOutputProgram::OutputDCLStaticCallback(DCLCommandStruct *param_1)

{
  undefined8 *puVar1;
  
  if (((param_1 != (DCLCommandStruct *)0x0) &&
      (puVar1 = *(undefined8 **)(param_1 + 0x20), puVar1 != (undefined8 *)0x0)) &&
     ((m_audio_b_FWDCLOutputProgram *)*puVar1 != (m_audio_b_FWDCLOutputProgram *)0x0)) {
    OutputDCLCallback((m_audio_b_FWDCLOutputProgram *)*puVar1,puVar1[1],false);
    return;
  }
  return;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::ResetPort @ 0002541a ===  

/* m_audio_b_FWDCLProgram::ResetPort(bool, unsigned int) */

ulong __thiscall
m_audio_b_FWDCLProgram::ResetPort(m_audio_b_FWDCLProgram *this,bool param_1,uint param_2)

{
  code *pcVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  ulong uVar4;
  long *plVar5;
  uint uVar6;
  
  if ((((this[0x8c] == (m_audio_b_FWDCLProgram)0x0) || (*(long *)(this + 0x78) == 0)) ||
      (*(long *)(this + 0x10) == 0)) ||
     (((*(long *)(*(long *)(this + 0x10) + 0x10) == 0 || (*(long *)(this + 0x58) == 0)) ||
      (plVar5 = (long *)IOFireWireNub::getBus(), plVar5 == (long *)0x0)))) {
    uVar4 = 0xe00002bc;
  }
  else {
    *(uint *)(this + 0x4d4) = param_2;
    if (*(long **)(*(long *)(this + 0x58) + 0x28) != (long *)0x0) {
      (**(code **)(**(long **)(*(long *)(this + 0x58) + 0x28) + 0x128))();
    }
    uVar6 = param_2 >> 0xc;
    uVar2 = 0x7fff;
    if (param_1) {
      uVar6 = param_2;
      uVar2 = 0x7fff000;
    }
    if (*(long *)(this + 0x10) == 0) {
                    /* WARNING: Subroutine does not return */
      _panic(
            "\"DCL program is NULL\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWDCLBuilder.h:85"
            );
    }
    plVar5 = (long *)(**(code **)(*plVar5 + 0x858))
                               (plVar5,param_1,*(undefined8 *)(*(long *)(this + 0x10) + 0x10),0,1,
                                uVar6,uVar2);
    if (plVar5 == (long *)0x0) {
      uVar4 = 0xe00002be;
    }
    else {
      uVar4 = (**(code **)(**(long **)(this + 0x58) + 0x138))(*(long **)(this + 0x58),plVar5);
      if ((int)uVar4 == 0) {
        uVar2 = *(undefined4 *)(*(long *)(this + 0x58) + 0x4c);
        pcVar1 = *(code **)(*plVar5 + 0x120);
        uVar3 = (**(code **)(**(long **)(this + 0x78) + 0x850))();
        uVar6 = (*pcVar1)(plVar5,uVar3,uVar2);
        uVar4 = (ulong)uVar6;
        if (uVar6 == 0) {
          (**(code **)(*plVar5 + 0x28))(plVar5);
        }
      }
    }
  }
  return uVar4;
}

 (GhidraScript)  
=== m_audio_b_FWDCLProgram::Disconnect @ 000250a2 ===  

/* m_audio_b_FWDCLProgram::Disconnect() */

void __thiscall m_audio_b_FWDCLProgram::Disconnect(m_audio_b_FWDCLProgram *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x58);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x150))(plVar1);
    (**(code **)(**(long **)(this + 0x58) + 0x28))();
    *(undefined8 *)(this + 0x58) = 0;
    this[0x8c] = (m_audio_b_FWDCLProgram)0x0;
  }
  return;
}

 (GhidraScript)  

######## aux: TRingBuffer, FWTPacketIndex, iterators, TPacketIterator::CommonInit (Ghidra 12.1.4 headless, 2026-10-15)
=== m_audio_FWb::TRingBuffer::AdvanceIndex @ 00007b6a ===  

/* m_audio_FWb::TRingBuffer::AdvanceIndex(unsigned int, unsigned int&, unsigned int&) */

undefined8 __thiscall
m_audio_FWb::TRingBuffer::AdvanceIndex(TRingBuffer *this,uint param_1,uint *param_2,uint *param_3)

{
  uint uVar1;
  
  uVar1 = param_1 + *param_2;
  *param_2 = uVar1;
  if (*(uint *)(this + 0x10) <= uVar1) {
    *param_2 = uVar1 - *(uint *)(this + 0x10);
    *param_3 = *param_3 + 1;
    return 1;
  }
  return 0;
}

 (GhidraScript)  
=== m_audio_FWb::TRingBuffer::WriteNextData @ 00007b8a ===  

/* m_audio_FWb::TRingBuffer::WriteNextData(unsigned int, void*) */

void __thiscall
m_audio_FWb::TRingBuffer::WriteNextData(TRingBuffer *this,uint param_1,void *param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  size_t sVar5;
  
  sVar5 = (size_t)param_1;
  uVar1 = *(uint *)(this + 0x18);
  uVar3 = uVar1 + param_1;
  *(uint *)(this + 0x18) = uVar3;
  uVar2 = *(uint *)(this + 0x10);
  if (uVar3 < uVar2) {
    this[0x25] = (TRingBuffer)0x0;
  }
  else {
    uVar4 = uVar3 - uVar2;
    *(uint *)(this + 0x18) = uVar4;
    *(int *)(this + 0x20) = *(int *)(this + 0x20) + 1;
    this[0x25] = (TRingBuffer)0x1;
    if (uVar3 != uVar2) {
      sVar5 = (size_t)(param_1 - uVar4);
      _memcpy(*(void **)(this + 8),(void *)((long)param_2 + sVar5),(ulong)uVar4);
    }
  }
  _memcpy((void *)((ulong)uVar1 + *(long *)(this + 8)),param_2,sVar5);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TRingBuffer::ReadNextData @ 00007bf8 ===  

/* m_audio_FWb::TRingBuffer::ReadNextData(unsigned int, void*) */

void __thiscall m_audio_FWb::TRingBuffer::ReadNextData(TRingBuffer *this,uint param_1,void *param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  size_t sVar5;
  
  sVar5 = (size_t)param_1;
  uVar1 = *(uint *)(this + 0x14);
  uVar3 = uVar1 + param_1;
  *(uint *)(this + 0x14) = uVar3;
  uVar2 = *(uint *)(this + 0x10);
  if (uVar3 < uVar2) {
    this[0x24] = (TRingBuffer)0x0;
  }
  else {
    uVar4 = uVar3 - uVar2;
    *(uint *)(this + 0x14) = uVar4;
    *(int *)(this + 0x1c) = *(int *)(this + 0x1c) + 1;
    this[0x24] = (TRingBuffer)0x1;
    if (uVar3 != uVar2) {
      sVar5 = (size_t)(param_1 - uVar4);
      _memcpy((void *)((long)param_2 + sVar5),*(void **)(this + 8),(ulong)uVar4);
    }
  }
  _memcpy(param_2,(void *)(*(long *)(this + 8) + (ulong)uVar1),sVar5);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TRingBuffer::Invalidate @ 00007c6a ===  

/* m_audio_FWb::TRingBuffer::Invalidate() */

void __thiscall m_audio_FWb::TRingBuffer::Invalidate(TRingBuffer *this)

{
  long *plVar1;
  
  plVar1 = *(long **)this;
  if (plVar1 != (long *)0x0) {
    *(undefined4 *)(this + 0x10) = 0;
    *(undefined8 *)(this + 8) = 0;
    (**(code **)(*plVar1 + 0x1f8))(plVar1,0);
    (**(code **)(**(long **)this + 0x28))();
    *(undefined8 *)this = 0;
  }
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes @ 00007cc4 ===  

/* m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes(unsigned int) */

undefined8 __thiscall
m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes(TRingBuffer *this,uint param_1)

{
  int iVar1;
  long *plVar2;
  void *pvVar3;
  
  if (*(long *)this != 0) {
    Invalidate(this);
  }
  *(uint *)(this + 0x10) = param_1;
  plVar2 = (long *)IOBufferMemoryDescriptor::withOptions(0,(ulong)param_1,0x1000);
  *(long **)this = plVar2;
  if (plVar2 != (long *)0x0) {
    iVar1 = (**(code **)(*plVar2 + 0x1f0))(plVar2,0);
    if (iVar1 == 0) {
      pvVar3 = (void *)(**(code **)(**(long **)this + 0x2e0))();
      *(void **)(this + 8) = pvVar3;
      if (pvVar3 != (void *)0x0) {
        _bzero(pvVar3,(ulong)*(uint *)(this + 0x10));
        *(undefined8 *)(this + 0x1c) = 0;
        *(undefined8 *)(this + 0x14) = 0;
        return 1;
      }
      (**(code **)(**(long **)this + 0x1f8))(*(long **)this,0);
    }
    (**(code **)(**(long **)this + 0x28))();
    *(undefined8 *)this = 0;
  }
  return 0;
}

 (GhidraScript)  
=== m_audio_b_FWTPacketIndex::withElementCount @ 000107c2 ===  

/* m_audio_b_FWTPacketIndex::withElementCount(unsigned int) */

OSObject * __thiscall
m_audio_b_FWTPacketIndex::withElementCount(m_audio_b_FWTPacketIndex *this,uint param_1)

{
  char cVar1;
  OSObject *this_00;
  undefined4 in_register_00000034;
  
  if ((int)this != 0) {
    this_00 = OSObject::operator_new
                        ((OSObject *)&segment_command_00000020.filesize,
                         CONCAT44(in_register_00000034,param_1));
    if (this_00 == (OSObject *)0x0) {
      return (OSObject *)0x0;
    }
    OSObject::OSObject(this_00,(OSMetaClass *)&gMetaClass);
    *(undefined ***)this_00 = &PTR__m_audio_b_FWTPacketIndex_00038130;
    OSMetaClass::instanceConstructed();
    cVar1 = (**(code **)(*(long *)this_00 + 0x118))(this_00,(ulong)this & 0xffffffff);
    if (cVar1 != '\0') {
      return this_00;
    }
    (**(code **)(*(long *)this_00 + 8))(this_00);
  }
  return (OSObject *)0x0;
}

 (GhidraScript)  
=== m_audio_b_FWTPacketIndex::free @ 00010498 ===  

/* m_audio_b_FWTPacketIndex::free() */

void __thiscall m_audio_b_FWTPacketIndex::free(m_audio_b_FWTPacketIndex *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x10);
  if (plVar1 != (long *)0x0) {
    *(undefined8 *)(this + 0x30) = 0;
    *(undefined8 *)(this + 0x38) = 0;
    *(undefined8 *)(this + 0x40) = 0;
    *(undefined8 *)(this + 0x48) = 0;
    (**(code **)(*plVar1 + 0x1f8))(plVar1,0);
    (**(code **)(**(long **)(this + 0x10) + 0x28))();
    *(undefined8 *)(this + 0x10) = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000104fd. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300d8 + 0xa0))(this);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator @ 000114ec ===  

/* m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator(m_audio_FWb::TPacketPool&, unsigned
   int, unsigned int) */

void __thiscall
m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator
          (TPacketForwardIterator *this,TPacketPool *param_1,uint param_2,uint param_3)

{
  *(TPacketPool **)this = param_1;
  TPacketIterator::CommonInit((TPacketIterator *)this,param_2,param_3,false);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator @ 00011500 ===  

/* m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator(m_audio_FWb::TPacketPool&, unsigned
   int, unsigned int) */

void __thiscall
m_audio_FWb::TPacketForwardIterator::TPacketForwardIterator
          (TPacketForwardIterator *this,TPacketPool *param_1,uint param_2,uint param_3)

{
  *(TPacketPool **)this = param_1;
  TPacketIterator::CommonInit((TPacketIterator *)this,param_2,param_3,false);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketReverseIterator::TPacketReverseIterator @ 00011514 ===  

/* m_audio_FWb::TPacketReverseIterator::TPacketReverseIterator(m_audio_FWb::TPacketPool&, unsigned
   int, unsigned int) */

void __thiscall
m_audio_FWb::TPacketReverseIterator::TPacketReverseIterator
          (TPacketReverseIterator *this,TPacketPool *param_1,uint param_2,uint param_3)

{
  int iVar1;
  uint uVar2;
  long lVar3;
  uint uVar4;
  
  *(TPacketPool **)this = param_1;
  TPacketIterator::CommonInit((TPacketIterator *)this,param_2,param_3,true);
  lVar3 = *(long *)this;
  iVar1 = *(int *)(param_1 + 0x18);
  uVar2 = *(uint *)(lVar3 + 0x28);
  uVar4 = (iVar1 - 1U) % uVar2;
  *(uint *)(this + 0x38) = uVar4 + 1;
  *(ulong *)(this + 0x30) =
       (ulong)(((iVar1 - 1U) / uVar2) * 0x1000 + uVar4 * *(int *)(lVar3 + 0x20)) +
       *(long *)(lVar3 + 0x10);
  return;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketReverseIterator::PreviousPacket @ 000115ac ===  

/* m_audio_FWb::TPacketReverseIterator::PreviousPacket() */

long __thiscall m_audio_FWb::TPacketReverseIterator::PreviousPacket(TPacketReverseIterator *this)

{
  long lVar1;
  long lVar2;
  ulong uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  char local_90 [128];
  
  lVar1 = *(long *)(this + 0x18);
  *(int *)(this + 0x10) = *(int *)(this + 8);
  iVar5 = *(int *)(this + 8) + -1;
  *(int *)(this + 8) = iVar5;
  iVar6 = *(int *)(this + 0x20);
  *(int *)(this + 0x20) = iVar6 + 1;
  if ((iVar6 + 1 == *(int *)(this + 0x2c)) ||
     ((*(int *)(this + 0xc) != -1 && (iVar5 == *(int *)(this + 0xc))))) {
    *(undefined8 *)(this + 0x18) = 0;
    lVar2 = *(long *)this;
    uVar3 = 0;
  }
  else {
    if (iVar5 < 0) {
      lVar2 = *(long *)this;
      iVar6 = *(int *)(lVar2 + 0x18) + -1;
      *(int *)(this + 8) = iVar6;
      *(int *)(this + 0x28) = iVar6 - *(int *)(this + 0x38);
      uVar3 = *(ulong *)(this + 0x30);
    }
    else {
      lVar2 = *(long *)this;
      if (*(int *)(this + 0x28) < iVar5) {
        uVar4 = *(uint *)(lVar2 + 0x20);
      }
      else {
        *(int *)(this + 0x28) = iVar5 - *(int *)(lVar2 + 0x28);
        uVar4 = *(int *)(lVar2 + 0x20) + *(int *)(lVar2 + 0x34);
      }
      uVar3 = lVar1 - (ulong)uVar4;
    }
    *(ulong *)(this + 0x18) = uVar3;
  }
  if (((uVar3 < *(ulong *)(lVar2 + 0x10)) && (uVar3 != 0)) && (lVar1 != 0)) {
    _snprintf(local_90,0x80,"fPacketAddress is %lu, should be >= %p (index:%u)");
                    /* WARNING: Subroutine does not return */
    _panic(
          "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/TPacketIterator.cpp:216"
          );
  }
  return lVar1;
}

 (GhidraScript)  
=== m_audio_FWb::TPacketForwardIterator::NextPacket @ 00011686 ===  

/* m_audio_FWb::TPacketForwardIterator::NextPacket() */

long __thiscall m_audio_FWb::TPacketForwardIterator::NextPacket(TPacketForwardIterator *this)

{
  int iVar1;
  long lVar2;
  long lVar3;
  ulong uVar4;
  uint uVar5;
  char local_90 [128];
  ulong uVar6;
  
  lVar2 = *(long *)(this + 0x18);
  *(int *)(this + 0x10) = *(int *)(this + 8);
  uVar5 = *(int *)(this + 8) + 1;
  uVar6 = (ulong)uVar5;
  *(uint *)(this + 8) = uVar5;
  iVar1 = *(int *)(this + 0x20);
  *(int *)(this + 0x20) = iVar1 + 1;
  if ((iVar1 + 1 == *(int *)(this + 0x2c)) ||
     ((*(uint *)(this + 0xc) != 0xffffffff && (uVar5 == *(uint *)(this + 0xc))))) {
    *(undefined8 *)(this + 0x18) = 0;
    lVar3 = *(long *)this;
    uVar4 = 0;
  }
  else {
    lVar3 = *(long *)this;
    if (uVar5 < *(uint *)(lVar3 + 0x18)) {
      if (uVar5 < *(uint *)(this + 0x28)) {
        uVar5 = *(uint *)(lVar3 + 0x20);
      }
      else {
        *(uint *)(this + 0x28) = *(int *)(lVar3 + 0x28) + uVar5;
        uVar5 = *(int *)(lVar3 + 0x34) + *(int *)(lVar3 + 0x20);
      }
      uVar4 = (ulong)uVar5 + lVar2;
      *(ulong *)(this + 0x18) = uVar4;
    }
    else {
      *(undefined4 *)(this + 8) = 0;
      *(undefined4 *)(this + 0x28) = *(undefined4 *)(lVar3 + 0x28);
      uVar4 = *(ulong *)(lVar3 + 0x10);
      *(ulong *)(this + 0x18) = uVar4;
      uVar6 = 0;
    }
  }
  if (((uVar4 < *(ulong *)(lVar3 + 0x10)) && (uVar4 != 0)) && (lVar2 != 0)) {
    _snprintf(local_90,0x80,"fPacketAddress is %lu, should be >= %p (index:%u)",uVar4,
              *(ulong *)(lVar3 + 0x10),uVar6);
                    /* WARNING: Subroutine does not return */
    _panic(
          "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/TPacketIterator.cpp:115"
          );
  }
  return lVar2;
}

 (GhidraScript)  
/* m_audio_FWb::TPacketIterator::CommonInit(unsigned int, unsigned int, bool) */

void __thiscall
m_audio_FWb::TPacketIterator::CommonInit
          (TPacketIterator *this,uint param_1,uint param_2,bool param_3)

{
  uint uVar1;
  long lVar2;
  uint uVar3;
  
  lVar2 = *(long *)this;
  uVar1 = *(uint *)(lVar2 + 0x28);
  *(undefined4 *)(this + 0x20) = 0;
  *(undefined4 *)(this + 0x2c) = 0x7fffffff;
  *(uint *)(this + 8) = param_1;
  *(uint *)(this + 0x10) = param_1;
  if (param_2 == 0xffffffff) {
    *(undefined4 *)(this + 0xc) = 0xffffffff;
  }
  else {
    *(uint *)(this + 0xc) = param_2 + param_1;
  }
  uVar3 = param_1 % uVar1;
  *(uint *)(this + 0x24) = param_1 / uVar1;
  if (param_3) {
    *(uint *)(this + 0x28) = (param_1 - 1) - uVar3;
  }
  else {
    *(uint *)(this + 0x28) = (uVar1 + param_1) - uVar3;
  }
  *(ulong *)(this + 0x18) =
       (ulong)((param_1 / uVar1) * 0x1000 + uVar3 * *(int *)(lVar2 + 0x20)) +
       *(long *)(lVar2 + 0x10);
  return;
}

 (GhidraScript)  
