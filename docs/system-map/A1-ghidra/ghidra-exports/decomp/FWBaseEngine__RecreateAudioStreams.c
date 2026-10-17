// m_audio_b_FWBaseEngine::RecreateAudioStreams
// Entry: 00004546
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::RecreateAudioStreams(bool, bool) */

bool __thiscall
m_audio_b_FWBaseEngine::RecreateAudioStreams(m_audio_b_FWBaseEngine *this,bool param_1,bool param_2)

{
  long lVar1;
  m_audio_b_FWBaseEngine *pmVar2;
  bool bVar3;
  
  (**(code **)(*(long *)this + 0xbf0))(this);
  if (*(long **)(this + 0x218) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x218) + 0x28))();
    *(undefined8 *)(this + 0x218) = 0;
  }
  if (*(long **)(this + 0x210) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x210) + 0x28))();
    *(undefined8 *)(this + 0x210) = 0;
  }
  pmVar2 = this;
  if (!param_2) {
    pmVar2 = (m_audio_b_FWBaseEngine *)0x0;
  }
  lVar1 = (**(code **)(*(long *)this + 0xc80))
                    (this,pmVar2,0,*(undefined8 *)(this + 0x208),*(undefined4 *)(this + 0x2d8));
  *(long *)(this + 0x218) = lVar1;
  bVar3 = false;
  if (lVar1 != 0) {
    pmVar2 = (m_audio_b_FWBaseEngine *)0x0;
    if (param_1) {
      pmVar2 = this;
    }
    lVar1 = (**(code **)(*(long *)this + 0xc80))
                      (this,pmVar2,1,*(undefined8 *)(this + 0x290),*(undefined4 *)(this + 0x2dc));
    *(long *)(this + 0x210) = lVar1;
    bVar3 = lVar1 != 0;
    if (bVar3) {
      this[0x346] = (m_audio_b_FWBaseEngine)((byte)this[0x346] | 0x20);
    }
  }
  return bVar3;
}
