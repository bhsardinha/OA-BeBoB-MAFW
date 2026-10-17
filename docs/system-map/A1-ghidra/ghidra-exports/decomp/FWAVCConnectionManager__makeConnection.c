// m_audio_b_FWAVCConnectionManager::makeConnection
// Entry: 00024980
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::makeConnection(unsigned int, unsigned int, IOFWSpeed) */

int __thiscall
m_audio_b_FWAVCConnectionManager::makeConnection
          (m_audio_b_FWAVCConnectionManager *this,ulong param_2_00,undefined4 param_2,
          undefined4 param_4)

{
  int iVar1;
  long lVar2;
  OSObject *this_00;
  ulong uVar3;
  
  uVar3 = param_2_00;
  if (*(long *)(this + 0x18) == 0) {
    lVar2 = OSArray::withCapacity(4);
    *(long *)(this + 0x18) = lVar2;
    if (lVar2 == 0) {
      return -0x1ffffd43;
    }
  }
  this_00 = OSObject::operator_new((OSObject *)&MACH_HEADER.flags,uVar3);
  if (this_00 == (OSObject *)0x0) {
    return -0x1ffffd43;
  }
  OSObject::OSObject(this_00,(OSMetaClass *)&FWP2PConnection::gMetaClass);
  *(undefined ***)this_00 = &PTR__FWP2PConnection_00049fb0;
  OSMetaClass::instanceConstructed();
  iVar1 = (**(code **)(*(long *)this + 0x170))(this,param_2_00 & 0xffffffff,1,0,param_2,param_4);
  if (iVar1 == 0) {
    *(int *)(this_00 + 0xc) = (int)param_2_00;
    *(undefined4 *)(this_00 + 0x10) = param_2;
    (**(code **)(**(long **)(this + 0x18) + 0x1b8))(*(long **)(this + 0x18),this_00);
  }
  (**(code **)(*(long *)this_00 + 0x28))(this_00);
  return iVar1;
}
