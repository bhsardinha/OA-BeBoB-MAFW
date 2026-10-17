// m_audio_b_FWIsochChannel::setTalker
// Entry: 00006f32
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::setTalker(IOFWIsochPort*) */

undefined8 __thiscall
m_audio_b_FWIsochChannel::setTalker(m_audio_b_FWIsochChannel *this,IOFWIsochPort *param_1)

{
  undefined8 uVar1;
  
  if ((param_1 != (IOFWIsochPort *)0x0) && (*(IOFWIsochPort **)(this + 0x28) != param_1)) {
    if (this[0x78] != (m_audio_b_FWIsochChannel)0x0) {
                    /* WARNING: Subroutine does not return */
      _panic(
            "\"FWIsochChannel::setTalker while running\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWIsochChannel.cpp:47"
            );
    }
    (**(code **)(*(long *)param_1 + 0x20))(param_1);
    ReleaseTalker(this);
                    /* WARNING: Could not recover jumptable at 0x00006f81. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar1 = (**(code **)(PTR_vtable_000300c8 + 0x148))(this,param_1);
    return uVar1;
  }
  return 0xe00002c2;
}
