// m_audio_b_FWBaseEngine::ResetSampleBuffers
// Entry: 000057f8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::ResetSampleBuffers() */

undefined1 __thiscall m_audio_b_FWBaseEngine::ResetSampleBuffers(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  char cVar2;
  undefined1 uVar3;
  uint uVar4;
  uint uVar5;
  
  if (*(long *)(this + 0x218) == 0) {
    uVar3 = 1;
  }
  else {
    iVar1 = *(int *)(this + 0x2e8);
    *(int *)(this + 0x2d8) = (*(int *)(this + 0x2ec) + iVar1) * 0x8000;
    *(int *)(this + 0x2dc) = (*(int *)(this + 0x2f0) + iVar1) * 0x8000;
    uVar5 = (*(int *)(this + 0x2e4) + iVar1) * 0x8000;
    uVar4 = (iVar1 + *(int *)(this + 0x2e0)) * 0x8000;
    cVar2 = m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes((TRingBuffer *)(this + 0x1e0),uVar4)
    ;
    if ((((cVar2 == '\0') ||
         (cVar2 = m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes
                            ((TRingBuffer *)(this + 0x148),uVar4), cVar2 == '\0')) ||
        (cVar2 = m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes
                           ((TRingBuffer *)(this + 0x268),uVar5), cVar2 == '\0')) ||
       ((cVar2 = m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes
                           ((TRingBuffer *)(this + 0x180),uVar5), cVar2 == '\0' ||
        (cVar2 = m_audio_FWb::TRingBuffer::InitializeWithSizeInBytes
                           ((TRingBuffer *)(this + 0x1b0),uVar5), cVar2 == '\0')))) {
      uVar3 = 0;
    }
    else {
      *(undefined8 *)(this + 0x208) = *(undefined8 *)(this + 0x1e8);
      *(undefined8 *)(this + 0x290) = *(undefined8 *)(this + 0x270);
      *(undefined8 *)(this + 0x1a8) = *(undefined8 *)(this + 0x188);
      *(undefined8 *)(this + 0x1d8) = *(undefined8 *)(this + 0x1b8);
      *(undefined8 *)(this + 0x170) = *(undefined8 *)(this + 0x150);
      (**(code **)(**(long **)(this + 0x218) + 0x9e0))
                (*(long **)(this + 0x218),*(undefined8 *)(this + 0x1e8),
                 *(undefined4 *)(this + 0x1f0));
      (**(code **)(**(long **)(this + 0x218) + 0x9f8))
                (*(long **)(this + 0x218),*(undefined8 *)(this + 0x170),
                 *(undefined4 *)(this + 0x158));
      (**(code **)(**(long **)(this + 0x210) + 0x9e0))
                (*(long **)(this + 0x210),*(undefined8 *)(this + 0x290),
                 *(undefined4 *)(this + 0x278));
      uVar3 = 1;
    }
  }
  return uVar3;
}
