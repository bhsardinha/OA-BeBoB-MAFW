// m_audio_b_FWAVCUnit::AVCCommandExtendedTimeout
// Entry: 00001594
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::AVCCommandExtendedTimeout(unsigned char const*, unsigned int, unsigned
   char*, unsigned int*) */

undefined4 __thiscall
m_audio_b_FWAVCUnit::AVCCommandExtendedTimeout
          (m_audio_b_FWAVCUnit *this,uchar *param_1,uint param_2,uchar *param_3,uint *param_4)

{
  undefined4 uVar1;
  long *plVar2;
  
  if (param_2 - 1 < 0x200) {
    plVar2 = (long *)IOFireWireAVCCommand::withNub
                               (*(IOFireWireNub **)(this + 0x88),param_1,param_2,param_3,param_4);
    if (plVar2 == (long *)0x0) {
      uVar1 = 0xe00002bd;
    }
    else {
      _IOLockLock(*(undefined8 *)(this + 0x98));
      *(long **)(this + 0xa0) = plVar2;
      *(undefined4 *)(plVar2 + 9) = 1000000;
      uVar1 = (**(code **)(**(long **)(this + 0xa0) + 0x140))(*(long **)(this + 0xa0),0);
      _IOLockLock(*(undefined8 *)(this + 0xd0));
      *(undefined8 *)(this + 0xa0) = 0;
      _IOLockUnlock(*(undefined8 *)(this + 0xd0));
      (**(code **)(*plVar2 + 0x28))(plVar2);
      _IOLockUnlock(*(undefined8 *)(this + 0x98));
    }
  }
  else {
    _IOLog("Loopy AVCCmd, len %u, respLen %u\n",param_2,*param_4);
    uVar1 = 0xe00002c2;
  }
  return uVar1;
}
