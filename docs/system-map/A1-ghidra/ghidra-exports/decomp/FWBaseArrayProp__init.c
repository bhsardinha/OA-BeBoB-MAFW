// m_audio_b_FWBaseArrayProp::init
// Entry: 000084f8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::init(m_audio_b_FWPropertyDict*, char const*, unsigned int) */

undefined4 __thiscall
m_audio_b_FWBaseArrayProp::init
          (m_audio_b_FWBaseArrayProp *this,m_audio_b_FWPropertyDict *param_1,char *param_2,
          uint param_3)

{
  undefined4 uVar1;
  void *pvVar2;
  char local_a0 [128];
  
  (*(code *)PTR_init_00034dc8)(this,param_2);
  *(undefined4 *)(this + 0x58) = 0;
  *(uint *)(this + 0x2c) = param_3;
  if (param_3 == 0) {
    _snprintf(local_a0,0x80,"Cannot create an array with zero (%d) elements",0);
                    /* WARNING: Subroutine does not return */
    _panic(
          "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/../Shared/KernelPropertyIPC/TArrayProperty.cpp:32"
          );
  }
  pvVar2 = operator_new__((ulong)param_3 << 2);
  *(void **)(this + 0x50) = pvVar2;
  if (pvVar2 == (void *)0x0) {
    (**(code **)(*(long *)this + 0x28))(this);
    uVar1 = 0xe00002bd;
  }
  else {
    uVar1 = m_audio_b_FWPropertyDict::InstallProperty(param_1,(m_audio_b_FWBaseProp *)this);
    (**(code **)(*(long *)this + 0x28))(this);
  }
  return uVar1;
}
