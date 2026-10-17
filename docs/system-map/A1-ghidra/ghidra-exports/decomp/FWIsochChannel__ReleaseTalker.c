// m_audio_b_FWIsochChannel::ReleaseTalker
// Entry: 00006ee6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::ReleaseTalker() */

void __thiscall m_audio_b_FWIsochChannel::ReleaseTalker(m_audio_b_FWIsochChannel *this)

{
  if (this[0x78] != (m_audio_b_FWIsochChannel)0x0) {
                    /* WARNING: Subroutine does not return */
    _panic(
          "\"FWIsochChannel::ReleaseTalker while running\"@/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/KernelExtension/Source/FWIsochChannel.cpp:25"
          );
  }
  if (*(long **)(this + 0x28) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x28) + 0x128))();
    (**(code **)(**(long **)(this + 0x28) + 0x28))();
    *(undefined8 *)(this + 0x28) = 0;
  }
  return;
}
