// m_audio_FWb::BaseChannelGroup::Solo
// Entry: 0001ed64
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::BaseChannelGroup::Solo() const */

undefined1 __thiscall m_audio_FWb::BaseChannelGroup::Solo(BaseChannelGroup *this)

{
  uint uVar1;
  long lVar2;
  uint uVar3;
  
  uVar1 = (**(code **)(*(long *)this + 0x10))(this);
  uVar3 = 0;
  while( true ) {
    if (uVar1 <= uVar3) {
      return 0;
    }
    lVar2 = (**(code **)(*(long *)this + 0x18))(this,uVar3);
    if (*(long *)(lVar2 + 0x10) != 0) break;
    uVar3 = uVar3 + 1;
  }
  return 1;
}
