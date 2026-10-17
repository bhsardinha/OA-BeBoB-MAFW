// m_audio_b_FWBaseEngine::init
// Entry: 00005dd4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::init() */

undefined1 __thiscall m_audio_b_FWBaseEngine::init(m_audio_b_FWBaseEngine *this)

{
  undefined1 uVar1;
  void *pvVar2;
  size_t local_20;
  undefined4 local_14;
  
  this[0x2d5] = (m_audio_b_FWBaseEngine)0x0;
  this[0x2d4] = (m_audio_b_FWBaseEngine)0x0;
  this[0x2d3] = (m_audio_b_FWBaseEngine)0x0;
  this[0x238] = (m_audio_b_FWBaseEngine)0x0;
  this[0x2d0] = (m_audio_b_FWBaseEngine)0x0;
  *(undefined4 *)(this + 0x220) = 0xac44;
  *(undefined4 *)(this + 0x228) = 0;
  *(undefined8 *)(this + 0x140) = 0;
  *(undefined8 *)(this + 0x210) = 0;
  *(undefined8 *)(this + 0x218) = 0;
  *(undefined8 *)(this + 0x300) = 0;
  *(undefined8 *)(this + 0x308) = 0;
  *(undefined8 *)(this + 0x310) = 0;
  *(m_audio_b_FWBaseEngine **)(this + 0x2b8) = this;
  *(undefined4 *)(this + 0x260) = 0;
  *(undefined4 *)(this + 0x264) = 0;
  pvVar2 = operator_new__(0x1000);
  *(void **)(this + 600) = pvVar2;
  local_14 = 0;
  local_20 = 4;
  _sysctlbyname("hw.optional.sse3",&local_14,&local_20,(void *)0x0,0);
  this[0x340] = (m_audio_b_FWBaseEngine)0x0;
  if (*(long *)(this + 600) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(PTR_vtable_000300b0 + 600))(this,0);
  }
  return uVar1;
}
