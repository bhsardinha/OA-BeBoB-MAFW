=== m_audio_b_FWKIPCUserClient::initWithTask @ 0001618e ===

/* m_audio_b_FWKIPCUserClient::initWithTask(task*, void*, unsigned int) */

bool __thiscall
m_audio_b_FWKIPCUserClient::initWithTask
          (m_audio_b_FWKIPCUserClient *this,task *param_1,void *param_2,uint param_3)

{
  char cVar1;
  
  _bzero(this + 0xe0,0x2c);
  *(undefined4 *)(this + 0x118) = 0;
  *(undefined8 *)(this + 0x110) = 0;
  *(undefined8 *)(this + 0x120) = 0;
  *(task **)(this + 0x138) = param_1;
  cVar1 = (**(code **)(PTR_vtable_000300a0 + 0x8f8))(this,param_1,param_2,param_3);
  return cVar1 != '\0';
}


=== m_audio_b_FWKIPCUserClient::start @ 0001625a ===

/* m_audio_b_FWKIPCUserClient::start(IOService*) */

bool __thiscall
m_audio_b_FWKIPCUserClient::start(m_audio_b_FWKIPCUserClient *this,IOService *param_1)

{
  char cVar1;
  
  cVar1 = (**(code **)(PTR_vtable_000300a0 + 0x5d0))(this,param_1);
  if (cVar1 != '\0') {
    *(IOService **)(this + 0xd8) = param_1;
    UserClientAdded(this);
  }
  return cVar1 != '\0';
}


=== m_audio_b_FWKIPCUserClient::stop @ 000160e4 ===

/* m_audio_b_FWKIPCUserClient::stop(IOService*) */

void __thiscall
m_audio_b_FWKIPCUserClient::stop(m_audio_b_FWKIPCUserClient *this,IOService *param_1)

{
  *(undefined8 *)(this + 0xe8) = 0;
  UserClientRemoved(this);
  if (*(long **)(this + 0x110) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x110) + 0x28))();
    *(undefined8 *)(this + 0x110) = 0;
  }
  if (*(long *)(this + 0x120) != 0) {
    _IORecursiveLockFree();
    *(undefined8 *)(this + 0x120) = 0;
  }
  if (*(long **)(this + 0x128) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x128) + 0x28))();
    *(undefined8 *)(this + 0x128) = 0;
  }
  if (*(long **)(this + 0x130) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x130) + 0x28))();
    *(undefined8 *)(this + 0x130) = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x00016188. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300a0 + 0x5d8))(this,param_1);
  return;
}


=== m_audio_b_FWKIPCUserClient::registerNotificationPort @ 00015f3a ===

/* m_audio_b_FWKIPCUserClient::registerNotificationPort(ipc_port*, unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::registerNotificationPort
          (m_audio_b_FWKIPCUserClient *this,ipc_port *param_1,uint param_2,uint param_3)

{
  if (param_2 == 0) {
    *(undefined4 *)(this + 0xe0) = 0x13;
    *(undefined4 *)(this + 0xe4) = 0x2c;
    *(undefined8 *)(this + 0xf0) = 0;
    *(undefined4 *)(this + 0xf8) = 0;
    *(undefined4 *)(this + 0xfc) = 0;
    *(ipc_port **)(this + 0xe8) = param_1;
    *(undefined4 *)(this + 0x108) = *(undefined4 *)(this + 0x118);
    *(uint *)(this + 0x104) = param_3;
    return 0;
  }
  _IOLog("M-FW: Someone registered for an unknown message type:%X\n",param_2);
  return 0xe00002c2;
}


=== m_audio_b_FWKIPCUserClient::SendMachMessage @ 00015faa ===

/* m_audio_b_FWKIPCUserClient::SendMachMessage(unsigned int, MAudioMessage*, unsigned long) */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::SendMachMessage
          (m_audio_b_FWKIPCUserClient *this,uint param_1,MAudioMessage *param_2,ulong param_3)

{
  int iVar1;
  undefined8 uVar2;
  
  *(uint *)(param_2 + 0x20) = param_1;
  iVar1 = _mach_msg_send_from_kernel_proper(param_2,param_3 & 0xffffffff);
  uVar2 = 0xe00002bc;
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  return uVar2;
}


=== m_audio_b_FWKIPCUserClient::message @ 00015fca ===

/* m_audio_b_FWKIPCUserClient::message(unsigned int, IOService*, void*) */

void __thiscall
m_audio_b_FWKIPCUserClient::message
          (m_audio_b_FWKIPCUserClient *this,uint param_1,IOService *param_2,void *param_3)

{
  if (*(long *)(this + 0xe8) != 0) {
    *(uint *)(this + 0x100) = param_1;
    _mach_msg_send_from_kernel_proper(this + 0xe0,*(undefined4 *)(this + 0xe4));
  }
                    /* WARNING: Could not recover jumptable at 0x00016023. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300a0 + 0x740))(this,param_1,param_2,param_3);
  return;
}


=== m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage @ 0001602a ===

/* m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage() */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage(m_audio_b_FWKIPCUserClient *this)

{
  undefined8 uVar1;
  undefined4 uVar2;
  int iVar3;
  long *plVar4;
  undefined8 *puVar5;
  
  plVar4 = (long *)SerializeModifiedProperties(this);
  if (plVar4 != (long *)0x0) {
    puVar5 = (undefined8 *)(**(code **)(*plVar4 + 0x2e0))(plVar4);
    uVar2 = (**(code **)(*plVar4 + 0x1c8))(plVar4);
    if (*(long *)(this + 0xe8) != 0) {
      *(undefined4 *)(puVar5 + 5) = *(undefined4 *)(this + 0x108);
      puVar5[4] = *(undefined8 *)(this + 0x100);
      puVar5[3] = *(undefined8 *)(this + 0xf8);
      puVar5[2] = *(undefined8 *)(this + 0xf0);
      uVar1 = *(undefined8 *)(this + 0xe0);
      puVar5[1] = *(undefined8 *)(this + 0xe8);
      *puVar5 = uVar1;
      *(undefined4 *)((long)puVar5 + 4) = uVar2;
      *(undefined4 *)(puVar5 + 4) = 0;
      iVar3 = _mach_msg_send_from_kernel_proper();
      if (iVar3 != 0) {
        return 0xe00002bc;
      }
      return 0;
    }
  }
  return 0xe00002bc;
}


=== m_audio_b_FWKIPCUserClient::SerializeModifiedProperties @ 00015cba ===

/* m_audio_b_FWKIPCUserClient::SerializeModifiedProperties() */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::SerializeModifiedProperties(m_audio_b_FWKIPCUserClient *this)

{
  char cVar1;
  uint uVar2;
  long lVar3;
  long *plVar4;
  void *pvVar5;
  ulong uVar6;
  
  if ((*(long *)(this + 0xe8) == 0) || (*(long *)(this + 0x110) == 0)) goto LAB_00015df1;
  if (*(long **)(this + 0x130) == (long *)0x0) {
    lVar3 = OSSerialize::withCapacity(0x200);
    *(long *)(this + 0x130) = lVar3;
    if (lVar3 == 0) goto LAB_00015df1;
  }
  else {
    (**(code **)(**(long **)(this + 0x130) + 0x120))();
  }
  _IORecursiveLockLock(*(undefined8 *)(this + 0x120));
  cVar1 = (**(code **)(**(long **)(this + 0x110) + 0x30))
                    (*(long **)(this + 0x110),*(undefined8 *)(this + 0x130));
  (**(code **)(**(long **)(this + 0x110) + 0x158))();
  _IORecursiveLockUnlock(*(undefined8 *)(this + 0x120));
  if (cVar1 == '\0') goto LAB_00015df1;
  uVar2 = (**(code **)(**(long **)(this + 0x130) + 0x158))();
  uVar6 = (ulong)uVar2;
  plVar4 = (long *)0x0;
  if (*(long **)(this + 0x128) == (long *)0x0) {
LAB_00015d99:
    if (plVar4 != (long *)0x0) {
      (**(code **)(*plVar4 + 0x28))();
    }
    plVar4 = (long *)IOBufferMemoryDescriptor::withCapacity(uVar6 + 0x2c,1,false);
    *(long **)(this + 0x128) = plVar4;
  }
  else {
    lVar3 = (**(code **)(**(long **)(this + 0x128) + 0x2d8))();
    plVar4 = *(long **)(this + 0x128);
    if (lVar3 != uVar6 + 0x2c) goto LAB_00015d99;
  }
  if (plVar4 != (long *)0x0) {
    lVar3 = (**(code **)(*plVar4 + 0x2e0))();
    pvVar5 = (void *)(**(code **)(**(long **)(this + 0x130) + 0x118))();
    _memcpy((void *)(lVar3 + 0x2c),pvVar5,uVar6);
  }
LAB_00015df1:
  return *(undefined8 *)(this + 0x128);
}


=== m_audio_b_FWKIPCUserClient::AddModifiedProperties @ 00015e04 ===

/* m_audio_b_FWKIPCUserClient::AddModifiedProperties(OSDictionary*) */

void __thiscall
m_audio_b_FWKIPCUserClient::AddModifiedProperties
          (m_audio_b_FWKIPCUserClient *this,OSDictionary *param_1)

{
  code *pcVar1;
  uint uVar2;
  long lVar3;
  long *plVar4;
  undefined8 uVar5;
  
  if (*(long *)(this + 0x110) == 0) {
    uVar2 = (**(code **)(*(long *)param_1 + 0x130))(param_1);
    lVar3 = OSDictionary::withCapacity(uVar2);
    *(long *)(this + 0x110) = lVar3;
    if (lVar3 == 0) {
      return;
    }
    lVar3 = _IORecursiveLockAlloc();
    *(long *)(this + 0x120) = lVar3;
    if (lVar3 == 0) {
      return;
    }
  }
  _OSIncrementAtomic(this + 0x118);
  _IORecursiveLockLock(*(undefined8 *)(this + 0x120));
  if ((param_1 == (OSDictionary *)0x0) ||
     (plVar4 = (long *)OSCollectionIterator::withCollection((OSCollection *)param_1),
     plVar4 == (long *)0x0)) {
    _IORecursiveLockUnlock(*(undefined8 *)(this + 0x120));
    return;
  }
  lVar3 = (**(code **)(*plVar4 + 0x128))(plVar4);
  while (lVar3 != 0) {
    pcVar1 = *(code **)(**(long **)(this + 0x110) + 0x1c0);
    uVar5 = (**(code **)(*(long *)param_1 + 0x1f8))(param_1,lVar3);
    (*pcVar1)(*(undefined8 *)(this + 0x110),lVar3,uVar5);
    lVar3 = (**(code **)(*plVar4 + 0x128))(plVar4);
  }
  _IORecursiveLockUnlock(*(undefined8 *)(this + 0x120));
  if (plVar4 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00015f36. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar4 + 0x28))(plVar4);
  return;
}


=== m_audio_b_FWKIPCUserClient::UserClientAdded @ 0001620c ===

/* m_audio_b_FWKIPCUserClient::UserClientAdded(m_audio_b_FWKIPCUserClient*) */

void m_audio_b_FWKIPCUserClient::UserClientAdded(m_audio_b_FWKIPCUserClient *param_1)

{
  undefined8 in_RDX;
  undefined1 auVar1 [16];
  
  auVar1._8_8_ = in_RDX;
  auVar1._0_8_ = sUserClients;
  if (sUserClients == (long *)0x0) {
    auVar1 = OSSet::withCapacity(4);
  }
  sUserClients = auVar1._0_8_;
  if (sUserClients == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00016257. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*sUserClients + 0x1c0))
            (sUserClients,param_1,auVar1._8_8_,*(code **)(*sUserClients + 0x1c0));
  return;
}


=== m_audio_b_FWKIPCUserClient::UserClientRemoved @ 00015bea ===

/* m_audio_b_FWKIPCUserClient::UserClientRemoved(m_audio_b_FWKIPCUserClient*) */

void m_audio_b_FWKIPCUserClient::UserClientRemoved(m_audio_b_FWKIPCUserClient *param_1)

{
  int iVar1;
  
  if (sUserClients != (long *)0x0) {
    (**(code **)(*sUserClients + 0x1d8))(sUserClients,param_1);
    iVar1 = (**(code **)(*sUserClients + 0x130))();
    if (iVar1 == 0) {
      (**(code **)(*sUserClients + 0x28))();
      sUserClients = (long *)0x0;
    }
  }
  return;
}


=== m_audio_b_FWKIPCUserClient::clientClose @ 00015c38 ===

/* m_audio_b_FWKIPCUserClient::clientClose() */

undefined8 __thiscall m_audio_b_FWKIPCUserClient::clientClose(m_audio_b_FWKIPCUserClient *this)

{
  *(undefined8 *)(this + 0xe8) = 0;
  return 0;
}


INFO  REPORT: Save succeeded for processed file: /M-AudioFireWireBeBoB.x86_64-slice (HeadlessAnalyzer)  
