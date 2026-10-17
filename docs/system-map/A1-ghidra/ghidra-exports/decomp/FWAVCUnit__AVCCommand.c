// m_audio_b_FWAVCUnit::AVCCommand
// Entry: 000017fc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::AVCCommand(unsigned char const*, unsigned int, m_audio_FWb::TFunctor<void
   (unsigned int, unsigned char*), m_audio_FWb::FunctorNilArgument, m_audio_FWb::FunctorNilArgument,
   m_audio_FWb::FunctorNilArgument>*) */

ulong m_audio_b_FWAVCUnit::AVCCommand(uchar *param_1,uint param_2,TFunctor *param_3)

{
  IOFireWireAVCAsynchronousCommand *this;
  ulong uVar1;
  undefined4 in_register_00000034;
  
  this = OSObject::operator_new
                   ((OSObject *)&section_00000068.flags,CONCAT44(in_register_00000034,param_2));
  if (this == (IOFireWireAVCAsynchronousCommand *)0x0) {
    return 0xe00002bd;
  }
  IOFireWireAVCAsynchronousCommand::IOFireWireAVCAsynchronousCommand
            (this,(OSMetaClass *)&m_audio_b_FWAsyncAVCCommand::gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWAsyncAVCCommand_00030e70;
  OSMetaClass::instanceConstructed();
  uVar1 = IOFireWireAVCAsynchronousCommand::init
                    ((uchar *)this,param_2,
                     (_func_void_void_ptr_IOFireWireAVCAsynchronousCommand_ptr *)
                     ((ulong)param_3 & 0xffffffff),FWAVCAsyncCommandCallback);
  if ((int)uVar1 == 0) {
    *(uchar **)(this + 0xa0) = param_1;
    if (*(long *)(param_1 + 0xf0) == 0) {
      if (*(long *)(param_1 + 0xe8) != 0) {
        return uVar1;
      }
      uVar1 = IOFireWireAVCAsynchronousCommand::submit((IOFireWireAVCNub *)this);
      if ((int)uVar1 != 0) goto LAB_000018bc;
      *(IOFireWireAVCAsynchronousCommand **)(param_1 + 0xe8) = this;
    }
    else {
      *(IOFireWireAVCAsynchronousCommand **)(*(long *)(param_1 + 0xf0) + 0x98) = this;
    }
    *(IOFireWireAVCAsynchronousCommand **)(param_1 + 0xf0) = this;
  }
  else {
LAB_000018bc:
    uVar1 = uVar1 & 0xffffffff;
    (**(code **)(*(long *)this + 0x28))(this);
  }
  return uVar1;
}
