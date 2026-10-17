// m_audio_b_FWBaseEngine::SetMIDIPortProperties
// Entry: 00005028
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_b_FWBaseEngine::SetMIDIPortProperties(m_audio_b_FWAudioDevice*, unsigned int, bool, char
   const**) */

void __thiscall
m_audio_b_FWBaseEngine::SetMIDIPortProperties
          (m_audio_b_FWBaseEngine *this,m_audio_b_FWAudioDevice *param_1,uint param_2,bool param_3,
          char **param_4)

{
  long *plVar1;
  long *plVar2;
  char *pcVar3;
  ulong uVar4;
  char local_34 [4];
  
  if (this[0x298] == (m_audio_b_FWBaseEngine)0x0) {
    if (param_2 == 0) {
      if (param_1 != (m_audio_b_FWAudioDevice *)0x0) {
        pcVar3 = "MAMIDIInputPorts";
        if (!param_3) {
          pcVar3 = "MAMIDIOutputPorts";
        }
        (**(code **)(*(long *)param_1 + 0x2a0))(param_1,pcVar3);
      }
    }
    else {
      plVar1 = (long *)OSArray::withCapacity(4);
      if (plVar1 != (long *)0x0) {
        if (param_4 == (char **)0x0) {
          local_34[0] = '1';
          local_34[1] = 0;
          local_34[2] = 0;
          local_34[3] = 0;
          do {
            plVar2 = (long *)OSString::withCString(local_34);
            local_34[0] = local_34[0] + '\x01';
            if (plVar2 != (long *)0x0) {
              (**(code **)(*plVar1 + 0x1b8))(plVar1,plVar2);
              (**(code **)(*plVar2 + 0x28))(plVar2);
            }
            param_2 = param_2 - 1;
          } while (param_2 != 0);
        }
        else {
          uVar4 = (ulong)param_2;
          do {
            plVar2 = (long *)OSString::withCString(*param_4);
            if (plVar2 != (long *)0x0) {
              (**(code **)(*plVar1 + 0x1b8))(plVar1,plVar2);
              (**(code **)(*plVar2 + 0x28))(plVar2);
            }
            param_4 = param_4 + 1;
            uVar4 = uVar4 - 1;
          } while (uVar4 != 0);
        }
        if (param_1 != (m_audio_b_FWAudioDevice *)0x0) {
          pcVar3 = "MAMIDIInputPorts";
          if (!param_3) {
            pcVar3 = "MAMIDIOutputPorts";
          }
          (**(code **)(*(long *)param_1 + 0x268))(param_1,pcVar3,plVar1);
        }
        (**(code **)(*plVar1 + 0x28))(plVar1);
      }
    }
  }
  return;
}
