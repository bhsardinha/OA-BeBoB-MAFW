// m_audio_b_FWBaseEngine::PerformSampleRateChangeCore
// Entry: 0000479e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::PerformSampleRateChangeCore(_IOAudioSampleRate const*) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::PerformSampleRateChangeCore
          (m_audio_b_FWBaseEngine *this,_IOAudioSampleRate *param_1)

{
  int iVar1;
  long *plVar2;
  long lVar3;
  long *local_20;
  
  *(undefined4 *)(this + 0x220) = *(undefined4 *)param_1;
  local_20 = (long *)0x0;
  lVar3 = TCollectionIterator<m_audio_b_FWDCLProgram*>::start
                    ((TCollectionIterator<m_audio_b_FWDCLProgram*> *)&local_20,
                     *(OSCollection **)(this + 0x178));
  plVar2 = local_20;
  do {
    if (lVar3 == 0) {
      if (plVar2 != (long *)0x0) {
        (**(code **)(*plVar2 + 0x28))(plVar2);
      }
      ResetLatency(this);
      this[0x346] = (m_audio_b_FWBaseEngine)((byte)this[0x346] | 0x1d);
      return 1;
    }
    iVar1 = *(int *)(this + 0x220);
    if (iVar1 < 48000) {
      if (iVar1 == 32000) {
        *(undefined4 *)(this + 0x224) = 8;
        *(undefined4 *)(this + 0x228) = 0;
      }
      else {
LAB_0000484e:
        *(undefined4 *)(this + 0x224) = 8;
        *(undefined4 *)(this + 0x228) = 1;
      }
    }
    else if (iVar1 < 0x15888) {
      if (iVar1 != 48000) goto LAB_0000484e;
      *(undefined4 *)(this + 0x224) = 8;
      *(undefined4 *)(this + 0x228) = 2;
    }
    else if (iVar1 < 0x2ee00) {
      if (iVar1 == 0x15888) {
        *(undefined4 *)(this + 0x224) = 0x10;
        *(undefined4 *)(this + 0x228) = 3;
      }
      else if (iVar1 == 96000) {
        *(undefined4 *)(this + 0x224) = 0x10;
        *(undefined4 *)(this + 0x228) = 4;
      }
      else {
        if (iVar1 != 0x2b110) goto LAB_0000484e;
        *(undefined4 *)(this + 0x224) = 0x20;
        *(undefined4 *)(this + 0x228) = 5;
      }
    }
    else {
      if (iVar1 != 0x2ee00) goto LAB_0000484e;
      *(undefined4 *)(this + 0x224) = 0x20;
      *(undefined4 *)(this + 0x228) = 6;
    }
    lVar3 = (**(code **)(*plVar2 + 0x128))(plVar2);
  } while( true );
}
