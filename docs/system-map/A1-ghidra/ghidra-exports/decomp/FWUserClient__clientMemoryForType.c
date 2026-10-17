// com_m_audio_FWUserClient::clientMemoryForType
// Entry: 00007220
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::clientMemoryForType(unsigned int, unsigned int*, IOMemoryDescriptor**)
    */

undefined8 __thiscall
com_m_audio_FWUserClient::clientMemoryForType
          (com_m_audio_FWUserClient *this,uint param_1,uint *param_2,IOMemoryDescriptor **param_3)

{
  IOMemoryDescriptor *pIVar1;
  long lVar2;
  undefined8 uVar3;
  
  *param_3 = (IOMemoryDescriptor *)0x0;
  if (param_1 == 1) {
    pIVar1 = *(IOMemoryDescriptor **)(*(long *)(this + 0x290) + 0x100);
    *param_3 = pIVar1;
    if (pIVar1 == (IOMemoryDescriptor *)0x0) {
      return 0xe00002bc;
    }
    (**(code **)(*(long *)pIVar1 + 0x20))();
    *param_2 = 0x1000;
    return 0;
  }
  lVar2 = *(long *)(*(long *)(this + 0x290) + 0xe8);
  if (param_1 < 0x800) {
    if ((0x3ff < param_1) && (param_1 - 0x400 < *(uint *)(lVar2 + 0x29c))) {
      *param_3 = *(IOMemoryDescriptor **)
                  (*(long *)(lVar2 + 0x2b0) + 0x20 + (ulong)(param_1 - 0x400) * 0x28);
      *param_2 = 0x1000;
      goto LAB_000072dd;
    }
  }
  else if (param_1 - 0x800 < *(uint *)(lVar2 + 0x2a0)) {
    *param_3 = *(IOMemoryDescriptor **)
                (*(long *)(lVar2 + 0x2a8) + 0x20 + (ulong)(param_1 - 0x800) * 0x28);
    *param_2 = 0;
LAB_000072dd:
    uVar3 = 0;
    goto LAB_000072df;
  }
  uVar3 = 0xe00002c7;
LAB_000072df:
  if (*param_3 != (IOMemoryDescriptor *)0x0) {
    (**(code **)(*(long *)*param_3 + 0x20))();
  }
  return uVar3;
}
