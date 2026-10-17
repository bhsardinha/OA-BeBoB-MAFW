// m_audio_b_FWAVCAudioDevice::CustomMixerWriteQuads
// Entry: 00022c7a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAVCAudioDevice::CustomMixerWriteQuads(unsigned int, unsigned int, void*) */

int __thiscall
m_audio_b_FWAVCAudioDevice::CustomMixerWriteQuads
          (m_audio_b_FWAVCAudioDevice *this,uint param_1,uint param_2,void *param_3)

{
  int iVar1;
  void *pvVar2;
  long *plVar3;
  IOBufferMemoryDescriptor *pIVar4;
  IOBufferMemoryDescriptor *local_40;
  undefined2 local_38;
  undefined2 local_36;
  int local_34;
  
  local_40 = (IOBufferMemoryDescriptor *)0x0;
  if (*(long *)(this + 0xe0) == 0) {
    iVar1 = 0;
    pIVar4 = (IOBufferMemoryDescriptor *)0x0;
  }
  else {
    iVar1 = AllocateCustomMixerWriteBuffer(this,param_2,&local_40);
    pIVar4 = local_40;
    if (iVar1 == 0) {
      pvVar2 = (void *)(**(code **)(*(long *)local_40 + 0x2e0))(local_40);
      _memcpy(pvVar2,param_3,(ulong)param_2 << 2);
      local_38 = 0;
      local_36 = 0xffc7;
      local_34 = param_1 + 0x700000;
      plVar3 = (long *)(**(code **)(**(long **)(this + 0xe0) + 0x890))
                                 (*(long **)(this + 0xe0),&local_38,pIVar4,0,0,0);
      if (plVar3 == (long *)0x0) {
        iVar1 = -0x1ffffd42;
      }
      else {
        iVar1 = (**(code **)(*plVar3 + 0x140))(plVar3,0);
        (**(code **)(*plVar3 + 0x28))(plVar3);
      }
    }
  }
  ReleaseCustomMixerWriteBuffer(this,pIVar4);
  return iVar1;
}
