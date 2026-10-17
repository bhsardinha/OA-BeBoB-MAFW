// m_audio_b_FWBaseArrayProp::CheckInvariant
// Entry: 00007f8a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::CheckInvariant(unsigned int) */

void __thiscall
m_audio_b_FWBaseArrayProp::CheckInvariant(m_audio_b_FWBaseArrayProp *this,uint param_1)

{
  char local_118 [128];
  char local_98 [128];
  
  if (*(long *)(this + 0x50) == 0) {
    _snprintf(local_98,0x80,"fValues == NULL (%d)",0);
                    /* WARNING: Subroutine does not return */
    _panic(
          "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/../Shared/KernelPropertyIPC/TArrayProperty.cpp:341"
          );
  }
  if (*(uint *)(this + 0x2c) <= param_1) {
    _snprintf(local_118,0x80,"Tried to access index %u of an array with %u elements",param_1);
                    /* WARNING: Subroutine does not return */
    _panic(
          "&yuck[0]@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/../Shared/KernelPropertyIPC/TArrayProperty.cpp:346"
          );
  }
  return;
}
