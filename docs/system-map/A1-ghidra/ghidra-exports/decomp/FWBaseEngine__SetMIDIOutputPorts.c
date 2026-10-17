// m_audio_b_FWBaseEngine::SetMIDIOutputPorts
// Entry: 00005160
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_b_FWBaseEngine::SetMIDIOutputPorts(m_audio_b_FWAudioDevice*, unsigned int, char const**)
    */

void __thiscall
m_audio_b_FWBaseEngine::SetMIDIOutputPorts
          (m_audio_b_FWBaseEngine *this,m_audio_b_FWAudioDevice *param_1,uint param_2,char **param_3
          )

{
  char cVar1;
  void *pvVar2;
  uint uVar3;
  uint uVar4;
  
  if (this[0x298] != (m_audio_b_FWBaseEngine)0x0) {
    return;
  }
  if (*(void **)(this + 0x2a8) != (void *)0x0) {
    operator_delete__(*(void **)(this + 0x2a8));
  }
  uVar4 = 8;
  if (param_2 < 8) {
    uVar4 = param_2;
  }
  *(uint *)(this + 0x2a0) = uVar4;
  uVar3 = 0;
  if (uVar4 != 0) {
    pvVar2 = operator_new__((ulong)uVar4 * 0x28);
    *(void **)(this + 0x2a8) = pvVar2;
    if (pvVar2 == (void *)0x0) {
      return;
    }
    for (uVar4 = 0; uVar3 = *(uint *)(this + 0x2a0), uVar4 < uVar3; uVar4 = uVar4 + 1) {
      cVar1 = m_audio_FWb::TKernelMIDIPort::Init
                        ((TKernelMIDIPort *)(*(long *)(this + 0x2a8) + (ulong)uVar4 * 0x28));
      if (cVar1 == '\0') {
        uVar3 = *(uint *)(this + 0x2a0);
        break;
      }
    }
  }
  SetMIDIPortProperties(this,param_1,uVar3,false,param_3);
  return;
}
