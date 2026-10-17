// m_audio_b_FWDCLProgram::CreateIsochChannelAndPorts
// Entry: 000255c2  Size: 420 bytes
// Prototype: undefined CreateIsochChannelAndPorts(m_audio_b_FWDCLProgram * this, bool param_1, uint param_2, uint param_3)


/* m_audio_b_FWDCLProgram::CreateIsochChannelAndPorts(bool, unsigned int, unsigned int) */

ulong __thiscall
m_audio_b_FWDCLProgram::CreateIsochChannelAndPorts
          (m_audio_b_FWDCLProgram *this,bool param_1,uint param_2,uint param_3)

{
  code *pcVar1;
  char cVar2;
  undefined4 uVar3;
  uint uVar4;
  long *plVar5;
  ulong uVar6;
  m_audio_b_FWRemoteIsochPort *this_00;
  ulong uVar7;
  m_audio_b_FWIsochChannel *this_01;
  undefined8 uVar8;
  undefined8 in_stack_ffffffffffffffb8;
  
  uVar4 = (uint)((ulong)in_stack_ffffffffffffffb8 >> 0x20);
  *(uint *)(this + 0x88) = param_2;
  plVar5 = (long *)IOFireWireNub::getBus();
  if (*(long *)(this + 0x10) == 0) {
                    /* WARNING: Subroutine does not return */
    _panic(
          "\"DCL program is NULL\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWDCLBuilder.h:85"
          );
  }
  uVar6 = (ulong)param_1;
  plVar5 = (long *)(**(code **)(*plVar5 + 0x858))
                             (plVar5,uVar6,*(undefined8 *)(*(long *)(this + 0x10) + 0x10),0,0,0,
                              (ulong)uVar4 << 0x20);
  if (plVar5 == (long *)0x0) {
LAB_00025637:
    uVar6 = 0xe00002be;
  }
  else {
    this_00 = OSObject::operator_new((OSObject *)&segment_command_00000020,uVar6);
    if (this_00 != (m_audio_b_FWRemoteIsochPort *)0x0) {
      m_audio_b_FWRemoteIsochPort::m_audio_b_FWRemoteIsochPort(this_00);
      uVar6 = *(ulong *)(*(long *)(this + 0x70) + 0xf0);
      uVar7 = (**(code **)(*(long *)this_00 + 0x150))(this_00,uVar6,!param_1,param_2);
      if ((int)uVar7 != 0) {
        return uVar7;
      }
      this_01 = OSObject::operator_new((OSObject *)(section_00000068.segname + 8),uVar6);
      if (this_01 != (m_audio_b_FWIsochChannel *)0x0) {
        m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel(this_01);
        pcVar1 = *(code **)(*(long *)this_01 + 0x128);
        uVar3 = (**(code **)(**(long **)(this + 0x78) + 0x850))();
        uVar8 = IOFireWireNub::getController();
        cVar2 = (*pcVar1)(this_01,uVar8,1,param_3,uVar3,ForceStopNotificationProc,this);
        if (cVar2 != '\0') {
          (**(code **)(*(long *)this_01 + 0x138))(this_01,plVar5);
          (**(code **)(*(long *)this_01 + 0x140))(this_01,this_00);
          (**(code **)(*plVar5 + 0x28))(plVar5);
          (**(code **)(*(long *)this_00 + 0x28))(this_00);
          uVar4 = (**(code **)(*(long *)this_01 + 0x148))(this_01);
          if (uVar4 != 0) {
            (**(code **)(*(long *)this_01 + 0x28))(this_01);
            return (ulong)uVar4;
          }
          *(m_audio_b_FWIsochChannel **)(this + 0x58) = this_01;
          this[0x8c] = (m_audio_b_FWDCLProgram)0x1;
          return 0;
        }
        goto LAB_00025637;
      }
    }
    uVar6 = 0xe00002bd;
  }
  return uVar6;
}


