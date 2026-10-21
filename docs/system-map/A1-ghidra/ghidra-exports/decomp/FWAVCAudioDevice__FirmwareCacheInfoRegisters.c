// m_audio_b_FWAVCAudioDevice::FirmwareCacheInfoRegisters
// Entry: 00023702
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_b_FWAVCAudioDevice::FirmwareCacheInfoRegisters(IOService*) */

undefined4 __thiscall
m_audio_b_FWAVCAudioDevice::FirmwareCacheInfoRegisters
          (m_audio_b_FWAVCAudioDevice *this,IOService *param_1)

{
  char cVar1;
  undefined4 uVar2;
  long lVar3;
  IOFWReadQuadCommand *this_00;
  ulong uVar4;
  undefined2 local_28;
  undefined2 local_26;
  undefined4 local_24;
  
  if ((param_1 == (IOService *)0x0) ||
     (lVar3 = OSMetaClassBase::safeMetaCast
                        ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030050),
     lVar3 == 0)) {
    lVar3 = *(long *)(this + 0xe0);
  }
  if (lVar3 != 0) {
    uVar4 = 0x50;
    _bzero(this + 0x278,0x50);
    this_00 = OSObject::operator_new((OSObject *)&section_000000b8.addr,uVar4);
    if (this_00 != (IOFWReadQuadCommand *)0x0) {
      IOFWReadQuadCommand::IOFWReadQuadCommand(this_00);
      local_28 = 0;
      local_26 = 0xffff;
      local_24 = 0xc8020000;
      cVar1 = (**(code **)(*(long *)this_00 + 0x1d0))
                        (this_00,lVar3,&local_28,this + 0x278,0x14,0,0,0);
      if (cVar1 != '\0') {
        uVar2 = (**(code **)(*(long *)this_00 + 0x140))(this_00,0);
        (**(code **)(*(long *)this_00 + 0x28))(this_00);
        return uVar2;
      }
    }
  }
  return 0xe00002bc;
}
