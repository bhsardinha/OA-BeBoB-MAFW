// m_audio_b_FWAVCAudioDevice::AllocateConnectionManager
// Entry: 00023c72
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCAudioDevice::AllocateConnectionManager() */

m_audio_b_FWAVCConnectionManager * m_audio_b_FWAVCAudioDevice::AllocateConnectionManager(void)

{
  char cVar1;
  undefined8 uVar2;
  m_audio_b_FWAVCUnit *this;
  m_audio_b_FWAVCConnectionManager *this_00;
  ulong in_RSI;
  ulong uVar3;
  long in_RDI;
  
  if (*(long **)(in_RDI + 0xe0) != (long *)0x0) {
    uVar2 = (**(code **)(**(long **)(in_RDI + 0xe0) + 0x2f0))();
    this = OSObject::operator_new((OSObject *)&section_000000b8.flags,in_RSI);
    if (this != (m_audio_b_FWAVCUnit *)0x0) {
      m_audio_b_FWAVCUnit::m_audio_b_FWAVCUnit(this);
    }
    *(m_audio_b_FWAVCUnit **)(in_RDI + 0x268) = this;
    cVar1 = (**(code **)(*(long *)this + 0x248))(this,uVar2);
    if (cVar1 != '\0') {
      (**(code **)(**(long **)(in_RDI + 0x268) + 0x670))();
      (**(code **)(**(long **)(in_RDI + 0x268) + 0x28))();
      uVar3 = *(ulong *)(in_RDI + 0xe0);
      cVar1 = (**(code **)(**(long **)(in_RDI + 0x268) + 0x5c0))();
      if (cVar1 != '\0') {
        this_00 = OSObject::operator_new((OSObject *)&segment_command_00000020.vmaddr,uVar3);
        if (this_00 == (m_audio_b_FWAVCConnectionManager *)0x0) {
          return (m_audio_b_FWAVCConnectionManager *)0x0;
        }
        m_audio_b_FWAVCConnectionManager::m_audio_b_FWAVCConnectionManager(this_00);
        (**(code **)(*(long *)this_00 + 0x118))(this_00);
        return this_00;
      }
    }
  }
  return (m_audio_b_FWAVCConnectionManager *)0x0;
}
