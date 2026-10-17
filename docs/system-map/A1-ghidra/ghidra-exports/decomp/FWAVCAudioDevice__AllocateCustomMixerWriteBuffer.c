// m_audio_b_FWAVCAudioDevice::AllocateCustomMixerWriteBuffer
// Entry: 00022c12
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::AllocateCustomMixerWriteBuffer(unsigned int,
   IOBufferMemoryDescriptor*&) */

int __thiscall
m_audio_b_FWAVCAudioDevice::AllocateCustomMixerWriteBuffer
          (m_audio_b_FWAVCAudioDevice *this,uint param_1,IOBufferMemoryDescriptor **param_2)

{
  int iVar1;
  IOBufferMemoryDescriptor *pIVar2;
  
  pIVar2 = (IOBufferMemoryDescriptor *)
           IOBufferMemoryDescriptor::withOptions(3,(ulong)param_1 << 2,0x10);
  *param_2 = pIVar2;
  if (pIVar2 == (IOBufferMemoryDescriptor *)0x0) {
    iVar1 = -0x1ffffd43;
    pIVar2 = (IOBufferMemoryDescriptor *)0x0;
  }
  else {
    iVar1 = (**(code **)(*(long *)pIVar2 + 0x1f0))(pIVar2,0);
    if (iVar1 == 0) {
      return 0;
    }
    pIVar2 = *param_2;
  }
  if (pIVar2 != (IOBufferMemoryDescriptor *)0x0) {
    (**(code **)(*(long *)pIVar2 + 0x28))(pIVar2);
    *param_2 = (IOBufferMemoryDescriptor *)0x0;
  }
  return iVar1;
}
